#include <codec/Block.h>
#include <codec/Frame.h>
#include <iostream>

int main() {
  telehealth::codec::FrameYUV frame(64, 64);
  int count = 0;
  telehealth::codec::for_each_macroblock(frame, [&count](telehealth::codec::BlockCoord coord,
                                                          telehealth::codec::BlockView y,
                                                          telehealth::codec::BlockView u,
                                                          telehealth::codec::BlockView v) {
    if (!y.valid() || !u.valid() || !v.valid()) {
      std::cerr << "Invalid block view at " << coord.mb_x << "," << coord.mb_y << "\n";
      return;
    }
    count++;
  });
  int expected = (64 / 16) * (64 / 16);
  if (count != expected) {
    std::cerr << "Expected " << expected << " macroblocks, got " << count << "\n";
    return 1;
  }
  std::cout << "Block iterator test OK (" << count << " macroblocks)\n";
  return 0;
}
