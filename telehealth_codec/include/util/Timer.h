#pragma once

#include <chrono>
#include <cstdint>

namespace telehealth {
namespace util {

class Timer {
 public:
  void start();
  void stop();
  int64_t elapsed_us() const;
  double elapsed_ms() const;

 private:
  std::chrono::steady_clock::time_point start_;
  std::chrono::steady_clock::time_point end_;
  bool running_ = false;
};

class ScopedTimer {
 public:
  explicit ScopedTimer(int64_t* out_us) : out_us_(out_us) { timer_.start(); }
  ~ScopedTimer() {
    timer_.stop();
    if (out_us_) *out_us_ = timer_.elapsed_us();
  }

 private:
  Timer timer_;
  int64_t* out_us_;
};

}  // namespace util
}  // namespace telehealth
