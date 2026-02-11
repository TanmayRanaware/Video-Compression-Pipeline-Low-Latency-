#pragma once

#include "Frame.h"
#include "MotionVector.h"
#include <cstdint>
#include <vector>
#include <string>

namespace telehealth {
namespace codec {

enum class FrameType { I, P };

/// File header for our custom bitstream
struct BitstreamFileHeader {
  uint32_t magic = 0x54434F44;  // "TCOD"
  uint16_t version = 1;
  uint16_t width = 0;
  uint16_t height = 0;
  uint8_t fps = 30;
  uint8_t chroma_format = 0;  // 0 = 4:2:0
  uint8_t reserved[2] = {};
};

/// Per-frame header in bitstream
struct BitstreamFrameHeader {
  uint8_t frame_type = 0;  // 0=I, 1=P
  uint32_t frame_id = 0;
  uint64_t timestamp_us = 0;
  uint8_t qp = 28;
  uint32_t mv_payload_bytes = 0;
  uint32_t coeff_payload_bytes = 0;
  uint8_t reserved[4] = {};
};

/// Encoded frame output (bytes + metadata)
struct EncodedFrame {
  FrameType type = FrameType::I;
  uint32_t frame_id = 0;
  uint64_t timestamp_us = 0;
  uint8_t qp = 28;
  std::vector<uint8_t> mv_bytes;
  std::vector<uint8_t> coeff_bytes;
  std::vector<uint8_t> raw_bytes;  // full serialized for packetizer
  uint32_t total_bytes() const {
    return static_cast<uint32_t>(mv_bytes.size() + coeff_bytes.size());
  }
};

/// Bitstream writer: pack bits into buffer
class BitstreamWriter {
 public:
  BitstreamWriter() = default;
  void reset();
  void write_bits(uint32_t value, int num_bits);
  void write_byte(uint8_t b);
  void write_bytes(const uint8_t* data, size_t len);
  void flush_byte_align();
  const std::vector<uint8_t>& buffer() const { return buffer_; }
  size_t bit_position() const { return bit_pos_; }
  size_t byte_position() const { return (bit_pos_ + 7) / 8; }

 private:
  std::vector<uint8_t> buffer_;
  size_t bit_pos_ = 0;
};

/// Bitstream reader for decoder and roundtrip tests
class BitstreamReader {
 public:
  BitstreamReader() = default;
  void set_data(const uint8_t* data, size_t size_bytes);
  void set_data(const std::vector<uint8_t>& data) { set_data(data.data(), data.size()); }
  uint32_t read_bits(int num_bits);
  uint8_t read_byte();
  void read_bytes(uint8_t* out, size_t len);
  void align_to_byte();
  bool eof() const;
  size_t bit_position() const { return bit_pos_; }
  size_t byte_position() const { return (bit_pos_ + 7) / 8; }
  size_t size_bytes() const { return size_bytes_; }

 private:
  const uint8_t* data_ = nullptr;
  size_t size_bytes_ = 0;
  size_t bit_pos_ = 0;
};

}  // namespace codec
}  // namespace telehealth
