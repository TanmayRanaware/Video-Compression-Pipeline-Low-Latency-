#include <codec/EntropyCoder.h>
#include <codec/Block.h>
#include <algorithm>
#include <cstring>

namespace telehealth {
namespace codec {

const int kZigzag8x8[64] = {
  0,  1,  8, 16,  9,  2,  3, 10,
  17, 24, 32, 25, 18, 11,  4,  5,
  12, 19, 26, 33, 40, 48, 41, 34,
  27, 20, 13,  6,  7, 14, 21, 28,
  35, 42, 49, 56, 57, 50, 43, 36,
  29, 22, 15, 23, 30, 37, 44, 51,
  58, 59, 52, 45, 38, 31, 39, 46,
  53, 60, 61, 54, 47, 55, 62, 63
};

static void encode_coeff_run(BitstreamWriter& out, int run, int level) {
  if (run > 15) {
    out.write_bits(0xF, 4);
    out.write_bits(static_cast<uint32_t>(run - 16), 8);
  } else {
    out.write_bits(static_cast<uint32_t>(run), 4);
  }
  int l = std::abs(level);
  int s = level < 0 ? 1 : 0;
  out.write_bits(static_cast<uint32_t>(l), 12);
  out.write_bits(static_cast<uint32_t>(s), 1);
}

static void decode_coeff_run(BitstreamReader& in, int& run, int& level) {
  run = static_cast<int>(in.read_bits(4));
  if (run == 15)
    run += static_cast<int>(in.read_bits(8));
  level = static_cast<int>(in.read_bits(12));
  if (in.read_bits(1))
    level = -level;
}

void EntropyCoder::encode_block_8x8(const int32_t* coeff, int qp, BitstreamWriter& out) {
  int run = 0;
  for (int i = 0; i < 64; ++i) {
    int idx = kZigzag8x8[i];
    int v = coeff[idx];
    if (v == 0) {
      run++;
    } else {
      encode_coeff_run(out, run, v);
      run = 0;
    }
  }
  encode_coeff_run(out, run, 0);
}

void EntropyCoder::decode_block_8x8(BitstreamReader& in, int qp, int32_t* coeff_out) {
  std::memset(coeff_out, 0, 64 * sizeof(int32_t));
  int run, level;
  int k = 0;
  for (;;) {
    decode_coeff_run(in, run, level);
    k += run;
    if (k >= 64) break;
    coeff_out[kZigzag8x8[k]] = level;
    k++;
    if (k >= 64) break;
  }
}

void EntropyCoder::encode_mv(MotionVector mv, BitstreamWriter& out) {
  int dx = mv.dx, dy = mv.dy;
  out.write_bits(static_cast<uint32_t>(dx & 0xFFFF), 16);
  out.write_bits(static_cast<uint32_t>(dy & 0xFFFF), 16);
}

MotionVector EntropyCoder::decode_mv(BitstreamReader& in) {
  MotionVector mv;
  mv.dx = static_cast<int16_t>(in.read_bits(16));
  mv.dy = static_cast<int16_t>(in.read_bits(16));
  return mv;
}

void EntropyCoder::encode_mb(const int32_t* coeff_y, const int32_t* coeff_u, const int32_t* coeff_v,
                             const MotionVector* mv, bool is_p_frame, int qp, BitstreamWriter& out) {
  if (is_p_frame && mv)
    encode_mv(*mv, out);
  for (int i = 0; i < 4; ++i)
    encode_block_8x8(coeff_y + i * 64, qp, out);
  encode_block_8x8(coeff_u, qp, out);
  encode_block_8x8(coeff_v, qp, out);
}

}  // namespace codec
}  // namespace telehealth
