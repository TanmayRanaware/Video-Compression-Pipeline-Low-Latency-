#pragma once

#include "Block.h"
#include "Frame.h"
#include "MotionVector.h"

namespace telehealth {
namespace codec {

class MotionCompensation {
 public:
  MotionCompensation() = default;

  /// Build predicted block from reference frame using integer-pel MV; clamp at boundaries.
  void predict_block(BlockView pred_out,
                     const FrameYUV& ref_frame,
                     BlockCoord pos,
                     MotionVector mv) const;

  /// Build full predicted frame from ref and MV array (one MV per macroblock).
  void predict_frame(FrameYUV& pred_frame,
                     const FrameYUV& ref_frame,
                     const MotionVector* mvs,
                     int mb_cols,
                     int mb_rows) const;
};

}  // namespace codec
}  // namespace telehealth
