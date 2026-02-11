#include <codec/Encoder.h>
#include <codec/EncoderConfig.h>
#include <codec/YuvConverter.h>
#include <io/VideoSource.h>
#include <io/FileBitstreamSink.h>
#include <util/Logger.h>
#include <iostream>
#include <string>
#include <cstdlib>

int main(int argc, char** argv) {
  std::string input_path = "synthetic";
  std::string output_path = "output.bin";
  int width = 640, height = 480, fps = 30, qp = 28, gop = 30;
  int max_frames = 100;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-i" && i + 1 < argc) { input_path = argv[++i]; continue; }
    if (arg == "-o" && i + 1 < argc) { output_path = argv[++i]; continue; }
    if (arg == "-w" && i + 1 < argc) { width = std::atoi(argv[++i]); continue; }
    if (arg == "-h" && i + 1 < argc) { height = std::atoi(argv[++i]); continue; }
    if (arg == "-fps" && i + 1 < argc) { fps = std::atoi(argv[++i]); continue; }
    if (arg == "-qp" && i + 1 < argc) { qp = std::atoi(argv[++i]); continue; }
    if (arg == "-gop" && i + 1 < argc) { gop = std::atoi(argv[++i]); continue; }
    if (arg == "-n" && i + 1 < argc) { max_frames = std::atoi(argv[++i]); continue; }
    if (arg == "--help") {
      std::cerr << "Usage: " << argv[0] << " [-i input|synthetic] [-o output.bin] [-w width] [-h height] [-fps fps] [-qp qp] [-gop gop] [-n max_frames]\n";
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
    TELECODEC_LOG_ERROR("Failed to open video source: " << input_path);
    return 1;
  }

  telehealth::codec::EncoderConfig enc_cfg;
  enc_cfg.width = source->width();
  enc_cfg.height = source->height();
  enc_cfg.fps = source->fps();
  enc_cfg.qp_default = qp;
  enc_cfg.gop_size = gop;

  telehealth::codec::Encoder encoder(enc_cfg);
  telehealth::io::FileBitstreamSink sink;
  if (!sink.open(output_path)) {
    TELECODEC_LOG_ERROR("Failed to open output: " << output_path);
    return 1;
  }

  telehealth::codec::BitstreamFileHeader file_header;
  file_header.width = static_cast<uint16_t>(enc_cfg.width);
  file_header.height = static_cast<uint16_t>(enc_cfg.height);
  file_header.fps = static_cast<uint8_t>(enc_cfg.fps);
  if (!sink.write_file_header(file_header)) {
    TELECODEC_LOG_ERROR("Failed to write file header");
    return 1;
  }

  telehealth::codec::FrameRGB rgb;
  telehealth::codec::FrameMeta meta;
  telehealth::codec::YuvConverter converter;
  int count = 0;
  while (count < max_frames && source->read(rgb, meta)) {
    telehealth::codec::FrameYUV yuv;
    converter.rgb_to_yuv420(rgb, yuv);
    auto encoded = encoder.encode(yuv, meta);
    if (!sink.write_frame(encoded)) {
      TELECODEC_LOG_ERROR("Failed to write frame " << count);
      break;
    }
    if (count % 30 == 0)
      TELECODEC_LOG_INFO("Encoded frame " << count << " (" << encoded.total_bytes() << " bytes)");
    count++;
  }

  sink.close();
  TELECODEC_LOG_INFO("Encoded " << count << " frames to " << output_path);
  return 0;
}
