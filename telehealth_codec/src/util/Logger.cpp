#include <util/Logger.h>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace telehealth {
namespace util {

Logger& Logger::instance() {
  static Logger inst;
  return inst;
}

void Logger::log(LogLevel level, const std::string& msg) {
  if (level < level_) return;
  std::lock_guard lock(mutex_);
  auto now = std::chrono::system_clock::now();
  auto t = std::chrono::system_clock::to_time_t(now);
  std::ostringstream prefix;
  prefix << "[" << std::put_time(std::localtime(&t), "%H:%M:%S") << "] ";
  switch (level) {
    case LogLevel::Debug: prefix << "D "; break;
    case LogLevel::Info:  prefix << "I "; break;
    case LogLevel::Warn:  prefix << "W "; break;
    case LogLevel::Error: prefix << "E "; break;
  }
  std::cerr << prefix.str() << msg << std::endl;
}

}  // namespace util
}  // namespace telehealth
