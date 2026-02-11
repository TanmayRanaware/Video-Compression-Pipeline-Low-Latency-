#include <codec/Quantizer.h>
#include <algorithm>
#include <cmath>

namespace telehealth {
namespace codec {

int Quantizer::qp_to_scale(int qp) {
  if (qp <= 0) return 1;
  if (qp >= 51) return 256;
  return static_cast<int>(std::round(std::exp(qp * 0.115)));  // roughly 2^(qp/6)
}

void Quantizer::quantize_8x8(int32_t* coeff, int qp) {
  int scale = qp_to_scale(qp);
  for (int i = 0; i < 64; ++i) {
    int v = coeff[i];
    coeff[i] = (v >= 0) ? (v + scale / 2) / scale : (v - scale / 2) / scale;
  }
}

void Quantizer::dequantize_8x8(const int32_t* coeff_in, int32_t* coeff_out, int qp) {
  int scale = qp_to_scale(qp);
  for (int i = 0; i < 64; ++i)
    coeff_out[i] = coeff_in[i] * scale;
}

}  // namespace codec
}  // namespace telehealth
