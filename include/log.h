#pragma once

#include <string>

class Log{
public:
  /// Enables trace logs globally; they are suppressed by default.
  static void enable_trace();

  /// Machine-parseable log for response metrics.
  static void res_metrics(
    const std::string& client_ip,
    size_t req_bytes,
    size_t res_bytes,
    const std::string& method,
    unsigned response_code
  );

  /// Convenience wrappers for BOOST_LOG_TRIVIAL macros.
  static void debug(const std::string& source, const std::string& msg);
  static void error(const std::string& source, const std::string& msg);
  static void fatal(const std::string& source, const std::string& msg);
  static void info(const std::string& source, const std::string& msg);
  static void trace(const std::string& source, const std::string& msg);
  static void warn(const std::string& source, const std::string& msg);
};
