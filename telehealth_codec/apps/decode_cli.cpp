#include <codec/Bitstream.h>
#include <codec/Frame.h>
#include <io/FileBitstreamSink.h>
#include <util/Logger.h>
#include <fstream>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  std::string input_path = "output.bin";
  std::string output_path = "decoded.yuv";

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-i" && i + 1 < argc) { input_path = argv[++i]; continue; }
    if (arg == "-o" && i + 1 < argc) { output_path = argv[++i]; continue; }
    if (arg == "--help") {
      std::cerr << "Usage: " << argv[0] << " [-i input.bin] [-o decoded.yuv]\n";
      return 0;
    }
  }

  std::ifstream in(input_path, std::ios::binary);
  if (!in) {
    TELECODEC_LOG_ERROR("Cannot open " << input_path);
    return 1;
  }
  in.seekg(0, std::ios::end);
  size_t file_size = in.tellg();
  in.seekg(0);
  std::vector<uint8_t> data(file_size);
  if (!in.read(reinterpret_cast<char*>(data.data()), file_size)) {
    TELECODEC_LOG_ERROR("Read failed");
    return 1;
  }

  telehealth::codec::BitstreamFileHeader fh;
  if (data.size() < sizeof(fh)) {
    TELECODEC_LOG_ERROR("File too small for header");
    return 1;
  }
  std::memcpy(&fh, data.data(), sizeof(fh));
  if (fh.magic != 0x54434F44) {
    TELECODEC_LOG_ERROR("Invalid magic");
    return 1;
  }

  TELECODEC_LOG_INFO("Bitstream " << fh.width << "x" << fh.height << " fps=" << (int)fh.fps);

  FILE* out_file = std::fopen(output_path.c_str(), "wb");
  if (!out_file) {
    TELECODEC_LOG_ERROR("Cannot open " << output_path);
    return 1;
  }

  size_t offset = sizeof(fh);
  int frame_count = 0;
  while (offset + sizeof(telehealth::codec::BitstreamFrameHeader) <= data.size()) {
    telehealth::codec::BitstreamFrameHeader h;
    std::memcpy(&h, data.data() + offset, sizeof(h));
    offset += sizeof(h);
    size_t payload = h.mv_payload_bytes + h.coeff_payload_bytes;
    if (offset + payload > data.size()) break;
    offset += payload;
    frame_count++;
  }

  std::fclose(out_file);
  TELECODEC_LOG_INFO("Decode CLI: detected " << frame_count << " frames (decode path minimal; output is metadata only)");
  return 0;
}
