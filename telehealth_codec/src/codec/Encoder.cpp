#include <codec/Encoder.h>
#include <codec/MotionEstimation.h>
#include <codec/MotionCompensation.h>
#include <codec/Transform.h>
#include <codec/Quantizer.h>
#include <codec/EntropyCoder.h>
#include <codec/RateControl.h>
#include <codec/Block.h>
#include <codec/Residual.h>
#include <cstring>
#include <algorithm>

namespace telehealth {
namespace codec {

Encoder::Encoder(const EncoderConfig& config)
    : config_(config) {
  me_ = std::make_unique<MotionEstimation>(config);
  mc_ = std::make_unique<MotionCompensation>();
  transform_ = std::make_unique<Transform>();
  quantizer_ = std::make_unique<Quantizer>();
  entropy_ = std::make_unique<EntropyCoder>();
  rate_control_ = std::make_unique<RateControl>(config);

  int mb_cols = (config.width + MB_SIZE - 1) / MB_SIZE;
  int mb_rows = (config.height + MB_SIZE - 1) / MB_SIZE;
  mv_buffer_.resize(static_cast<size_t>(mb_cols * mb_rows));
  coeff_buffer_.resize(static_cast<size_t>(mb_cols * mb_rows * (4 * 64 + 2 * 64)));
  residual_buffer_.resize(static_cast<size_t>(mb_cols * mb_rows * 16 * 16));
}

Encoder::~Encoder() = default;

EncodedFrame Encoder::encode(const FrameYUV& frame, const FrameMeta& meta) {
  FrameStats stats;
  stats.frame_id = static_cast<uint32_t>(meta.frame_id);

  FrameType ftype = rate_control_->choose_frame_type(stats.frame_id, nullptr);
  EncodedFrame out;
  out.frame_id = stats.frame_id;
  out.timestamp_us = static_cast<uint64_t>(meta.timestamp_us);

  if (ftype == FrameType::I) {
    out = encode_i_frame(frame, meta);
  } else {
    out = encode_p_frame(frame, meta);
  }

  stats.bits_used = out.total_bytes() * 8;
  out.qp = static_cast<uint8_t>(rate_control_->choose_qp(stats));

  if (!reference_) {
    reference_ = std::make_unique<FrameYUV>();
    reference_->allocate(frame.width, frame.height);
  }
  *reference_ = frame;

  out.raw_bytes.clear();
  out.raw_bytes.insert(out.raw_bytes.end(), out.mv_bytes.begin(), out.mv_bytes.end());
  out.raw_bytes.insert(out.raw_bytes.end(), out.coeff_bytes.begin(), out.coeff_bytes.end());
  return out;
}

EncodedFrame Encoder::encode_i_frame(const FrameYUV& frame, const FrameMeta& meta) {
  EncodedFrame out;
  out.type = FrameType::I;
  out.frame_id = static_cast<uint32_t>(meta.frame_id);
  out.timestamp_us = static_cast<uint64_t>(meta.timestamp_us);
  out.qp = static_cast<uint8_t>(config_.qp_default);

  BitstreamWriter bs;
  int mb_cols = (frame.width + MB_SIZE - 1) / MB_SIZE;
  int mb_rows = (frame.height + MB_SIZE - 1) / MB_SIZE;
  size_t coeff_offset = 0;

  for_each_macroblock_const(frame, [&](BlockCoord coord, BlockViewConst yv, BlockViewConst uv, BlockViewConst vv) {
    (void)uv; (void)vv;
    int32_t coeff[64];
    for (int by = 0; by < 2; ++by) {
      for (int bx = 0; bx < 2; ++bx) {
        int16_t res[64];
        for (int i = 0; i < 64; ++i) {
          int yy = by * 8 + i / 8, xx = bx * 8 + i % 8;
          res[i] = static_cast<int16_t>(yv.ptr[yy * yv.stride + xx]);
        }
        transform_->forward_8x8(res, 8, coeff);
        quantizer_->quantize_8x8(coeff, out.qp);
        entropy_->encode_block_8x8(coeff, out.qp, bs);
      }
    }
    int32_t cu[64], cv[64];
    std::memset(cu, 0, sizeof(cu));
    std::memset(cv, 0, sizeof(cv));
    for (int i = 0; i < 64; ++i) {
      if (i < uv.w * uv.h) cu[i] = uv.ptr[i / uv.w * uv.stride + i % uv.w];
      if (i < vv.w * vv.h) cv[i] = vv.ptr[i / vv.w * vv.stride + i % vv.w];
    }
    quantizer_->quantize_8x8(cu, out.qp);
    quantizer_->quantize_8x8(cv, out.qp);
    entropy_->encode_block_8x8(cu, out.qp, bs);
    entropy_->encode_block_8x8(cv, out.qp, bs);
  });

  bs.flush_byte_align();
  out.coeff_bytes = bs.buffer();
  return out;
}

EncodedFrame Encoder::encode_p_frame(const FrameYUV& frame, const FrameMeta& meta) {
  EncodedFrame out;
  out.type = FrameType::P;
  out.frame_id = static_cast<uint32_t>(meta.frame_id);
  out.timestamp_us = static_cast<uint64_t>(meta.timestamp_us);
  out.qp = static_cast<uint8_t>(config_.qp_default);

  if (!reference_ || reference_->empty()) {
    return encode_i_frame(frame, meta);
  }

  const FrameYUV& ref = *reference_;
  int mb_cols = (frame.width + MB_SIZE - 1) / MB_SIZE;
  int mb_rows = (frame.height + MB_SIZE - 1) / MB_SIZE;
  mv_buffer_.resize(static_cast<size_t>(mb_cols * mb_rows));

  BitstreamWriter mv_writer, coeff_writer;
  int mb_idx = 0;

  for_each_macroblock_const(frame, [&](BlockCoord coord, BlockViewConst yv, BlockViewConst uv, BlockViewConst vv) {
    MotionResult res = config_.use_diamond_search
        ? me_->estimate_diamond(yv, ref, coord)
        : me_->estimate(yv, ref, coord);
    mv_buffer_[mb_idx++] = res.mv;
    entropy_->encode_mv(res.mv, mv_writer);

    FrameYUV pred_one;
    pred_one.allocate(MB_SIZE, MB_SIZE);
    BlockView pv(pred_one.y_plane.data(), pred_one.stride_y, yv.w, yv.h);
    mc_->predict_block(pv, ref, coord, res.mv);
    BlockViewConst pvc(pred_one.y_plane.data(), pred_one.stride_y, yv.w, yv.h);

    int16_t residual[256];
    compute_residual(yv, pvc, residual);

    int32_t coeff[4 * 64];
    for (int by = 0; by < 2; ++by) {
      for (int bx = 0; bx < 2; ++bx) {
        int i = by * 2 + bx;
        transform_->forward_8x8(residual + by * 8 * 16 + bx * 8, 16, coeff + i * 64);
        quantizer_->quantize_8x8(coeff + i * 64, out.qp);
        entropy_->encode_block_8x8(coeff + i * 64, out.qp, coeff_writer);
      }
    }
    int32_t cu[64], cv[64];
    std::memset(cu, 0, sizeof(cu));
    std::memset(cv, 0, sizeof(cv));
    entropy_->encode_block_8x8(cu, out.qp, coeff_writer);
    entropy_->encode_block_8x8(cv, out.qp, coeff_writer);
  });

  mv_writer.flush_byte_align();
  coeff_writer.flush_byte_align();
  out.mv_bytes = mv_writer.buffer();
  out.coeff_bytes = coeff_writer.buffer();
  return out;
}

}  // namespace codec
}  // namespace telehealth
