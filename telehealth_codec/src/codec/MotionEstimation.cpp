#include <codec/MotionEstimation.h>
#include <algorithm>
#include <cmath>
#include <limits>

namespace telehealth {
namespace codec {

MotionEstimation::MotionEstimation(const EncoderConfig& config) : config_(config) {}

bool MotionEstimation::in_bounds(const FrameYUV& frame, int x, int y, int w, int h) const {
  return x >= 0 && y >= 0 && x + w <= frame.width && y + h <= frame.height;
}

bool MotionEstimation::in_bounds(const Frame& frame, int x, int y, int w, int h) const {
  return x >= 0 && y >= 0 && x + w <= frame.width() && y + h <= frame.height();
}

uint32_t MotionEstimation::sad_block(const BlockViewConst& cur, const BlockViewConst& ref) const {
  uint32_t sad = 0;
  const int h = std::min(cur.h, ref.h);
  const int w = std::min(cur.w, ref.w);
  for (int y = 0; y < h; ++y) {
    const uint8_t* p_cur = cur.row(y);
    const uint8_t* p_ref = ref.row(y);
    for (int x = 0; x < w; ++x)
      sad += std::abs(static_cast<int>(p_cur[x]) - static_cast<int>(p_ref[x]));
  }
  return sad;
}

MotionResult MotionEstimation::estimate(const BlockViewConst& cur_block,
                                        const FrameYUV& ref_frame,
                                        BlockCoord pos) const {
  const int range = config_.search_range;
  const int base_x = pos.mb_x * MB_SIZE;
  const int base_y = pos.mb_y * MB_SIZE;

  MotionResult best;
  best.cost = 0xFFFFFFFFu;

  for (int dy = -range; dy <= range; ++dy) {
    for (int dx = -range; dx <= range; ++dx) {
      int ref_x = base_x + dx;
      int ref_y = base_y + dy;
      if (!in_bounds(ref_frame, ref_x, ref_y, cur_block.w, cur_block.h))
        continue;

      BlockViewConst ref_block(ref_frame.y_plane.data() + ref_y * ref_frame.stride_y + ref_x,
                               ref_frame.stride_y, cur_block.w, cur_block.h);
      uint32_t cost = sad_block(cur_block, ref_block);
      if (cost < best.cost) {
        best.cost = cost;
        best.mv.dx = static_cast<int16_t>(dx);
        best.mv.dy = static_cast<int16_t>(dy);
      }
    }
  }
  return best;
}

MotionResult MotionEstimation::estimate(const BlockViewConst& cur_block,
                                        const Frame& ref_frame,
                                        BlockCoord pos) const {
  const int range = config_.search_range;
  const int base_x = pos.mb_x * MB_SIZE;
  const int base_y = pos.mb_y * MB_SIZE;

  MotionResult best;
  best.cost = 0xFFFFFFFFu;

  for (int dy = -range; dy <= range; ++dy) {
    for (int dx = -range; dx <= range; ++dx) {
      int ref_x = base_x + dx;
      int ref_y = base_y + dy;
      if (!in_bounds(ref_frame, ref_x, ref_y, cur_block.w, cur_block.h))
        continue;

      BlockViewConst ref_block(ref_frame.y_plane_ptr() + ref_y * ref_frame.stride_y() + ref_x,
                               ref_frame.stride_y(), cur_block.w, cur_block.h);
      uint32_t cost = sad_block(cur_block, ref_block);
      if (cost < best.cost) {
        best.cost = cost;
        best.mv.dx = static_cast<int16_t>(dx);
        best.mv.dy = static_cast<int16_t>(dy);
      }
    }
  }
  return best;
}

MotionResult MotionEstimation::estimate_diamond(const BlockViewConst& cur_block,
                                                const FrameYUV& ref_frame,
                                                BlockCoord pos) const {
  const int range = config_.search_range;
  const int base_x = pos.mb_x * MB_SIZE;
  const int base_y = pos.mb_y * MB_SIZE;

  int cx = 0, cy = 0;
  MotionResult best;
  best.mv.dx = 0;
  best.mv.dy = 0;
  best.cost = 0xFFFFFFFFu;

  auto check = [&](int dx, int dy) {
    int ref_x = base_x + dx;
    int ref_y = base_y + dy;
    if (!in_bounds(ref_frame, ref_x, ref_y, cur_block.w, cur_block.h)) return;
    BlockViewConst ref_block(ref_frame.y_plane.data() + ref_y * ref_frame.stride_y + ref_x,
                             ref_frame.stride_y, cur_block.w, cur_block.h);
    uint32_t cost = sad_block(cur_block, ref_block);
    if (cost < best.cost) {
      best.cost = cost;
      best.mv.dx = static_cast<int16_t>(dx);
      best.mv.dy = static_cast<int16_t>(dy);
      cx = dx;
      cy = dy;
    }
  };

  check(0, 0);
  int step = range;
  while (step > 0) {
    check(cx + step, cy);
    check(cx - step, cy);
    check(cx, cy + step);
    check(cx, cy - step);
    check(cx + step, cy + step);
    check(cx + step, cy - step);
    check(cx - step, cy + step);
    check(cx - step, cy - step);
    step /= 2;
  }
  return best;
}

MotionResult MotionEstimation::estimate_diamond(const BlockViewConst& cur_block,
                                                const Frame& ref_frame,
                                                BlockCoord pos) const {
  const int range = config_.search_range;
  const int base_x = pos.mb_x * MB_SIZE;
  const int base_y = pos.mb_y * MB_SIZE;

  int cx = 0, cy = 0;
  MotionResult best;
  best.mv.dx = 0;
  best.mv.dy = 0;
  best.cost = 0xFFFFFFFFu;

  auto check = [&](int dx, int dy) {
    int ref_x = base_x + dx;
    int ref_y = base_y + dy;
    if (!in_bounds(ref_frame, ref_x, ref_y, cur_block.w, cur_block.h)) return;
    BlockViewConst ref_block(ref_frame.y_plane_ptr() + ref_y * ref_frame.stride_y() + ref_x,
                             ref_frame.stride_y(), cur_block.w, cur_block.h);
    uint32_t cost = sad_block(cur_block, ref_block);
    if (cost < best.cost) {
      best.cost = cost;
      best.mv.dx = static_cast<int16_t>(dx);
      best.mv.dy = static_cast<int16_t>(dy);
      cx = dx;
      cy = dy;
    }
  };

  check(0, 0);
  int step = range;
  while (step > 0) {
    check(cx + step, cy);
    check(cx - step, cy);
    check(cx, cy + step);
    check(cx, cy - step);
    check(cx + step, cy + step);
    check(cx + step, cy - step);
    check(cx - step, cy + step);
    check(cx - step, cy - step);
    step /= 2;
  }
  return best;
}

}  // namespace codec
}  // namespace telehealth
