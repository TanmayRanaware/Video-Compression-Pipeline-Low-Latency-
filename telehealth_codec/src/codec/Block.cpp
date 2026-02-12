#include <codec/Block.h>
#include <algorithm>

namespace telehealth {
namespace codec {

void for_each_macroblock(const FrameYUV& frame,
                         std::function<void(BlockCoord, BlockView, BlockView, BlockView)> fn) {
  const int mb_cols = (frame.width + MB_SIZE - 1) / MB_SIZE;
  const int mb_rows = (frame.height + MB_SIZE - 1) / MB_SIZE;

  for (int mb_y = 0; mb_y < mb_rows; ++mb_y) {
    for (int mb_x = 0; mb_x < mb_cols; ++mb_x) {
      BlockCoord coord{mb_x, mb_y};
      BlockView vy, vu, vv;
      get_macroblock_views(const_cast<FrameYUV&>(frame), coord, &vy, &vu, &vv);
      fn(coord, vy, vu, vv);
    }
  }
}

void get_macroblock_views(FrameYUV& frame, BlockCoord coord,
                          BlockView* out_y, BlockView* out_u, BlockView* out_v) {
  const int mb_cols = (frame.width + MB_SIZE - 1) / MB_SIZE;
  const int mb_rows = (frame.height + MB_SIZE - 1) / MB_SIZE;
  if (coord.mb_x < 0 || coord.mb_x >= mb_cols || coord.mb_y < 0 || coord.mb_y >= mb_rows) {
    if (out_y) *out_y = BlockView();
    if (out_u) *out_u = BlockView();
    if (out_v) *out_v = BlockView();
    return;
  }

  int px = coord.mb_x * MB_SIZE;
  int py = coord.mb_y * MB_SIZE;
  int w = std::min(MB_SIZE, frame.width - px);
  int h = std::min(MB_SIZE, frame.height - py);

  int cpx = coord.mb_x * MB_CHROMA_SIZE;
  int cpy = coord.mb_y * MB_CHROMA_SIZE;
  int cw = std::min(MB_CHROMA_SIZE, frame.width / 2 - cpx);
  int ch = std::min(MB_CHROMA_SIZE, frame.height / 2 - cpy);

  if (out_y) *out_y = BlockView(frame.y_plane.data() + py * frame.stride_y + px, frame.stride_y, w, h);
  if (out_u) *out_u = BlockView(frame.u_plane.data() + cpy * frame.stride_uv + cpx, frame.stride_uv, cw, ch);
  if (out_v) *out_v = BlockView(frame.v_plane.data() + cpy * frame.stride_uv + cpx, frame.stride_uv, cw, ch);
}

void for_each_macroblock_const(const FrameYUV& frame,
                               std::function<void(BlockCoord, BlockViewConst, BlockViewConst, BlockViewConst)> fn) {
  const int mb_cols = (frame.width + MB_SIZE - 1) / MB_SIZE;
  const int mb_rows = (frame.height + MB_SIZE - 1) / MB_SIZE;

  for (int mb_y = 0; mb_y < mb_rows; ++mb_y) {
    for (int mb_x = 0; mb_x < mb_cols; ++mb_x) {
      BlockCoord coord{mb_x, mb_y};
      BlockViewConst cy, cu, cv;
      get_macroblock_views_const(frame, coord, &cy, &cu, &cv);
      fn(coord, cy, cu, cv);
    }
  }
}

void get_macroblock_views_const(const FrameYUV& frame, BlockCoord coord,
                                BlockViewConst* out_y, BlockViewConst* out_u, BlockViewConst* out_v) {
  const int mb_cols = (frame.width + MB_SIZE - 1) / MB_SIZE;
  const int mb_rows = (frame.height + MB_SIZE - 1) / MB_SIZE;
  if (coord.mb_x < 0 || coord.mb_x >= mb_cols || coord.mb_y < 0 || coord.mb_y >= mb_rows) {
    if (out_y) *out_y = BlockViewConst();
    if (out_u) *out_u = BlockViewConst();
    if (out_v) *out_v = BlockViewConst();
    return;
  }

  int px = coord.mb_x * MB_SIZE;
  int py = coord.mb_y * MB_SIZE;
  int w = std::min(MB_SIZE, frame.width - px);
  int h = std::min(MB_SIZE, frame.height - py);

  int cpx = coord.mb_x * MB_CHROMA_SIZE;
  int cpy = coord.mb_y * MB_CHROMA_SIZE;
  int cw = std::min(MB_CHROMA_SIZE, frame.width / 2 - cpx);
  int ch = std::min(MB_CHROMA_SIZE, frame.height / 2 - cpy);

  if (out_y) *out_y = BlockViewConst(frame.y_plane.data() + py * frame.stride_y + px, frame.stride_y, w, h);
  if (out_u) *out_u = BlockViewConst(frame.u_plane.data() + cpy * frame.stride_uv + cpx, frame.stride_uv, cw, ch);
  if (out_v) *out_v = BlockViewConst(frame.v_plane.data() + cpy * frame.stride_uv + cpx, frame.stride_uv, cw, ch);
}

void for_each_macroblock_const(const Frame& frame,
                               std::function<void(BlockCoord, BlockViewConst, BlockViewConst, BlockViewConst)> fn) {
  const int mb_cols = (frame.width() + MB_SIZE - 1) / MB_SIZE;
  const int mb_rows = (frame.height() + MB_SIZE - 1) / MB_SIZE;

  for (int mb_y = 0; mb_y < mb_rows; ++mb_y) {
    for (int mb_x = 0; mb_x < mb_cols; ++mb_x) {
      BlockCoord coord{mb_x, mb_y};
      BlockViewConst cy, cu, cv;
      get_macroblock_views_const(frame, coord, &cy, &cu, &cv);
      fn(coord, cy, cu, cv);
    }
  }
}

void get_macroblock_views_const(const Frame& frame, BlockCoord coord,
                                BlockViewConst* out_y, BlockViewConst* out_u, BlockViewConst* out_v) {
  const int mb_cols = (frame.width() + MB_SIZE - 1) / MB_SIZE;
  const int mb_rows = (frame.height() + MB_SIZE - 1) / MB_SIZE;
  if (coord.mb_x < 0 || coord.mb_x >= mb_cols || coord.mb_y < 0 || coord.mb_y >= mb_rows) {
    if (out_y) *out_y = BlockViewConst();
    if (out_u) *out_u = BlockViewConst();
    if (out_v) *out_v = BlockViewConst();
    return;
  }

  int px = coord.mb_x * MB_SIZE;
  int py = coord.mb_y * MB_SIZE;
  int w = std::min(MB_SIZE, frame.width() - px);
  int h = std::min(MB_SIZE, frame.height() - py);

  int cpx = coord.mb_x * MB_CHROMA_SIZE;
  int cpy = coord.mb_y * MB_CHROMA_SIZE;
  int cw = std::min(MB_CHROMA_SIZE, frame.width() / 2 - cpx);
  int ch = std::min(MB_CHROMA_SIZE, frame.height() / 2 - cpy);

  if (out_y) *out_y = BlockViewConst(frame.y_plane_ptr() + py * frame.stride_y() + px, frame.stride_y(), w, h);
  if (out_u) *out_u = BlockViewConst(frame.u_plane_ptr() + cpy * frame.stride_uv() + cpx, frame.stride_uv(), cw, ch);
  if (out_v) *out_v = BlockViewConst(frame.v_plane_ptr() + cpy * frame.stride_uv() + cpx, frame.stride_uv(), cw, ch);
}

}  // namespace codec
}  // namespace telehealth
