#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

namespace telehealth {
namespace pipeline {

/// Base stage: runs a loop that pulls from input queue, processes, pushes to output.
/// Subclass or use lambda-based stage.
class Stage {
 public:
  using ProcessFunc = std::function<bool()>;  // return false to stop

  Stage(std::string name, ProcessFunc run_loop);
  ~Stage();
  void start();
  void stop();
  bool running() const { return running_.load(); }
  const std::string& name() const { return name_; }

 private:
  void thread_loop();

  std::string name_;
  ProcessFunc run_loop_;
  std::unique_ptr<std::thread> thread_;
  std::atomic<bool> running_{false};
};

}  // namespace pipeline
}  // namespace telehealth
