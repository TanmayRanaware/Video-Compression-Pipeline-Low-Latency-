#include <codec/RateControl.h>
#include <algorithm>

namespace telehealth {
namespace codec {

RateControl::RateControl(const EncoderConfig& config)
    : config_(config), target_kbps_(config.target_bitrate_kbps), current_qp_(config.qp_default) {}

FrameType RateControl::choose_frame_type(uint32_t frame_id, const FrameStats* previous) const {
  if (frame_id == 0) return FrameType::I;
  if (config_.gop_size > 0 && (frame_id % config_.gop_size) == 0)
    return FrameType::I;
  if (previous && previous->force_keyframe) return FrameType::I;
  return FrameType::P;
}

int RateControl::choose_qp(const FrameStats& stats) {
  frame_count_++;
  window_bits_ += stats.bits_used;

  int target_bits_per_frame = (target_kbps_ * 1000) / (config_.fps > 0 ? config_.fps : 30);
  if (stats.bits_used > static_cast<uint32_t>(target_bits_per_frame * 120 / 100))
    current_qp_ = std::min(config_.qp_max, current_qp_ + 2);
  else if (stats.bits_used < static_cast<uint32_t>(target_bits_per_frame * 80 / 100))
    current_qp_ = std::max(config_.qp_min, current_qp_ - 1);

  return std::clamp(current_qp_, config_.qp_min, config_.qp_max);
}

}  // namespace codec
}  // namespace telehealth
