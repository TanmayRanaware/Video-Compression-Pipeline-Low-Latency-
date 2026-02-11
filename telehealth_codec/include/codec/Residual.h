#pragma once

#include "Block.h"
#include <cstdint>

namespace telehealth {
namespace codec {

/// Residual block: int16_t (current - predicted)
struct ResidualBlock {
  static constexpr int kSize = 16;
  int16_t data[kSize * kSize] = {};
};

/// Compute residual = current - predicted (luma 16x16 block)
void compute_residual(const BlockViewConst& cur,
                      const BlockViewConst& pred,
                      int16_t* residual_out);

/// Same for 8x8 (e.g. chroma or transform block)
void compute_residual_8x8(const uint8_t* cur, int cur_stride,
                          const uint8_t* pred, int pred_stride,
                          int16_t* residual_out);

}  // namespace codec
}  // namespace telehealth
