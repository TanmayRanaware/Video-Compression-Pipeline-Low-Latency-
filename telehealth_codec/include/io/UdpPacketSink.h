#pragma once

#include <io/Packet.h>
#include <string>
#include <cstdint>
#include <vector>

namespace telehealth {
namespace io {

class UdpPacketSink {
 public:
  UdpPacketSink() = default;
  ~UdpPacketSink();
  bool open(const std::string& host, uint16_t port);
  void close();
  bool send_packet(const uint8_t* payload, size_t payload_size, const PacketHeader& header);
  bool send_frame(const std::vector<uint8_t>& frame_data, uint32_t frame_id, uint64_t timestamp_us,
                  uint32_t stream_id = 0);
  bool is_open() const { return socket_ >= 0; }

 private:
  int socket_ = -1;
  std::string host_;
  uint16_t port_ = 0;
  size_t mtu_ = DEFAULT_MTU;
};

}  // namespace io
}  // namespace telehealth
