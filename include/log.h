#pragma once

#include <string>

class Log{
public:
  struct req_info {
    size_t bytes;
    std::string summary;
    std::string invalid_req;
  };

  /// Machine-parseable log for response metrics.
  static void res_metrics(
    const std::string& client_ip,
    req_info& req,
    size_t res_bytes,
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
