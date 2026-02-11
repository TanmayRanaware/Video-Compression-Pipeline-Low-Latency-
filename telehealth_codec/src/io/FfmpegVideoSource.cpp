#ifdef TELECODEC_USE_FFMPEG

#include <io/VideoSource.h>
#include <codec/Frame.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#include <stdexcept>
#include <cstring>

namespace telehealth {
namespace io {

class FfmpegVideoSourceImpl : public VideoSource {
 public:
  FfmpegVideoSourceImpl() = default;
  ~FfmpegVideoSourceImpl() override { close(); }

  bool open(const VideoSourceConfig& config) {
    config_ = config;
    if (config.path.empty()) return false;
    if (avformat_open_input(&fmt_ctx_, config.path.c_str(), nullptr, nullptr) != 0)
      return false;
    if (avformat_find_stream_info(fmt_ctx_, nullptr) < 0) {
      avformat_close_input(&fmt_ctx_);
      return false;
    }
    stream_idx_ = av_find_best_stream(fmt_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, &dec_, 0);
    if (stream_idx_ < 0) {
      avformat_close_input(&fmt_ctx_);
      return false;
    }
    AVCodecParameters* par = fmt_ctx_->streams[stream_idx_]->codecpar;
    dec_ctx_ = avcodec_alloc_context3(dec_);
    if (!dec_ctx_) { close(); return false; }
    if (avcodec_parameters_to_context(dec_ctx_, par) < 0) { close(); return false; }
    if (avcodec_open2(dec_ctx_, dec_, nullptr) < 0) { close(); return false; }
    width_ = dec_ctx_->width;
    height_ = dec_ctx_->height;
    fps_ = (fmt_ctx_->streams[stream_idx_]->avg_frame_rate.den > 0)
           ? fmt_ctx_->streams[stream_idx_]->avg_frame_rate.num / fmt_ctx_->streams[stream_idx_]->avg_frame_rate.den
           : 30;
    frame_ = av_frame_alloc();
    packet_ = av_packet_alloc();
    return true;
  }

  void close() {
    if (frame_) { av_frame_free(&frame_); frame_ = nullptr; }
    if (packet_) { av_packet_free(&packet_); packet_ = nullptr; }
    if (dec_ctx_) { avcodec_free_context(&dec_ctx_); dec_ctx_ = nullptr; }
    if (fmt_ctx_) { avformat_close_input(&fmt_ctx_); fmt_ctx_ = nullptr; }
    dec_ = nullptr;
    stream_idx_ = -1;
  }

  bool read(codec::FrameRGB& out, codec::FrameMeta& meta) override {
    if (!dec_ctx_ || !frame_ || !packet_) return false;
    for (;;) {
      int ret = av_read_frame(fmt_ctx_, packet_);
      if (ret < 0) return false;
      if (packet_->stream_index != stream_idx_) {
        av_packet_unref(packet_);
        continue;
      }
      ret = avcodec_send_packet(dec_ctx_, packet_);
      av_packet_unref(packet_);
      if (ret < 0) continue;
      ret = avcodec_receive_frame(dec_ctx_, frame_);
      if (ret == AVERROR(EAGAIN)) continue;
      if (ret < 0) return false;

      out.allocate(frame_->width, frame_->height);
      meta.frame_id = frame_id_++;
      meta.timestamp_us = frame_->pts != AV_NOPTS_VALUE ? frame_->pts * 1000000 / 90000 : 0;
      meta.pts_sec = meta.timestamp_us / 1e6;

      SwsContext* sws = sws_getContext(frame_->width, frame_->height, static_cast<AVPixelFormat>(frame_->format),
                                       frame_->width, frame_->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);
      if (!sws) return false;
      uint8_t* dst[1] = { out.row(0) };
      int dst_stride[1] = { out.stride };
      sws_scale(sws, frame_->data, frame_->linesize, 0, frame_->height, dst, dst_stride);
      sws_freeContext(sws);
      return true;
    }
  }

  bool opened() const override { return dec_ctx_ != nullptr; }
  int width() const override { return width_; }
  int height() const override { return height_; }
  int fps() const override { return fps_; }

 private:
  VideoSourceConfig config_;
  AVFormatContext* fmt_ctx_ = nullptr;
  AVCodecContext* dec_ctx_ = nullptr;
  const AVCodec* dec_ = nullptr;
  int stream_idx_ = -1;
  AVFrame* frame_ = nullptr;
  AVPacket* packet_ = nullptr;
  int width_ = 0, height_ = 0, fps_ = 0;
  int64_t frame_id_ = 0;
};

std::unique_ptr<VideoSource> create_video_source(const VideoSourceConfig& config) {
  auto src = std::make_unique<FfmpegVideoSourceImpl>();
  if (!config.path.empty() && src->open(config))
    return src;
  return nullptr;
}

}  // namespace io
}  // namespace telehealth

#else

#include <io/VideoSource.h>
#include <codec/Frame.h>
#include <cstdlib>
#include <cstring>

namespace telehealth {
namespace io {

class SyntheticVideoSource : public VideoSource {
 public:
  bool read(codec::FrameRGB& out, codec::FrameMeta& meta) override {
    out.allocate(width_, height_);
    for (int y = 0; y < height_; ++y) {
      uint8_t* row = out.row(y);
      for (int x = 0; x < width_; ++x) {
        row[x * 3]     = static_cast<uint8_t>((frame_id_ + x + y) % 256);
        row[x * 3 + 1] = static_cast<uint8_t>((frame_id_ * 2 + x) % 256);
        row[x * 3 + 2] = static_cast<uint8_t>((frame_id_ + y) % 256);
      }
    }
    meta.frame_id = frame_id_++;
    meta.timestamp_us = meta.frame_id * 1000000 / (fps_ > 0 ? fps_ : 30);
    meta.pts_sec = meta.timestamp_us / 1e6;
    return true;
  }
  bool opened() const override { return true; }
  int width() const override { return width_; }
  int height() const override { return height_; }
  int fps() const override { return fps_; }
  bool open(const VideoSourceConfig& config) {
    width_ = config.width; height_ = config.height; fps_ = config.fps;
    return width_ > 0 && height_ > 0;
  }
 private:
  int width_ = 640, height_ = 480, fps_ = 30;
  int64_t frame_id_ = 0;
};

std::unique_ptr<VideoSource> create_video_source(const VideoSourceConfig& config) {
  auto src = std::make_unique<SyntheticVideoSource>();
  if (src->open(config)) return src;
  return nullptr;
}

}  // namespace io
}  // namespace telehealth

#endif
