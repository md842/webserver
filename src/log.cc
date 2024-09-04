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
  const std::string& method,
  unsigned response_code
){
  BOOST_LOG_TRIVIAL(info) <<
    "[ResponseMetrics][client:" << client_ip <<
    "][req_bytes:" << req_bytes <<
    "][res_bytes:" << res_bytes <<
    "][method:" << method <<
    "][response_code:" << response_code << "]";
}


/// Convenience wrappers for BOOST_LOG_TRIVIAL macros.
void Log::debug(const std::string& msg){
  BOOST_LOG_TRIVIAL(debug) << msg;
}


void Log::error(const std::string& msg){
  BOOST_LOG_TRIVIAL(error) << msg;
}


void Log::fatal(const std::string& msg){
  BOOST_LOG_TRIVIAL(fatal) << msg;
}


void Log::info(const std::string& msg){
  BOOST_LOG_TRIVIAL(info) << msg;
}


void Log::trace(const std::string& msg){
  if (trace_enabled)
    BOOST_LOG_TRIVIAL(trace) << msg;
}


void Log::warn(const std::string& msg){
  BOOST_LOG_TRIVIAL(warning) << msg;
}