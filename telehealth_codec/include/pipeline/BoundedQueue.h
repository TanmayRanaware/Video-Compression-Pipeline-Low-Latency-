#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace telehealth {
namespace pipeline {

/// Bounded queue for pipeline stages. Backpressure: drop oldest when full (configurable).
template <typename T>
class BoundedQueue {
 public:
  explicit BoundedQueue(size_t max_size) : max_size_(max_size) {}

  bool push(T item);
  std::optional<T> pop(int timeout_ms = -1);
  bool try_pop(T& out);
  size_t size() const;
  size_t max_size() const { return max_size_; }
  void clear();

 private:
  mutable std::mutex mutex_;
  std::condition_variable cv_not_empty_;
  std::condition_variable cv_not_full_;
  std::queue<T> queue_;
  size_t max_size_;
};

template <typename T>
bool BoundedQueue<T>::push(T item) {
  std::unique_lock lock(mutex_);
  if (queue_.size() >= max_size_) {
    queue_.pop();
    queue_.push(std::move(item));
    return true;
  }
  queue_.push(std::move(item));
  cv_not_empty_.notify_one();
  return true;
}

template <typename T>
std::optional<T> BoundedQueue<T>::pop(int timeout_ms) {
  std::unique_lock lock(mutex_);
  if (timeout_ms < 0) {
    cv_not_empty_.wait(lock, [this] { return !queue_.empty(); });
  } else if (!cv_not_empty_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                                     [this] { return !queue_.empty(); })) {
    return std::nullopt;
  }
  T item = std::move(queue_.front());
  queue_.pop();
  cv_not_full_.notify_one();
  return item;
}

template <typename T>
bool BoundedQueue<T>::try_pop(T& out) {
  std::lock_guard lock(mutex_);
  if (queue_.empty()) return false;
  out = std::move(queue_.front());
  queue_.pop();
  cv_not_full_.notify_one();
  return true;
}

template <typename T>
size_t BoundedQueue<T>::size() const {
  std::lock_guard lock(mutex_);
  return queue_.size();
}

template <typename T>
void BoundedQueue<T>::clear() {
  std::lock_guard lock(mutex_);
  while (!queue_.empty()) queue_.pop();
  cv_not_full_.notify_all();
}

}  // namespace pipeline
}  // namespace telehealth
