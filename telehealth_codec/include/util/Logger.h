#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <mutex>

namespace telehealth {
namespace util {

enum class LogLevel { Debug, Info, Warn, Error };

class Logger {
 public:
  static Logger& instance();
  void set_level(LogLevel level) { level_ = level; }
  LogLevel level() const { return level_; }

  void log(LogLevel level, const std::string& msg);
  void debug(const std::string& msg) { log(LogLevel::Debug, msg); }
  void info(const std::string& msg) { log(LogLevel::Info, msg); }
  void warn(const std::string& msg) { log(LogLevel::Warn, msg); }
  void error(const std::string& msg) { log(LogLevel::Error, msg); }

 private:
  Logger() = default;
  std::mutex mutex_;
  LogLevel level_ = LogLevel::Info;
};

#define TELECODEC_LOG_DEBUG(x) do { std::ostringstream _s; _s << x; telehealth::util::Logger::instance().debug(_s.str()); } while(0)
#define TELECODEC_LOG_INFO(x)  do { std::ostringstream _s; _s << x; telehealth::util::Logger::instance().info(_s.str()); } while(0)
#define TELECODEC_LOG_WARN(x)  do { std::ostringstream _s; _s << x; telehealth::util::Logger::instance().warn(_s.str()); } while(0)
#define TELECODEC_LOG_ERROR(x) do { std::ostringstream _s; _s << x; telehealth::util::Logger::instance().error(_s.str()); } while(0)

}  // namespace util
}  // namespace telehealth
