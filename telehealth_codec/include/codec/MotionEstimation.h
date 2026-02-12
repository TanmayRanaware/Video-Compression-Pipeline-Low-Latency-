#pragma once

#include "Block.h"
#include "Frame.h"
#include "MotionVector.h"
#include "EncoderConfig.h"
#include <cstdint>

namespace telehealth {
namespace codec {

class MotionEstimation {
 public:
  explicit MotionEstimation(const EncoderConfig& config);

  /// Full search: find best MV in [−range, +range] using SAD.
  MotionResult estimate(const BlockViewConst& cur_block,
                        const FrameYUV& ref_frame,
                        BlockCoord pos) const;
  MotionResult estimate(const BlockViewConst& cur_block,
                        const Frame& ref_frame,
                        BlockCoord pos) const;

  /// Diamond search (faster, optional).
  MotionResult estimate_diamond(const BlockViewConst& cur_block,
                                const FrameYUV& ref_frame,
                                BlockCoord pos) const;
  MotionResult estimate_diamond(const BlockViewConst& cur_block,
                                const Frame& ref_frame,
                                BlockCoord pos) const;

  uint32_t sad_block(const BlockViewConst& cur, const BlockViewConst& ref) const;
  int search_range() const { return config_.search_range; }

 private:
  EncoderConfig config_;
  bool in_bounds(const FrameYUV& frame, int x, int y, int w, int h) const;
  bool in_bounds(const Frame& frame, int x, int y, int w, int h) const;
};

}  // namespace codec
}  // namespace telehealth
