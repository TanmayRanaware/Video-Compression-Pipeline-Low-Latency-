#include <io/UdpReceiver.h>
#include <io/FileBitstreamSink.h>
#include <codec/Bitstream.h>
#include <util/Logger.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <chrono>
#include <thread>

int main(int argc, char** argv) {
  uint16_t port = 5000;
  std::string output_path = "received.bin";

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-p" && i + 1 < argc) { port = static_cast<uint16_t>(std::atoi(argv[++i])); continue; }
    if (arg == "-o" && i + 1 < argc) { output_path = argv[++i]; continue; }
    if (arg == "--help") {
      std::cerr << "Usage: " << argv[0] << " [-p port] [-o output.bin]\n";
      return 0;
    }
  }

  telehealth::io::UdpReceiver receiver;
  if (!receiver.open(port)) {
    TELECODEC_LOG_ERROR("Failed to open UDP port " << port);
    return 1;
  }

  telehealth::io::FileBitstreamSink file_sink;
  bool header_written = false;
  int frame_count = 0;

  receiver.set_frame_callback([&](uint32_t frame_id, uint64_t timestamp_us, const uint8_t* data, size_t size) {
    (void)timestamp_us;
    if (size == 0) return;
    if (!header_written) {
      telehealth::codec::BitstreamFileHeader h;
      h.magic = 0x54434F44;
      h.version = 1;
      h.width = 640;
      h.height = 480;
      h.fps = 30;
      if (!file_sink.is_open() && !file_sink.open(output_path)) return;
      file_sink.write_file_header(h);
      header_written = true;
    }
    telehealth::codec::EncodedFrame ef;
    ef.frame_id = frame_id;
    ef.timestamp_us = timestamp_us;
    ef.raw_bytes.assign(data, data + size);
    size_t half = size / 2;
    ef.mv_bytes.assign(data, data + half);
    ef.coeff_bytes.assign(data + half, data + size);
    if (file_sink.is_open())
      file_sink.write_frame(ef);
    frame_count++;
    if (frame_count % 30 == 0)
      TELECODEC_LOG_INFO("Received frame " << frame_count);
  });

  TELECODEC_LOG_INFO("Listening on port " << port << ", writing to " << output_path);
  for (int i = 0; i < 600; ++i) {
    receiver.poll(100);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  file_sink.close();
  receiver.close();
  TELECODEC_LOG_INFO("Received " << frame_count << " frames");
  return 0;
}
