#include <io/FileBitstreamSink.h>
#include <cstring>
#include <cerrno>

namespace telehealth {
namespace io {

FileBitstreamSink::~FileBitstreamSink() {
  close();
}

bool FileBitstreamSink::open(const std::string& path) {
  close();
  file_ = std::fopen(path.c_str(), "wb");
  return file_ != nullptr;
}

void FileBitstreamSink::close() {
  if (file_) {
    std::fclose(file_);
    file_ = nullptr;
  }
}

bool FileBitstreamSink::write_file_header(const codec::BitstreamFileHeader& h) {
  if (!file_) return false;
  return std::fwrite(&h, sizeof(h), 1, file_) == 1;
}

bool FileBitstreamSink::write_frame(const codec::BitstreamFrameHeader& h,
                                    const uint8_t* mv_data, size_t mv_len,
                                    const uint8_t* coeff_data, size_t coeff_len) {
  if (!file_) return false;
  if (std::fwrite(&h, sizeof(h), 1, file_) != 1) return false;
  if (mv_len && std::fwrite(mv_data, 1, mv_len, file_) != mv_len) return false;
  if (coeff_len && std::fwrite(coeff_data, 1, coeff_len, file_) != coeff_len) return false;
  return true;
}

bool FileBitstreamSink::write_frame(const codec::EncodedFrame& frame) {
  codec::BitstreamFrameHeader h;
  h.frame_type = frame.type == codec::FrameType::I ? 0 : 1;
  h.frame_id = frame.frame_id;
  h.timestamp_us = frame.timestamp_us;
  h.qp = frame.qp;
  h.mv_payload_bytes = static_cast<uint32_t>(frame.mv_bytes.size());
  h.coeff_payload_bytes = static_cast<uint32_t>(frame.coeff_bytes.size());
  return write_frame(h, frame.mv_bytes.data(), frame.mv_bytes.size(),
                     frame.coeff_bytes.data(), frame.coeff_bytes.size());
}

}  // namespace io
}  // namespace telehealth
