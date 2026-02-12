#pragma once

#include "BoundedQueue.h"
#include "Stage.h"
#include "codec/Frame.h"
#include "codec/Bitstream.h"
#include <memory>
#include <vector>

namespace telehealth {
namespace pipeline {

/// Item between capture and convert: refcounted RGB frame (shared across stages)
struct CaptureItem {
  std::shared_ptr<codec::Frame> frame;
};

/// Item between convert and encode: refcounted I420 frame (shared across stages)
struct ConvertedItem {
  std::shared_ptr<codec::Frame> frame;
};

/// Item between encode and send: encoded frame
struct EncodedItem {
  codec::EncodedFrame frame;
  codec::FrameMeta meta;
};

/// Pipeline: Capture -> Convert -> Encode -> Packetize/Send
/// Uses bounded queues; drop oldest on overflow.
class Pipeline {
 public:
  struct Config {
    size_t queue_capture = 2;
    size_t queue_convert = 2;
    size_t queue_encode = 2;
    int width = 640;
    int height = 480;
    int fps = 30;
    int qp_default = 28;
    int gop_size = 30;
  };

  explicit Pipeline(Config config);
  ~Pipeline();

  void start();
  void stop();

  BoundedQueue<CaptureItem>* capture_queue() { return capture_queue_.get(); }
  BoundedQueue<ConvertedItem>* convert_queue() { return convert_queue_.get(); }
  BoundedQueue<EncodedItem>* encode_queue() { return encode_queue_.get(); }

  void push_capture(CaptureItem item);
  bool pop_encoded(EncodedItem& out, int timeout_ms = -1);

 private:
  Config config_;
  std::unique_ptr<BoundedQueue<CaptureItem>> capture_queue_;
  std::unique_ptr<BoundedQueue<ConvertedItem>> convert_queue_;
  std::unique_ptr<BoundedQueue<EncodedItem>> encode_queue_;
  std::vector<std::unique_ptr<Stage>> stages_;
};

}  // namespace pipeline
}  // namespace telehealth
