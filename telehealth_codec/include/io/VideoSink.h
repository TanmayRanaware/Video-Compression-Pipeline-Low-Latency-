#pragma once

#include <codec/Frame.h>
#include <cstdint>
#include <vector>
#include <string>

namespace telehealth {
namespace io {

class VideoSink {
 public:
  virtual ~VideoSink() = default;
  virtual bool write(const codec::FrameYUV& frame, const codec::FrameMeta& meta) = 0;
  virtual bool open(const std::string& path, int width, int height, int fps) = 0;
  virtual void close() = 0;
};

/// Writes raw YUV frames to file (e.g. for PSNR testing)
class FileYuvSink : public VideoSink {
 public:
  bool write(const codec::FrameYUV& frame, const codec::FrameMeta& meta) override;
  bool open(const std::string& path, int width, int height, int fps) override;
  void close() override;

 private:
  std::string path_;
  FILE* file_ = nullptr;
  int width_ = 0, height_ = 0;
};

}  // namespace io
}  // namespace telehealth
