#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace telehealth {
namespace codec {

/// Pixel format for raw input (e.g. from camera)
enum class PixelFormat { RGB24, YUV420P };

/// Format for refcounted Frame (zero-copy mindset)
enum class FrameFormat { I420, NV12, RGB24 };

/// Refcounted storage for frame planes (shared across stages)
struct FrameStorage {
  int width = 0;
  int height = 0;
  std::vector<uint8_t> y_plane;
  std::vector<uint8_t> u_plane;
  std::vector<uint8_t> v_plane;
  std::vector<uint8_t> data;  // for RGB24: single contiguous buffer
  int stride_y = 0;
  int stride_uv = 0;
  int stride = 0;  // RGB stride
};

/// Frame with plane pointers, format, timestamp/frame_id, and reference-counted ownership.
/// Shared across pipeline stages via shared_ptr<Frame>.
class Frame {
 public:
  Frame() = default;

  int width() const { return width_; }
  int height() const { return height_; }
  FrameFormat format() const { return format_; }
  int64_t frame_id() const { return frame_id_; }
  int64_t timestamp_us() const { return timestamp_us_; }
  double pts_sec() const { return pts_sec_; }

  void set_meta(int64_t id, int64_t ts_us, double pts = 0.0) {
    frame_id_ = id;
    timestamp_us_ = ts_us;
    pts_sec_ = pts;
  }

  bool empty() const { return width_ == 0 || height_ == 0; }

  /// I420 / YUV accessors (valid when format() is I420 or NV12)
  int stride_y() const { return stride_y_; }
  int stride_uv() const { return stride_uv_; }
  const uint8_t* y_plane_ptr() const { return y_ptr_; }
  const uint8_t* u_plane_ptr() const { return u_ptr_; }
  const uint8_t* v_plane_ptr() const { return v_ptr_; }
  uint8_t* y_plane_ptr() { return y_ptr_; }
  uint8_t* u_plane_ptr() { return u_ptr_; }
  uint8_t* v_plane_ptr() { return v_ptr_; }
  const uint8_t* y_row(int row) const { return y_ptr_ + row * stride_y_; }
  uint8_t* y_row(int row) { return y_ptr_ + row * stride_y_; }
  const uint8_t* u_row(int row) const { return u_ptr_ + row * stride_uv_; }
  uint8_t* u_row(int row) { return u_ptr_ + row * stride_uv_; }
  const uint8_t* v_row(int row) const { return v_ptr_ + row * stride_uv_; }
  uint8_t* v_row(int row) { return v_ptr_ + row * stride_uv_; }

  /// RGB accessors (valid when format() is RGB24)
  int stride() const { return stride_; }
  const uint8_t* data_ptr() const { return data_ptr_; }
  uint8_t* data_ptr() { return data_ptr_; }
  const uint8_t* row(int row) const { return data_ptr_ + row * stride_; }
  uint8_t* row(int row) { return data_ptr_ + row * stride_; }

  /// Factory: create refcounted I420 frame (allocates storage)
  static std::shared_ptr<Frame> make_i420(int w, int h);
  /// Factory: create refcounted RGB24 frame (allocates storage)
  static std::shared_ptr<Frame> make_rgb24(int w, int h);
  /// Create refcounted Frame from existing FrameRGB (copy); meta optional
  static std::shared_ptr<Frame> from_rgb(const struct FrameRGB& rgb,
                                         int64_t frame_id = 0, int64_t timestamp_us = 0, double pts_sec = 0.0);

 private:
  Frame(std::shared_ptr<FrameStorage> storage, FrameFormat fmt, int w, int h,
        int stride_y, int stride_uv, int stride,
        uint8_t* y, uint8_t* u, uint8_t* v, uint8_t* data);

  std::shared_ptr<FrameStorage> storage_;
  FrameFormat format_ = FrameFormat::I420;
  int width_ = 0;
  int height_ = 0;
  int stride_y_ = 0;
  int stride_uv_ = 0;
  int stride_ = 0;
  uint8_t* y_ptr_ = nullptr;
  uint8_t* u_ptr_ = nullptr;
  uint8_t* v_ptr_ = nullptr;
  uint8_t* data_ptr_ = nullptr;
  int64_t frame_id_ = 0;
  int64_t timestamp_us_ = 0;
  double pts_sec_ = 0.0;
};

/// Planar YUV 4:2:0 frame: Y full res, U/V half resolution (legacy owning type)
struct FrameYUV {
  int width = 0;
  int height = 0;
  int stride_y = 0;   // bytes per row for Y
  int stride_uv = 0;  // bytes per row for U and V
  std::vector<uint8_t> y_plane;
  std::vector<uint8_t> u_plane;
  std::vector<uint8_t> v_plane;

  FrameYUV() = default;
  FrameYUV(int w, int h);
  void allocate(int w, int h);
  bool empty() const { return width == 0 || height == 0; }
  uint8_t* y_row(int row) { return y_plane.data() + row * stride_y; }
  const uint8_t* y_row(int row) const { return y_plane.data() + row * stride_y; }
  uint8_t* u_row(int row) { return u_plane.data() + row * stride_uv; }
  const uint8_t* u_row(int row) const { return u_plane.data() + row * stride_uv; }
  uint8_t* v_row(int row) { return v_plane.data() + row * stride_uv; }
  const uint8_t* v_row(int row) const { return v_plane.data() + row * stride_uv; }
};

/// RGB frame (e.g. from capture) (legacy owning type)
struct FrameRGB {
  int width = 0;
  int height = 0;
  int stride = 0;  // bytes per row
  std::vector<uint8_t> data;

  FrameRGB() = default;
  FrameRGB(int w, int h);
  void allocate(int w, int h);
  bool empty() const { return width == 0 || height == 0; }
  uint8_t* row(int row) { return data.data() + row * stride; }
  const uint8_t* row(int row) const { return data.data() + row * stride; }
};

/// Metadata for a frame (timestamps, IDs)
struct FrameMeta {
  int64_t frame_id = 0;
  int64_t timestamp_us = 0;
  double pts_sec = 0.0;
};

}  // namespace codec
}  // namespace telehealth
