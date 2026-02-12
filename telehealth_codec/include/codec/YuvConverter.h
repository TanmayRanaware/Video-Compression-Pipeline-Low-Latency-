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

  /// Convert refcounted Frame RGB24 -> I420. out must be I420 (e.g. from Frame::make_i420).
  void rgb_to_yuv420(const Frame& in, Frame& out);

  /// Convert YUV420p to RGB24 (e.g. for display or PSNR). Allocates out if needed.
  void yuv420_to_rgb(const FrameYUV& in, FrameRGB& out);
};

}  // namespace codec
}  // namespace telehealth
