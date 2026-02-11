#pragma once

#include <cstdint>
#include <string>

namespace telehealth {
namespace codec {

struct EncoderConfig {
  int width = 640;
  int height = 480;
  int fps = 30;
  int gop_size = 30;           // I-frame every N frames
  int search_range = 16;       // ±pixels for motion search
  int qp_default = 28;         // default quantization parameter
  int qp_min = 18;
  int qp_max = 42;
  uint32_t target_bitrate_kbps = 500;
  bool use_diamond_search = false;  // else full search
  int early_termination_threshold = 0;  // 0 = disabled
  int frame_budget_ms = 33;    // target ms per frame for real-time
};

}  // namespace codec
}  // namespace telehealth
