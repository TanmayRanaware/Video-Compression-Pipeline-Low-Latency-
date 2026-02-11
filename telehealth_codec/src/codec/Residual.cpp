#include <codec/Residual.h>
#include <codec/Block.h>

namespace telehealth {
namespace codec {

void compute_residual(const BlockViewConst& cur,
                      const BlockViewConst& pred,
                      int16_t* residual_out) {
  const int h = std::min(cur.h, pred.h);
  const int w = std::min(cur.w, pred.w);
  for (int y = 0; y < h; ++y) {
    const uint8_t* c = cur.row(y);
    const uint8_t* p = pred.row(y);
    for (int x = 0; x < w; ++x)
      residual_out[y * 16 + x] = static_cast<int16_t>(static_cast<int>(c[x]) - static_cast<int>(p[x]));
  }
}

void compute_residual_8x8(const uint8_t* cur, int cur_stride,
                          const uint8_t* pred, int pred_stride,
                          int16_t* residual_out) {
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x)
      residual_out[y * 8 + x] = static_cast<int16_t>(static_cast<int>(cur[y * cur_stride + x]) - static_cast<int>(pred[y * pred_stride + x]));
  }
}

}  // namespace codec
}  // namespace telehealth
