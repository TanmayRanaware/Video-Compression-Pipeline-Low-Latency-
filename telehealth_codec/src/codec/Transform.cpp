#include <codec/Transform.h>
#include <cstring>
#include <algorithm>

namespace telehealth {
namespace codec {

// Simple integer 8x8 DCT-like transform (Hadamard-style for speed)
// Forward: scale and round; inverse: scale back.
static const int c8[8][8] = {
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,-1,-1,-1,-1},
  {1,1,-1,-1,0,0,0,0},
  {0,0,0,0,1,1,-1,-1},
  {1,-1,0,0,0,0,0,0},
  {0,0,1,-1,0,0,0,0},
  {0,0,0,0,1,-1,0,0},
  {0,0,0,0,0,0,1,-1}
};

static void transform_8x8_core(const int16_t* in, int in_stride, int32_t* out) {
  int32_t tmp[64];
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      int32_t sum = 0;
      for (int k = 0; k < 8; ++k)
        sum += c8[i][k] * in[k * in_stride + j];
      tmp[i * 8 + j] = sum;
    }
  }
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      int32_t sum = 0;
      for (int k = 0; k < 8; ++k)
        sum += tmp[i * 8 + k] * c8[j][k];
      out[i * 8 + j] = sum >> 3;
    }
  }
}

static void itransform_8x8_core(const int32_t* in, int32_t* out, int out_stride) {
  int32_t tmp[64];
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      int32_t sum = 0;
      for (int k = 0; k < 8; ++k)
        sum += c8[k][i] * in[k * 8 + j];
      tmp[i * 8 + j] = sum;
    }
  }
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      int32_t sum = 0;
      for (int k = 0; k < 8; ++k)
        sum += tmp[i * 8 + k] * c8[k][j];
      out[i * out_stride + j] = sum >> 3;
    }
  }
}

void Transform::forward_8x8(const int16_t* residual, int residual_stride, int32_t* coeff_out) {
  transform_8x8_core(residual, residual_stride, coeff_out);
}

void Transform::inverse_8x8(const int32_t* coeff, int32_t* residual_out, int residual_stride) {
  itransform_8x8_core(coeff, residual_out, residual_stride);
}

void Transform::forward_16x16(const int16_t* residual, int residual_stride, int32_t* coeff_out) {
  for (int by = 0; by < 2; ++by) {
    for (int bx = 0; bx < 2; ++bx) {
      const int16_t* blk = residual + by * 8 * residual_stride + bx * 8;
      int32_t* out_blk = coeff_out + (by * 2 + bx) * 64;
      forward_8x8(blk, residual_stride, out_blk);
    }
  }
}

void Transform::inverse_16x16(const int32_t* coeff, int16_t* residual_out, int residual_stride) {
  int32_t tmp[64];
  for (int by = 0; by < 2; ++by) {
    for (int bx = 0; bx < 2; ++bx) {
      inverse_8x8(coeff + (by * 2 + bx) * 64, tmp, 8);
      for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
          residual_out[(by * 8 + y) * residual_stride + bx * 8 + x] = static_cast<int16_t>(std::clamp(tmp[y * 8 + x], -32768, 32767));
    }
  }
}

}  // namespace codec
}  // namespace telehealth
