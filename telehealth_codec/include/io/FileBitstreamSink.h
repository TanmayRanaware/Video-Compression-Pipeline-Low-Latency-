#pragma once

#include <codec/Bitstream.h>
#include <string>
#include <cstdio>

namespace telehealth {
namespace io {

class FileBitstreamSink {
 public:
  FileBitstreamSink() = default;
  ~FileBitstreamSink();
  bool open(const std::string& path);
  void close();
  bool write_file_header(const codec::BitstreamFileHeader& h);
  bool write_frame(const codec::BitstreamFrameHeader& h,
                   const uint8_t* mv_data, size_t mv_len,
                   const uint8_t* coeff_data, size_t coeff_len);
  bool write_frame(const codec::EncodedFrame& frame);
  bool is_open() const { return file_ != nullptr; }

 private:
  FILE* file_ = nullptr;
};

}  // namespace io
}  // namespace telehealth
