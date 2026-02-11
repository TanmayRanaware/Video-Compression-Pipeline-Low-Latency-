#pragma once

#include "EncoderConfig.h"
#include "Bitstream.h"
#include <cstdint>

namespace telehealth {
namespace codec {

struct FrameStats {
  uint32_t frame_id = 0;
  uint32_t bits_used = 0;
  double sad_sum = 0;   // sum of SADs (scene activity)
  bool force_keyframe = false;
};

class RateControl {
 public:
  explicit RateControl(const EncoderConfig& config);

  int choose_qp(const FrameStats& stats);
  FrameType choose_frame_type(uint32_t frame_id, const FrameStats* previous) const;

  void set_target_bitrate_kbps(uint32_t kbps) { target_kbps_ = kbps; }

 private:
  EncoderConfig config_;
  uint32_t target_kbps_ = 500;
  int current_qp_ = 28;
  uint32_t window_bits_ = 0;
  int frame_count_ = 0;
};

}  // namespace codec
}  // namespace telehealth
