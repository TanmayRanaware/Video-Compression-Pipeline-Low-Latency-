#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace telehealth {
namespace codec {

/// Pixel format for raw input (e.g. from camera)
enum class PixelFormat { RGB24, YUV420P };

/// Planar YUV 4:2:0 frame: Y full res, U/V half resolution
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

/// RGB frame (e.g. from capture)
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
