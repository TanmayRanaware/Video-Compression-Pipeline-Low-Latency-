#include <io/VideoSink.h>
#include <cstdio>

namespace telehealth {
namespace io {

bool FileYuvSink::write(const codec::FrameYUV& frame, const codec::FrameMeta& meta) {
  (void)meta;
  if (!file_) return false;
  for (int y = 0; y < frame.height; ++y)
    if (std::fwrite(frame.y_row(y), 1, static_cast<size_t>(frame.width), file_) != static_cast<size_t>(frame.width))
      return false;
  for (int y = 0; y < frame.height / 2; ++y) {
    if (std::fwrite(frame.u_row(y), 1, static_cast<size_t>(frame.width / 2), file_) != static_cast<size_t>(frame.width / 2))
      return false;
    if (std::fwrite(frame.v_row(y), 1, static_cast<size_t>(frame.width / 2), file_) != static_cast<size_t>(frame.width / 2))
      return false;
  }
  return true;
}

bool FileYuvSink::open(const std::string& path, int width, int height, int fps) {
  (void)fps;
  close();
  path_ = path;
  width_ = width;
  height_ = height;
  file_ = std::fopen(path.c_str(), "wb");
  return file_ != nullptr;
}

void FileYuvSink::close() {
  if (file_) {
    std::fclose(file_);
    file_ = nullptr;
  }
}

}  // namespace io
}  // namespace telehealth
