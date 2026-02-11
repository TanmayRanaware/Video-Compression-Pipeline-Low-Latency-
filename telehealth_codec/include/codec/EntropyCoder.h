#pragma once

#include "Bitstream.h"
#include "MotionVector.h"
#include <cstdint>
#include <vector>

namespace telehealth {
namespace codec {

/// Zigzag order for 8x8
extern const int kZigzag8x8[64];

/// Encode quantized coeffs: zigzag + RLE zeros + simple VLC (Huffman-like).
class EntropyCoder {
 public:
  EntropyCoder() = default;

  void encode_block_8x8(const int32_t* coeff, int qp, BitstreamWriter& out);
  void decode_block_8x8(BitstreamReader& in, int qp, int32_t* coeff_out);

  void encode_mv(MotionVector mv, BitstreamWriter& out);
  MotionVector decode_mv(BitstreamReader& in);

  /// Encode full MB: 4x 8x8 blocks (luma 16x16) + 2x 8x8 chroma
  void encode_mb(const int32_t* coeff_y, const int32_t* coeff_u, const int32_t* coeff_v,
                 const MotionVector* mv, bool is_p_frame, int qp, BitstreamWriter& out);
};

}  // namespace codec
}  // namespace telehealth
