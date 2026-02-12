#include <codec/Frame.h>
#include <cstring>
#include <algorithm>

namespace telehealth {
namespace codec {

Frame::Frame(std::shared_ptr<FrameStorage> storage, FrameFormat fmt, int w, int h,
             int sy, int suv, int s, uint8_t* y, uint8_t* u, uint8_t* v, uint8_t* data)
    : storage_(std::move(storage)),
      format_(fmt),
      width_(w),
      height_(h),
      stride_y_(sy),
      stride_uv_(suv),
      stride_(s),
      y_ptr_(y),
      u_ptr_(u),
      v_ptr_(v),
      data_ptr_(data) {}

std::shared_ptr<Frame> Frame::make_i420(int w, int h) {
  if (w <= 0 || h <= 0) return std::make_shared<Frame>();
  auto storage = std::make_shared<FrameStorage>();
  storage->width = w;
  storage->height = h;
  storage->stride_y = (w + 31) & ~31;
  storage->stride_uv = ((w / 2) + 31) & ~31;
  storage->y_plane.resize(static_cast<size_t>(storage->stride_y * h));
  storage->u_plane.resize(static_cast<size_t>(storage->stride_uv * (h / 2)));
  storage->v_plane.resize(static_cast<size_t>(storage->stride_uv * (h / 2)));
  uint8_t* yp = storage->y_plane.empty() ? nullptr : storage->y_plane.data();
  uint8_t* up = storage->u_plane.empty() ? nullptr : storage->u_plane.data();
  uint8_t* vp = storage->v_plane.empty() ? nullptr : storage->v_plane.data();
  int sy = storage->stride_y, suv = storage->stride_uv;
  return std::shared_ptr<Frame>(new Frame(std::move(storage), FrameFormat::I420, w, h, sy, suv, 0, yp, up, vp, nullptr));
}

std::shared_ptr<Frame> Frame::make_rgb24(int w, int h) {
  if (w <= 0 || h <= 0) return std::make_shared<Frame>();
  auto storage = std::make_shared<FrameStorage>();
  storage->width = w;
  storage->height = h;
  storage->stride = (w * 3 + 31) & ~31;
  storage->data.resize(static_cast<size_t>(storage->stride * h));
  uint8_t* dp = storage->data.empty() ? nullptr : storage->data.data();
  int s = storage->stride;
  return std::shared_ptr<Frame>(new Frame(std::move(storage), FrameFormat::RGB24, w, h, 0, 0, s, nullptr, nullptr, nullptr, dp));
}

std::shared_ptr<Frame> Frame::from_rgb(const FrameRGB& rgb, int64_t frame_id, int64_t timestamp_us, double pts_sec) {
  if (rgb.empty()) return std::make_shared<Frame>();
  auto f = make_rgb24(rgb.width, rgb.height);
  const int h = rgb.height;
  const size_t line_bytes = static_cast<size_t>(rgb.stride);
  for (int y = 0; y < h; ++y)
    std::memcpy(f->row(y), rgb.row(y), line_bytes);
  f->frame_id_ = frame_id;
  f->timestamp_us_ = timestamp_us;
  f->pts_sec_ = pts_sec;
  return f;
}

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
