#pragma once

#include <io/Packet.h>
#include <codec/Bitstream.h>
#include <map>
#include <vector>
#include <cstdint>
#include <functional>

namespace telehealth {
namespace io {

/// Reassembles packets into frames; handles reorder and timeout (drop frame on loss).
class JitterBuffer {
 public:
  struct Config {
    int reassembly_timeout_ms = 100;
    uint32_t stream_id = 0;
  };

  JitterBuffer();
  explicit JitterBuffer(const Config& config);
  void push_packet(const uint8_t* data, size_t size);
  void set_frame_ready_callback(std::function<void(uint32_t frame_id, uint64_t ts, std::vector<uint8_t> payload)> cb) {
    frame_ready_ = std::move(cb);
  }
  void tick(int elapsed_ms);

 private:
  struct IncompleteFrame {
    uint32_t frame_id = 0;
    uint64_t timestamp_us = 0;
    std::map<uint16_t, std::vector<uint8_t>> packets;
    uint16_t total_packets = 0;
    int wait_ms = 0;
  };

  Config config_;
  std::map<uint32_t, IncompleteFrame> incomplete_;
  std::function<void(uint32_t frame_id, uint64_t ts, std::vector<uint8_t> payload)> frame_ready_;
};

}  // namespace io
}  // namespace telehealth
