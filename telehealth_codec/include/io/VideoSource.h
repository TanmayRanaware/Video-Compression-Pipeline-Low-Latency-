#pragma once

#include <codec/Frame.h>
#include <string>
#include <memory>

namespace telehealth {
namespace io {

struct VideoSourceConfig {
  std::string path;       // file path or empty for camera
  std::string device;     // camera device if path empty
  int width = 640;
  int height = 480;
  int fps = 30;
  bool use_file = true;
};

class VideoSource {
 public:
  virtual ~VideoSource() = default;
  virtual bool read(codec::FrameRGB& out, codec::FrameMeta& meta) = 0;
  virtual bool opened() const = 0;
  virtual int width() const = 0;
  virtual int height() const = 0;
  virtual int fps() const = 0;
};

std::unique_ptr<VideoSource> create_video_source(const VideoSourceConfig& config);

}  // namespace io
}  // namespace telehealth
