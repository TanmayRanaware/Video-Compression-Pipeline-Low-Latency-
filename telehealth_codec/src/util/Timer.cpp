#include <util/Timer.h>

namespace telehealth {
namespace util {

void Timer::start() {
  start_ = std::chrono::steady_clock::now();
  running_ = true;
}

void Timer::stop() {
  end_ = std::chrono::steady_clock::now();
  running_ = false;
}

int64_t Timer::elapsed_us() const {
  auto e = running_ ? std::chrono::steady_clock::now() : end_;
  return std::chrono::duration_cast<std::chrono::microseconds>(e - start_).count();
}

double Timer::elapsed_ms() const {
  return elapsed_us() / 1000.0;
}

}  // namespace util
}  // namespace telehealth
