#include <io/JitterBuffer.h>
#include <io/Packet.h>
#include <cstring>
#include <algorithm>

namespace telehealth {
namespace io {

JitterBuffer::JitterBuffer() : config_() {}

JitterBuffer::JitterBuffer(const Config& config) : config_(config) {}

void JitterBuffer::push_packet(const uint8_t* data, size_t size) {
  if (size < PACKET_HEADER_SIZE) return;
  PacketHeader h;
  std::memcpy(&h, data, PACKET_HEADER_SIZE);
  if (h.stream_id != config_.stream_id) return;

  auto& frame = incomplete_[h.frame_id];
  frame.frame_id = h.frame_id;
  frame.timestamp_us = h.timestamp_us;
  frame.total_packets = h.total_packets;
  frame.wait_ms = 0;
  frame.packets[h.packet_id].assign(data + PACKET_HEADER_SIZE, data + size);
}

void JitterBuffer::tick(int elapsed_ms) {
  for (auto it = incomplete_.begin(); it != incomplete_.end(); ) {
    it->second.wait_ms += elapsed_ms;
    if (it->second.wait_ms >= config_.reassembly_timeout_ms) {
      it = incomplete_.erase(it);
      continue;
    }
    if (it->second.packets.size() == it->second.total_packets) {
      std::vector<uint8_t> payload;
      for (uint16_t i = 0; i < it->second.total_packets; ++i) {
        auto p = it->second.packets.find(i);
        if (p == it->second.packets.end()) break;
        payload.insert(payload.end(), p->second.begin(), p->second.end());
      }
      if (frame_ready_ && !payload.empty())
        frame_ready_(it->second.frame_id, it->second.timestamp_us, std::move(payload));
      it = incomplete_.erase(it);
    } else {
      ++it;
    }
  }
}

}  // namespace io
}  // namespace telehealth
