#pragma once

#include "Frame.h"
#include <cstdint>
#include <functional>

namespace telehealth {
namespace codec {

/// View into a block of pixels (no copy). Used for macroblocks and sub-blocks.
struct BlockView {
  uint8_t* ptr = nullptr;
  int stride = 0;
  int w = 0;
  int h = 0;

  BlockView() = default;
  BlockView(uint8_t* p, int s, int width, int height) : ptr(p), stride(s), w(width), h(height) {}
  bool valid() const { return ptr != nullptr && stride > 0 && w > 0 && h > 0; }
  uint8_t* row(int r) { return ptr + r * stride; }
  const uint8_t* row(int r) const { return ptr + r * stride; }
};

/// Const block view
struct BlockViewConst {
  const uint8_t* ptr = nullptr;
  int stride = 0;
  int w = 0;
  int h = 0;

  BlockViewConst() = default;
  BlockViewConst(const uint8_t* p, int s, int width, int height) : ptr(p), stride(s), w(width), h(height) {}
  bool valid() const { return ptr != nullptr && stride > 0 && w > 0 && h > 0; }
  const uint8_t* row(int r) const { return ptr + r * stride; }
};

/// Coordinate of a macroblock (in macroblock units, not pixels)
struct BlockCoord {
  int mb_x = 0;
  int mb_y = 0;
};

/// Macroblock size: 16x16 luma; in 4:2:0, chroma is 8x8 per plane
constexpr int MB_SIZE = 16;
constexpr int MB_CHROMA_SIZE = 8;

/// Iterate over all macroblocks in a YUV frame; callback receives coord and block views.
void for_each_macroblock(const FrameYUV& frame,
                         std::function<void(BlockCoord, BlockView, BlockView, BlockView)> fn);

/// Iterate with const views (for read-only ME)
void for_each_macroblock_const(const FrameYUV& frame,
                               std::function<void(BlockCoord, BlockViewConst, BlockViewConst, BlockViewConst)> fn);

/// Overloads for refcounted Frame (I420)
void for_each_macroblock_const(const Frame& frame,
                               std::function<void(BlockCoord, BlockViewConst, BlockViewConst, BlockViewConst)> fn);
void get_macroblock_views_const(const Frame& frame, BlockCoord coord,
                                BlockViewConst* out_y, BlockViewConst* out_u, BlockViewConst* out_v);

/// Get a single macroblock view at (mb_x, mb_y). Returns empty views if out of bounds.
void get_macroblock_views(FrameYUV& frame, BlockCoord coord,
                          BlockView* out_y, BlockView* out_u, BlockView* out_v);
void get_macroblock_views_const(const FrameYUV& frame, BlockCoord coord,
                                BlockViewConst* out_y, BlockViewConst* out_u, BlockViewConst* out_v);

}  // namespace codec
}  // namespace telehealth
