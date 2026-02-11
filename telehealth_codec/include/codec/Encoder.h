#pragma once

#include "Bitstream.h"
#include "EncoderConfig.h"
#include "Frame.h"
#include "MotionVector.h"
#include <memory>
#include <vector>

namespace telehealth {
namespace codec {

class MotionEstimation;
class MotionCompensation;
class Transform;
class Quantizer;
class EntropyCoder;
class RateControl;

class Encoder {
 public:
  explicit Encoder(const EncoderConfig& config);
  ~Encoder();

  /// Encode one YUV frame. Returns encoded frame (raw_bytes filled for streaming).
  EncodedFrame encode(const FrameYUV& frame, const FrameMeta& meta);

  const EncoderConfig& config() const { return config_; }

 private:
  EncodedFrame encode_i_frame(const FrameYUV& frame, const FrameMeta& meta);
  EncodedFrame encode_p_frame(const FrameYUV& frame, const FrameMeta& meta);

  EncoderConfig config_;
  std::unique_ptr<FrameYUV> reference_;
  std::unique_ptr<MotionEstimation> me_;
  std::unique_ptr<MotionCompensation> mc_;
  std::unique_ptr<Transform> transform_;
  std::unique_ptr<Quantizer> quantizer_;
  std::unique_ptr<EntropyCoder> entropy_;
  std::unique_ptr<RateControl> rate_control_;
  std::vector<MotionVector> mv_buffer_;
  std::vector<int32_t> coeff_buffer_;
  std::vector<int16_t> residual_buffer_;
};

}  // namespace codec
}  // namespace telehealth
