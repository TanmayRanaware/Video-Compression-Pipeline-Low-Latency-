#include <codec/Bitstream.h>
#include <algorithm>
#include <cstring>

namespace telehealth {
namespace codec {

void BitstreamWriter::reset() {
  buffer_.clear();
  bit_pos_ = 0;
}

void BitstreamWriter::write_bits(uint32_t value, int num_bits) {
  if (num_bits <= 0 || num_bits > 32) return;
  size_t need_bytes = (bit_pos_ + num_bits + 7) / 8;
  if (buffer_.size() < need_bytes)
    buffer_.resize(need_bytes, 0);
  uint8_t* p = buffer_.data();
  uint32_t v = value & ((1u << num_bits) - 1);
  for (int i = 0; i < num_bits; ++i) {
    if (v & (1u << i))
      p[(bit_pos_ + i) / 8] |= (1 << ((bit_pos_ + i) % 8));
  }
  bit_pos_ += num_bits;
}

void BitstreamWriter::write_byte(uint8_t b) {
  flush_byte_align();
  size_t byte_idx = bit_pos_ / 8;
  if (byte_idx >= buffer_.size())
    buffer_.resize(byte_idx + 1, 0);
  buffer_[byte_idx] = b;
  bit_pos_ += 8;
}

void BitstreamWriter::write_bytes(const uint8_t* data, size_t len) {
  flush_byte_align();
  size_t byte_idx = bit_pos_ / 8;
  if (buffer_.size() < byte_idx + len)
    buffer_.resize(byte_idx + len, 0);
  std::memcpy(buffer_.data() + byte_idx, data, len);
  bit_pos_ += len * 8;
}

void BitstreamWriter::flush_byte_align() {
  if (bit_pos_ % 8 != 0)
    bit_pos_ = (bit_pos_ + 7) & ~7;
  if (bit_pos_ / 8 > buffer_.size())
    buffer_.resize(bit_pos_ / 8, 0);
}

void BitstreamReader::set_data(const uint8_t* data, size_t size_bytes) {
  data_ = data;
  size_bytes_ = size_bytes;
  bit_pos_ = 0;
}

uint32_t BitstreamReader::read_bits(int num_bits) {
  if (num_bits <= 0 || num_bits > 32) return 0;
  uint32_t v = 0;
  for (int i = 0; i < num_bits; ++i) {
    size_t byte_idx = (bit_pos_ + i) / 8;
    if (byte_idx >= size_bytes_) break;
    if (data_[byte_idx] & (1 << ((bit_pos_ + i) % 8)))
      v |= (1u << i);
  }
  bit_pos_ += num_bits;
  return v;
}

uint8_t BitstreamReader::read_byte() {
  align_to_byte();
  size_t byte_idx = bit_pos_ / 8;
  uint8_t b = byte_idx < size_bytes_ ? data_[byte_idx] : 0;
  bit_pos_ += 8;
  return b;
}

void BitstreamReader::read_bytes(uint8_t* out, size_t len) {
  align_to_byte();
  size_t byte_idx = bit_pos_ / 8;
  size_t avail = size_bytes_ > byte_idx ? size_bytes_ - byte_idx : 0;
  size_t copy = std::min(len, avail);
  if (copy) std::memcpy(out, data_ + byte_idx, copy);
  if (copy < len) std::memset(out + copy, 0, len - copy);
  bit_pos_ += len * 8;
}

void BitstreamReader::align_to_byte() {
  if (bit_pos_ % 8 != 0)
    bit_pos_ = (bit_pos_ + 7) & ~7;
}

bool BitstreamReader::eof() const {
  return (bit_pos_ + 7) / 8 >= size_bytes_;
}

}  // namespace codec
}  // namespace telehealth
