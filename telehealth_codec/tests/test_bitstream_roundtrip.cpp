#include <codec/Encoder.h>
#include <codec/EncoderConfig.h>
#include <codec/Frame.h>
#include <codec/Bitstream.h>
#include <codec/YuvConverter.h>
#include <io/VideoSource.h>
#include <io/FileBitstreamSink.h>
#include <iostream>
#include <cstdio>

int main() {
  telehealth::io::VideoSourceConfig src_cfg;
  src_cfg.path = "synthetic";
  src_cfg.width = 64;
  src_cfg.height = 64;
  src_cfg.fps = 30;
  auto source = telehealth::io::create_video_source(src_cfg);
  if (!source || !source->opened()) {
    std::cerr << "Failed to create synthetic source\n";
    return 1;
  }

  telehealth::codec::EncoderConfig enc_cfg;
  enc_cfg.width = 64;
  enc_cfg.height = 64;
  enc_cfg.fps = 30;
  enc_cfg.gop_size = 30;
  telehealth::codec::Encoder encoder(enc_cfg);

  const char* path = "/tmp/telehealth_roundtrip_test.bin";
  telehealth::io::FileBitstreamSink sink;
  if (!sink.open(path)) {
    std::cerr << "Failed to open temp file\n";
    return 1;
  }

  telehealth::codec::BitstreamFileHeader fh;
  fh.width = 64;
  fh.height = 64;
  fh.fps = 30;
  sink.write_file_header(fh);

  telehealth::codec::FrameRGB rgb;
  telehealth::codec::FrameMeta meta;
  telehealth::codec::YuvConverter conv;
  int encoded = 0;
  for (int i = 0; i < 5; ++i) {
    if (!source->read(rgb, meta)) break;
    telehealth::codec::FrameYUV yuv;
    conv.rgb_to_yuv420(rgb, yuv);
    auto ef = encoder.encode(yuv, meta);
    if (!sink.write_frame(ef)) break;
    encoded++;
  }
  sink.close();

  if (encoded < 2) {
    std::cerr << "Encoded too few frames\n";
    return 1;
  }

  std::FILE* f = std::fopen(path, "rb");
  if (!f) {
    std::cerr << "Cannot reopen bitstream file\n";
    return 1;
  }
  std::fseek(f, 0, SEEK_END);
  long sz = std::ftell(f);
  std::fseek(f, 0, SEEK_SET);
  if (sz < static_cast<long>(sizeof(telehealth::codec::BitstreamFileHeader))) {
    std::fclose(f);
    std::cerr << "File too small\n";
    return 1;
  }
  telehealth::codec::BitstreamFileHeader read_fh;
  std::fread(&read_fh, 1, sizeof(read_fh), f);
  std::fclose(f);
  std::remove(path);

  if (read_fh.magic != 0x54434F44 || read_fh.width != 64 || read_fh.height != 64) {
    std::cerr << "File header mismatch\n";
    return 1;
  }

  std::cout << "Bitstream roundtrip test OK (encoded " << encoded << " frames)\n";
  return 0;
}
