#include <io/UdpReceiver.h>
#include <io/Packet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <map>
#include <vector>

namespace telehealth {
namespace io {

UdpReceiver::~UdpReceiver() {
  close();
}

bool UdpReceiver::open(uint16_t port) {
  close();
  port_ = port;
  socket_ = ::socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_ < 0) return false;
  int opt = 1;
  setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  struct sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(socket_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
    ::close(socket_);
    socket_ = -1;
    return false;
  }
  recv_buf_.resize(65536);
  return true;
}

void UdpReceiver::close() {
  if (socket_ >= 0) {
    ::close(socket_);
    socket_ = -1;
  }
}

void UdpReceiver::poll(int timeout_ms) {
  if (socket_ < 0 || !frame_callback_) return;
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(socket_, &fds);
  struct timeval tv { timeout_ms / 1000, (timeout_ms % 1000) * 1000 };
  if (select(socket_ + 1, &fds, nullptr, nullptr, &tv) <= 0) return;

  struct sockaddr_in from;
  socklen_t fromlen = sizeof(from);
  ssize_t n = recvfrom(socket_, recv_buf_.data(), recv_buf_.size(), 0,
                      reinterpret_cast<struct sockaddr*>(&from), &fromlen);
  if (n < static_cast<ssize_t>(PACKET_HEADER_SIZE)) return;

  PacketHeader h;
  std::memcpy(&h, recv_buf_.data(), PACKET_HEADER_SIZE);
  size_t payload_len = std::min(static_cast<size_t>(h.payload_size), recv_buf_.size() - PACKET_HEADER_SIZE);
  if (h.total_packets == 1) {
    frame_callback_(h.frame_id, h.timestamp_us, recv_buf_.data() + PACKET_HEADER_SIZE, payload_len);
    return;
  }
  static thread_local std::map<uint32_t, std::map<uint16_t, std::vector<uint8_t>>> reassemble;
  auto& frame_packets = reassemble[h.frame_id];
  frame_packets[h.packet_id].assign(recv_buf_.data() + PACKET_HEADER_SIZE, recv_buf_.data() + PACKET_HEADER_SIZE + payload_len);
  if (frame_packets.size() == h.total_packets) {
    std::vector<uint8_t> full;
    for (uint16_t i = 0; i < h.total_packets; ++i) {
      auto it = frame_packets.find(i);
      if (it == frame_packets.end()) break;
      full.insert(full.end(), it->second.begin(), it->second.end());
    }
    if (full.size() > 0)
      frame_callback_(h.frame_id, h.timestamp_us, full.data(), full.size());
    reassemble.erase(h.frame_id);
  }
}

}  // namespace io
}  // namespace telehealth
