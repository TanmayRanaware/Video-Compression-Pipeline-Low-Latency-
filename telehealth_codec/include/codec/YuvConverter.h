#pragma once

#include "Frame.h"

namespace telehealth {
namespace codec {

/// Converts RGB to YUV420p planar. Produces aligned planes for SIMD.
class YuvConverter {
 public:
  YuvConverter() = default;

  /// Convert RGB24 to YUV420p. Fills out_frame (allocates if needed).
  void rgb_to_yuv420(const FrameRGB& in, FrameYUV& out);

  /// Convert YUV420p to RGB24 (e.g. for display or PSNR). Allocates out if needed.
  void yuv420_to_rgb(const FrameYUV& in, FrameRGB& out);
};

}  // namespace codec
}  // namespace telehealth
