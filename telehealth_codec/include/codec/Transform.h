#pragma once

#include <cstdint>

namespace telehealth {
namespace codec {

/// 8x8 integer DCT-like transform (forward and inverse for decoder path).
class Transform {
 public:
  Transform() = default;

  /// Forward: 8x8 int16 residual -> 8x8 int32 coeffs (before quant)
  void forward_8x8(const int16_t* residual, int residual_stride, int32_t* coeff_out);

  /// Inverse: 8x8 coeffs -> 8x8 int16 residual (after dequant)
  void inverse_8x8(const int32_t* coeff, int32_t* residual_out, int residual_stride);

  /// Forward for 16x16 MB: four 8x8 blocks
  void forward_16x16(const int16_t* residual, int residual_stride, int32_t* coeff_out);

  /// Inverse for 16x16 MB
  void inverse_16x16(const int32_t* coeff, int16_t* residual_out, int residual_stride);
};

}  // namespace codec
}  // namespace telehealth
