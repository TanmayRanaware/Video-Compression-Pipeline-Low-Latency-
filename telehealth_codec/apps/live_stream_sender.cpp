#include <pipeline/Pipeline.h>
#include <codec/Encoder.h>
#include <codec/EncoderConfig.h>
#include <codec/YuvConverter.h>
#include <io/VideoSource.h>
#include <io/UdpPacketSink.h>
#include <util/Logger.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>
#include <chrono>

int main(int argc, char** argv) {
  std::string input_path = "synthetic";
  std::string host = "127.0.0.1";
  uint16_t port = 5000;
  int width = 640, height = 480, fps = 30;
  int max_frames = 300;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-i" && i + 1 < argc) { input_path = argv[++i]; continue; }
    if (arg == "-h" && i + 1 < argc) { host = argv[++i]; continue; }
    if (arg == "-p" && i + 1 < argc) { port = static_cast<uint16_t>(std::atoi(argv[++i])); continue; }
    if (arg == "-w" && i + 1 < argc) { width = std::atoi(argv[++i]); continue; }
    if (arg == "--height" && i + 1 < argc) { height = std::atoi(argv[++i]); continue; }
    if (arg == "-fps" && i + 1 < argc) { fps = std::atoi(argv[++i]); continue; }
    if (arg == "-n" && i + 1 < argc) { max_frames = std::atoi(argv[++i]); continue; }
    if (arg == "--help") {
      std::cerr << "Usage: " << argv[0] << " [-i input] [-h host] [-p port] [-w width] [--height H] [-fps fps] [-n max_frames]\n";
      return 0;
    }
  }

  telehealth::io::VideoSourceConfig src_cfg;
  src_cfg.path = input_path;
  src_cfg.width = width;
  src_cfg.height = height;
  src_cfg.fps = fps;
  auto source = telehealth::io::create_video_source(src_cfg);
  if (!source || !source->opened()) {
    TELECODEC_LOG_ERROR("Failed to open video source");
    return 1;
  }

  telehealth::io::UdpPacketSink udp;
  if (!udp.open(host, port)) {
    TELECODEC_LOG_ERROR("Failed to open UDP socket to " << host << ":" << port);
    return 1;
  }

  telehealth::pipeline::Pipeline::Config pipe_cfg;
  pipe_cfg.width = source->width();
  pipe_cfg.height = source->height();
  pipe_cfg.fps = source->fps();
  telehealth::pipeline::Pipeline pipeline(pipe_cfg);
  pipeline.start();

  telehealth::codec::FrameRGB rgb;
  telehealth::codec::FrameMeta meta;
  int count = 0;
  int frame_interval_ms = 1000 / (fps > 0 ? fps : 30);

  while (count < max_frames && source->read(rgb, meta)) {
    telehealth::pipeline::CaptureItem item;
    item.frame = telehealth::codec::Frame::from_rgb(rgb, meta.frame_id, meta.timestamp_us, meta.pts_sec);
    pipeline.push_capture(std::move(item));

    telehealth::pipeline::EncodedItem enc;
    if (pipeline.pop_encoded(enc, 500)) {
      std::vector<uint8_t> raw = enc.frame.raw_bytes;
      if (raw.empty()) {
        raw.insert(raw.end(), enc.frame.mv_bytes.begin(), enc.frame.mv_bytes.end());
        raw.insert(raw.end(), enc.frame.coeff_bytes.begin(), enc.frame.coeff_bytes.end());
      }
      if (!raw.empty() && !udp.send_frame(raw, enc.frame.frame_id, enc.frame.timestamp_us, 0))
        TELECODEC_LOG_WARN("UDP send failed for frame " << enc.frame.frame_id);
      count++;
      if (count % 30 == 0)
        TELECODEC_LOG_INFO("Sent frame " << count);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(frame_interval_ms));
  }

  pipeline.stop();
  udp.close();
  TELECODEC_LOG_INFO("Sent " << count << " frames");
  return 0;
}
