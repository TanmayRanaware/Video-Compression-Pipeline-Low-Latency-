#include <codec/Encoder.h>
#include <codec/EncoderConfig.h>
#include <codec/YuvConverter.h>
#include <io/VideoSource.h>
#include <util/Timer.h>
#include <iostream>

int main() {
  telehealth::io::VideoSourceConfig src_cfg;
  src_cfg.path = "synthetic";
  src_cfg.width = 320;
  src_cfg.height = 240;
  src_cfg.fps = 30;
  auto source = telehealth::io::create_video_source(src_cfg);
  if (!source || !source->opened()) {
    std::cerr << "Failed to create source\n";
    return 1;
  }

  telehealth::codec::EncoderConfig enc_cfg;
  enc_cfg.width = 320;
  enc_cfg.height = 240;
  enc_cfg.fps = 30;
  enc_cfg.gop_size = 30;
  telehealth::codec::Encoder encoder(enc_cfg);
  telehealth::codec::YuvConverter conv;

  telehealth::codec::FrameRGB rgb;
  telehealth::codec::FrameMeta meta;
  int frames = 0;
  size_t total_bytes = 0;
  telehealth::util::Timer t;
  t.start();
  while (frames < 60 && source->read(rgb, meta)) {
    telehealth::codec::FrameYUV yuv;
    conv.rgb_to_yuv420(rgb, yuv);
    auto ef = encoder.encode(yuv, meta);
    total_bytes += ef.total_bytes();
    frames++;
  }
  t.stop();
  double ms = t.elapsed_ms();
  std::cout << "End-to-end: " << frames << " frames in " << ms << " ms ("
            << (frames / (ms / 1000.0)) << " fps, "
            << (total_bytes * 8 / (ms / 1000.0) / 1000.0) << " kbps)\n";
  return 0;
}
