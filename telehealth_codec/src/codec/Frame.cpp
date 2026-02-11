#include <codec/Frame.h>
#include <cstring>
#include <algorithm>

namespace telehealth {
namespace codec {

FrameYUV::FrameYUV(int w, int h) {
  allocate(w, h);
}

void FrameYUV::allocate(int w, int h) {
  width = w;
  height = h;
  stride_y = (w + 31) & ~31;
  stride_uv = ((w / 2) + 31) & ~31;
  y_plane.resize(static_cast<size_t>(stride_y * height));
  u_plane.resize(static_cast<size_t>(stride_uv * (height / 2)));
  v_plane.resize(static_cast<size_t>(stride_uv * (height / 2)));
}

FrameRGB::FrameRGB(int w, int h) {
  allocate(w, h);
}

void FrameRGB::allocate(int w, int h) {
  width = w;
  height = h;
  stride = (w * 3 + 31) & ~31;
  data.resize(static_cast<size_t>(stride * height));
}

}  // namespace codec
}  // namespace telehealth
