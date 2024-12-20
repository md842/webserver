#include <boost/log/trivial.hpp> // BOOST_LOG_TRIVIAL

#include "log.h"

static bool trace_enabled = false; // When false, trace logs are suppressed.


/// Enables trace logs globally; they are suppressed by default.
void Log::enable_trace(){
  trace_enabled = true;
}


/// Machine-parseable log for response metrics.
void Log::res_metrics(
  const std::string& client_ip,
  size_t req_bytes,
  size_t res_bytes,
  const std::string& req_summary,
  const std::string& invalid_req,
  unsigned response_code
){
  BOOST_LOG_TRIVIAL(info) << "[Response]          " <<
    " Client: " << client_ip <<
    " | Status: " << response_code <<
    " | Request: " << req_summary <<
    " | Received (B): " << req_bytes <<
    " | Sent (B): " << res_bytes <<
    (invalid_req.length() > 0 ? "\nInvalid request:\n" + invalid_req : "");
}


/// Convenience wrappers for BOOST_LOG_TRIVIAL macros.
void Log::debug(const std::string& source, const std::string& msg){
  BOOST_LOG_TRIVIAL(debug) << source << msg;
}


void Log::error(const std::string& source, const std::string& msg){
  BOOST_LOG_TRIVIAL(error) << source << msg;
}


void Log::fatal(const std::string& source, const std::string& msg){
  BOOST_LOG_TRIVIAL(fatal) << source << msg;
}


void Log::info(const std::string& source, const std::string& msg){
  BOOST_LOG_TRIVIAL(info) << source << msg;
}


void Log::trace(const std::string& source, const std::string& msg){
  if (trace_enabled)
    BOOST_LOG_TRIVIAL(trace) << source << msg;
}


void Log::warn(const std::string& source, const std::string& msg){
  BOOST_LOG_TRIVIAL(warning) << source << msg;
}