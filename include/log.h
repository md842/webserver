#pragma once

#include <string>

class Log{
public:
  /// Enables trace logs globally; they are suppressed by default.
  static void enable_trace();

  /// Convenience wrappers for BOOST_LOG_TRIVIAL macros.
  static void debug(const std::string& msg);
  static void error(const std::string& msg);
  static void fatal(const std::string& msg);
  static void info(const std::string& msg);
  static void trace(const std::string& msg);
  static void warn(const std::string& msg);
};
