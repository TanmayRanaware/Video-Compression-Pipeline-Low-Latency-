#include <codec/Frame.h>
#include <codec/Block.h>
#include <codec/MotionEstimation.h>
#include <codec/EncoderConfig.h>
#include <util/Timer.h>
#include <iostream>
#include <cstdlib>

int main() {
  const int w = 320, h = 240;
  telehealth::codec::EncoderConfig config;
  config.width = w;
  config.height = h;
  config.search_range = 16;

  telehealth::codec::FrameYUV cur(w, h);
  telehealth::codec::FrameYUV ref(w, h);
  for (size_t i = 0; i < cur.y_plane.size(); ++i)
    cur.y_plane[i] = static_cast<uint8_t>(rand() % 256);
  for (size_t i = 0; i < ref.y_plane.size(); ++i)
    ref.y_plane[i] = static_cast<uint8_t>(rand() % 256);

  telehealth::codec::MotionEstimation me(config);
  int mb_cols = (w + 15) / 16;
  int mb_rows = (h + 15) / 16;

  telehealth::util::Timer t;
  t.start();
  int iterations = 10;
  for (int it = 0; it < iterations; ++it) {
    telehealth::codec::for_each_macroblock_const(cur, [&](telehealth::codec::BlockCoord coord,
                                                         telehealth::codec::BlockViewConst yv,
                                                         telehealth::codec::BlockViewConst,
                                                         telehealth::codec::BlockViewConst) {
      (void)me.estimate(yv, ref, coord);
    });
  }
  t.stop();
  double ms = t.elapsed_ms();
  int mbs = mb_cols * mb_rows * iterations;
  std::cout << "Motion search: " << ms << " ms for " << mbs << " MBs (" << (mbs / (ms / 1000.0)) << " MB/s)\n";
  return 0;
}
