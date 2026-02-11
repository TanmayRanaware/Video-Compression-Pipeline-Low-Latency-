#include <io/UdpPacketSink.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

namespace telehealth {
namespace io {

UdpPacketSink::~UdpPacketSink() {
  close();
}

bool UdpPacketSink::open(const std::string& host, uint16_t port) {
  close();
  host_ = host;
  port_ = port;
  socket_ = ::socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_ < 0) return false;
  return true;
}

void UdpPacketSink::close() {
  if (socket_ >= 0) {
    ::close(socket_);
    socket_ = -1;
  }
}

bool UdpPacketSink::send_packet(const uint8_t* payload, size_t payload_size, const PacketHeader& header) {
  if (socket_ < 0 || !payload) return false;
  struct sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port_);
  if (inet_pton(AF_INET, host_.c_str(), &addr.sin_addr) <= 0) return false;

  std::vector<uint8_t> buf(PACKET_HEADER_SIZE + payload_size);
  std::memcpy(buf.data(), &header, PACKET_HEADER_SIZE);
  if (payload_size) std::memcpy(buf.data() + PACKET_HEADER_SIZE, payload, payload_size);

  ssize_t sent = sendto(socket_, buf.data(), buf.size(), 0,
                       reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
  return sent == static_cast<ssize_t>(buf.size());
}

bool UdpPacketSink::send_frame(const std::vector<uint8_t>& frame_data, uint32_t frame_id, uint64_t timestamp_us,
                               uint32_t stream_id) {
  if (socket_ < 0 || frame_data.empty()) return false;
  size_t payload_max = mtu_ - PACKET_HEADER_SIZE;
  size_t total = frame_data.size();
  uint16_t total_packets = static_cast<uint16_t>((total + payload_max - 1) / payload_max);

  for (uint16_t i = 0; i < total_packets; ++i) {
    size_t off = i * payload_max;
    size_t len = std::min(payload_max, total - off);
    PacketHeader h;
    h.stream_id = stream_id;
    h.frame_id = frame_id;
    h.packet_id = i;
    h.total_packets = total_packets;
    h.timestamp_us = timestamp_us;
    h.payload_size = static_cast<uint32_t>(len);
    if (!send_packet(frame_data.data() + off, len, h))
      return false;
  }
  return true;
}

}  // namespace io
}  // namespace telehealth
