#include <codec/MotionCompensation.h>
#include <algorithm>
#include <cstring>

namespace telehealth {
namespace codec {

void MotionCompensation::predict_block(BlockView pred_out,
                                       const FrameYUV& ref_frame,
                                       BlockCoord pos,
                                       MotionVector mv) const {
  const int base_x = pos.mb_x * MB_SIZE;
  const int base_y = pos.mb_y * MB_SIZE;
  int ref_x = base_x + mv.dx;
  int ref_y = base_y + mv.dy;

  ref_x = std::clamp(ref_x, 0, ref_frame.width - pred_out.w);
  ref_y = std::clamp(ref_y, 0, ref_frame.height - pred_out.h);

  const uint8_t* src = ref_frame.y_plane.data() + ref_y * ref_frame.stride_y + ref_x;
  for (int y = 0; y < pred_out.h; ++y) {
    std::memcpy(pred_out.row(y), src + y * ref_frame.stride_y, static_cast<size_t>(pred_out.w));
  }
}

void MotionCompensation::predict_block(BlockView pred_out,
                                       const Frame& ref_frame,
                                       BlockCoord pos,
                                       MotionVector mv) const {
  const int base_x = pos.mb_x * MB_SIZE;
  const int base_y = pos.mb_y * MB_SIZE;
  int ref_x = base_x + mv.dx;
  int ref_y = base_y + mv.dy;

  ref_x = std::clamp(ref_x, 0, ref_frame.width() - pred_out.w);
  ref_y = std::clamp(ref_y, 0, ref_frame.height() - pred_out.h);

  const uint8_t* src = ref_frame.y_plane_ptr() + ref_y * ref_frame.stride_y() + ref_x;
  for (int y = 0; y < pred_out.h; ++y) {
    std::memcpy(pred_out.row(y), src + y * ref_frame.stride_y(), static_cast<size_t>(pred_out.w));
  }
}

void MotionCompensation::predict_frame(FrameYUV& pred_frame,
                                       const FrameYUV& ref_frame,
                                       const MotionVector* mvs,
                                       int mb_cols,
                                       int mb_rows) const {
  pred_frame.allocate(ref_frame.width, ref_frame.height);

  for (int mb_y = 0; mb_y < mb_rows; ++mb_y) {
    for (int mb_x = 0; mb_x < mb_cols; ++mb_x) {
      BlockCoord coord{mb_x, mb_y};
      BlockView vy, vu, vv;
      get_macroblock_views(pred_frame, coord, &vy, &vu, &vv);

      const MotionVector& mv = mvs[mb_y * mb_cols + mb_x];

      int px = mb_x * MB_SIZE;
      int py = mb_y * MB_SIZE;
      int w = std::min(MB_SIZE, ref_frame.width - px);
      int h = std::min(MB_SIZE, ref_frame.height - py);
      int rx = std::clamp(px + mv.dx, 0, ref_frame.width - w);
      int ry = std::clamp(py + mv.dy, 0, ref_frame.height - h);

      for (int y = 0; y < h; ++y) {
        const uint8_t* src = ref_frame.y_plane.data() + (ry + y) * ref_frame.stride_y + rx;
        std::memcpy(vy.row(y), src, static_cast<size_t>(w));
      }

      int cpx = mb_x * MB_CHROMA_SIZE;
      int cpy = mb_y * MB_CHROMA_SIZE;
      int cw = std::min(MB_CHROMA_SIZE, ref_frame.width / 2 - cpx);
      int ch = std::min(MB_CHROMA_SIZE, ref_frame.height / 2 - cpy);
      int crx = std::clamp(cpx + mv.dx / 2, 0, ref_frame.width / 2 - cw);
      int cry = std::clamp(cpy + mv.dy / 2, 0, ref_frame.height / 2 - ch);

      for (int y = 0; y < ch; ++y) {
        const uint8_t* su = ref_frame.u_plane.data() + (cry + y) * ref_frame.stride_uv + crx;
        const uint8_t* sv = ref_frame.v_plane.data() + (cry + y) * ref_frame.stride_uv + crx;
        std::memcpy(vu.row(y), su, static_cast<size_t>(cw));
        std::memcpy(vv.row(y), sv, static_cast<size_t>(cw));
      }
    }
  }
}

void MotionCompensation::predict_frame(FrameYUV& pred_frame,
                                       const Frame& ref_frame,
                                       const MotionVector* mvs,
                                       int mb_cols,
                                       int mb_rows) const {
  pred_frame.allocate(ref_frame.width(), ref_frame.height());

  for (int mb_y = 0; mb_y < mb_rows; ++mb_y) {
    for (int mb_x = 0; mb_x < mb_cols; ++mb_x) {
      BlockCoord coord{mb_x, mb_y};
      BlockView vy, vu, vv;
      get_macroblock_views(pred_frame, coord, &vy, &vu, &vv);

      const MotionVector& mv = mvs[mb_y * mb_cols + mb_x];

      int px = mb_x * MB_SIZE;
      int py = mb_y * MB_SIZE;
      int w = std::min(MB_SIZE, ref_frame.width() - px);
      int h = std::min(MB_SIZE, ref_frame.height() - py);
      int rx = std::clamp(px + mv.dx, 0, ref_frame.width() - w);
      int ry = std::clamp(py + mv.dy, 0, ref_frame.height() - h);

      for (int y = 0; y < h; ++y) {
        const uint8_t* src = ref_frame.y_plane_ptr() + (ry + y) * ref_frame.stride_y() + rx;
        std::memcpy(vy.row(y), src, static_cast<size_t>(w));
      }

      int cpx = mb_x * MB_CHROMA_SIZE;
      int cpy = mb_y * MB_CHROMA_SIZE;
      int cw = std::min(MB_CHROMA_SIZE, ref_frame.width() / 2 - cpx);
      int ch = std::min(MB_CHROMA_SIZE, ref_frame.height() / 2 - cpy);
      int crx = std::clamp(cpx + mv.dx / 2, 0, ref_frame.width() / 2 - cw);
      int cry = std::clamp(cpy + mv.dy / 2, 0, ref_frame.height() / 2 - ch);

      for (int y = 0; y < ch; ++y) {
        const uint8_t* su = ref_frame.u_plane_ptr() + (cry + y) * ref_frame.stride_uv() + crx;
        const uint8_t* sv = ref_frame.v_plane_ptr() + (cry + y) * ref_frame.stride_uv() + crx;
        std::memcpy(vu.row(y), su, static_cast<size_t>(cw));
        std::memcpy(vv.row(y), sv, static_cast<size_t>(cw));
      }
    }
  }
}

}  // namespace codec
}  // namespace telehealth
