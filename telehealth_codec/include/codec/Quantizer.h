#pragma once

#include <cstdint>

namespace telehealth {
namespace codec {

/// Quantize/dequantize with QP. Simple scale-based for MVP.
class Quantizer {
 public:
  Quantizer() = default;

  /// Quantize 8x8 coeffs in place (int32 -> int16 or int32 with scale)
  void quantize_8x8(int32_t* coeff, int qp);
  void dequantize_8x8(const int32_t* coeff_in, int32_t* coeff_out, int qp);

  /// QP to scale factor (simplified)
  static int qp_to_scale(int qp);
};

}  // namespace codec
}  // namespace telehealth
