#pragma once

#include <io/Packet.h>
#include <functional>
#include <cstdint>
#include <vector>

namespace telehealth {
namespace io {

/// Callback: reassembled frame payload
using FrameCallback = std::function<void(uint32_t frame_id, uint64_t timestamp_us, const uint8_t* data, size_t size)>;

class UdpReceiver {
 public:
  UdpReceiver() = default;
  ~UdpReceiver();
  bool open(uint16_t port);
  void close();
  void set_frame_callback(FrameCallback cb) { frame_callback_ = std::move(cb); }
  void poll(int timeout_ms = 10);
  bool is_open() const { return socket_ >= 0; }

 private:
  int socket_ = -1;
  uint16_t port_ = 0;
  FrameCallback frame_callback_;
  std::vector<uint8_t> recv_buf_;
};

}  // namespace io
}  // namespace telehealth
