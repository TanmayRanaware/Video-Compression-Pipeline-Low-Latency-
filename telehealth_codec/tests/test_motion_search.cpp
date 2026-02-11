#include <codec/Frame.h>
#include <codec/Block.h>
#include <codec/MotionEstimation.h>
#include <codec/EncoderConfig.h>
#include <iostream>
#include <cstring>

int main() {
  telehealth::codec::EncoderConfig config;
  config.width = 64;
  config.height = 64;
  config.search_range = 8;

  telehealth::codec::FrameYUV cur(64, 64);
  telehealth::codec::FrameYUV ref(64, 64);
  for (int i = 0; i < 64 * 64; ++i) {
    cur.y_plane[i] = static_cast<uint8_t>(i % 256);
    ref.y_plane[i] = static_cast<uint8_t>((i + 3) % 256);
  }

  telehealth::codec::MotionEstimation me(config);
  telehealth::codec::BlockViewConst bcur(cur.y_plane.data(), cur.stride_y, 16, 16);
  telehealth::codec::BlockCoord pos{1, 1};
  auto result = me.estimate(bcur, ref, pos);
  if (result.cost == 0xFFFFFFFFu) {
    std::cerr << "Motion search returned invalid cost\n";
    return 1;
  }
  std::cout << "Motion search test OK (mv=(" << result.mv.dx << "," << result.mv.dy << ") cost=" << result.cost << ")\n";
  return 0;
}
