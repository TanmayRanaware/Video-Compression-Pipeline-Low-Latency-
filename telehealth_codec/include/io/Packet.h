#pragma once

#include <cstdint>
#include <vector>

namespace telehealth {
namespace io {

/// Packet header for UDP streaming (MTU-sized chunks)
struct PacketHeader {
  uint32_t stream_id = 0;
  uint32_t frame_id = 0;
  uint16_t packet_id = 0;
  uint16_t total_packets = 0;
  uint64_t timestamp_us = 0;
  uint32_t payload_size = 0;  // bytes in this packet
};

constexpr size_t PACKET_HEADER_SIZE = 24;
constexpr size_t DEFAULT_MTU = 1200;

}  // namespace io
}  // namespace telehealth
