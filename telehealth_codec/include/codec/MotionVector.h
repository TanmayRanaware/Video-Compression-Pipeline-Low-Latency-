#pragma once

#include <cstdint>

namespace telehealth {
namespace codec {

/// Integer-pel motion vector (dx, dy) in pixels
struct MotionVector {
  int16_t dx = 0;
  int16_t dy = 0;

  MotionVector() = default;
  MotionVector(int16_t x, int16_t y) : dx(x), dy(y) {}
};

/// Motion vector plus matching cost (e.g. SAD)
struct MotionResult {
  MotionVector mv;
  uint32_t cost = 0;  // SAD or similar
};

}  // namespace codec
}  // namespace telehealth
