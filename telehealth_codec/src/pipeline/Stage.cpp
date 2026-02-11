#include <pipeline/Stage.h>

namespace telehealth {
namespace pipeline {

Stage::Stage(std::string name, ProcessFunc run_loop)
    : name_(std::move(name)), run_loop_(std::move(run_loop)) {}

Stage::~Stage() {
  stop();
}

void Stage::start() {
  if (running_.exchange(true)) return;
  thread_ = std::make_unique<std::thread>(&Stage::thread_loop, this);
}

void Stage::stop() {
  if (!running_.exchange(false)) return;
  if (thread_ && thread_->joinable())
    thread_->join();
  thread_.reset();
}

void Stage::thread_loop() {
  while (running_.load() && run_loop_()) {}
}

}  // namespace pipeline
}  // namespace telehealth
