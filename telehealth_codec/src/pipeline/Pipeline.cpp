#include <pipeline/Pipeline.h>
#include <codec/YuvConverter.h>
#include <codec/Encoder.h>
#include <codec/EncoderConfig.h>

namespace telehealth {
namespace pipeline {

Pipeline::Pipeline(Config config)
    : config_(config),
      capture_queue_(std::make_unique<BoundedQueue<CaptureItem>>(config.queue_capture)),
      convert_queue_(std::make_unique<BoundedQueue<ConvertedItem>>(config.queue_convert)),
      encode_queue_(std::make_unique<BoundedQueue<EncodedItem>>(config.queue_encode)) {

  codec::EncoderConfig enc_cfg;
  enc_cfg.width = config.width;
  enc_cfg.height = config.height;
  enc_cfg.fps = config.fps;
  enc_cfg.qp_default = config.qp_default;
  enc_cfg.gop_size = config.gop_size;

  auto* cap_q = capture_queue_.get();
  auto* conv_q = convert_queue_.get();
  auto* enc_q = encode_queue_.get();
  int w = config.width, h = config.height, qp = config.qp_default, gop = config.gop_size;

  stages_.push_back(std::make_unique<Stage>("convert", [cap_q, conv_q, w, h]() {
    auto item = cap_q->pop(100);
    if (!item) return true;
    codec::YuvConverter conv;
    ConvertedItem out;
    conv.rgb_to_yuv420(item->frame, out.frame);
    out.meta = item->meta;
    conv_q->push(std::move(out));
    return true;
  }));

  stages_.push_back(std::make_unique<Stage>("encode", [conv_q, enc_q, w, h, qp, gop]() {
    auto item = conv_q->pop(100);
    if (!item) return true;
    codec::EncoderConfig cfg;
    cfg.width = w; cfg.height = h; cfg.qp_default = qp; cfg.gop_size = gop;
    codec::Encoder enc(cfg);
    codec::EncodedFrame ef = enc.encode(item->frame, item->meta);
    EncodedItem out;
    out.frame = std::move(ef);
    out.meta = item->meta;
    enc_q->push(std::move(out));
    return true;
  }));
}

Pipeline::~Pipeline() {
  stop();
}

void Pipeline::start() {
  for (auto& s : stages_)
    s->start();
}

void Pipeline::stop() {
  for (auto& s : stages_)
    s->stop();
}

void Pipeline::push_capture(CaptureItem item) {
  capture_queue_->push(std::move(item));
}

bool Pipeline::pop_encoded(EncodedItem& out, int timeout_ms) {
  auto opt = encode_queue_->pop(timeout_ms);
  if (!opt) return false;
  out = std::move(*opt);
  return true;
}

}  // namespace pipeline
}  // namespace telehealth
