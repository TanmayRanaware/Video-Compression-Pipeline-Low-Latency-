#include <codec/YuvConverter.h>
#include <algorithm>

namespace telehealth {
namespace codec {

static void rgb_to_yuv_pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t& y, uint8_t& u, uint8_t& v) {
  int ri = r, gi = g, bi = b;
  y = static_cast<uint8_t>(std::clamp((77 * ri + 150 * gi + 29 * bi) >> 8, 0, 255));
  u = static_cast<uint8_t>(std::clamp(((-43 * ri - 85 * gi + 128 * bi) >> 8) + 128, 0, 255));
  v = static_cast<uint8_t>(std::clamp(((128 * ri - 107 * gi - 21 * bi) >> 8) + 128, 0, 255));
}

void YuvConverter::rgb_to_yuv420(const FrameRGB& in, FrameYUV& out) {
  if (in.empty()) return;
  out.allocate(in.width, in.height);

  const int w = in.width;
  const int h = in.height;

  for (int y = 0; y < h; ++y) {
    const uint8_t* row = in.row(y);
    uint8_t* y_row = out.y_row(y);
    for (int x = 0; x < w; ++x) {
      uint8_t r = row[x * 3], g = row[x * 3 + 1], b = row[x * 3 + 2];
      uint8_t py, pu, pv;
      rgb_to_yuv_pixel(r, g, b, py, pu, pv);
      y_row[x] = py;
      if ((y & 1) == 0 && (x & 1) == 0) {
        out.u_row(y / 2)[x / 2] = pu;
        out.v_row(y / 2)[x / 2] = pv;
      }
    }
  }
}

void YuvConverter::rgb_to_yuv420(const Frame& in, Frame& out) {
  if (in.empty() || in.format() != FrameFormat::RGB24) return;
  if (out.empty() || out.format() != FrameFormat::I420) return;
  const int w = in.width();
  const int h = in.height();
  if (w != out.width() || h != out.height()) return;

  for (int y = 0; y < h; ++y) {
    const uint8_t* row = in.row(y);
    uint8_t* y_row = out.y_row(y);
    for (int x = 0; x < w; ++x) {
      uint8_t r = row[x * 3], g = row[x * 3 + 1], b = row[x * 3 + 2];
      uint8_t py, pu, pv;
      rgb_to_yuv_pixel(r, g, b, py, pu, pv);
      y_row[x] = py;
      if ((y & 1) == 0 && (x & 1) == 0) {
        out.u_row(y / 2)[x / 2] = pu;
        out.v_row(y / 2)[x / 2] = pv;
      }
    }
  }
}

void YuvConverter::yuv420_to_rgb(const FrameYUV& in, FrameRGB& out) {
  if (in.empty()) return;
  out.allocate(in.width, in.height);

  const int w = in.width;
  const int h = in.height;

  for (int y = 0; y < h; ++y) {
    const uint8_t* y_row = in.y_row(y);
    const uint8_t* u_row = in.u_row(y / 2);
    const uint8_t* v_row = in.v_row(y / 2);
    uint8_t* dst = out.row(y);
    for (int x = 0; x < w; ++x) {
      int yy = y_row[x];
      int uu = u_row[x / 2] - 128;
      int vv = v_row[x / 2] - 128;
      int r = yy + ((1436 * vv) >> 10);
      int g = yy - ((352 * uu + 731 * vv) >> 10);
      int b = yy + ((1812 * uu) >> 10);
      dst[x * 3]     = static_cast<uint8_t>(std::clamp(r, 0, 255));
      dst[x * 3 + 1] = static_cast<uint8_t>(std::clamp(g, 0, 255));
      dst[x * 3 + 2] = static_cast<uint8_t>(std::clamp(b, 0, 255));
    }
  }
}

}  // namespace codec
}  // namespace telehealth
