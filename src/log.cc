#include <boost/log/trivial.hpp> // BOOST_LOG_TRIVIAL

#include "log.h"


/// Machine-parseable log for response metrics.
void Log::res_metrics(
  const std::string& client_ip,
  req_info& req,
  size_t res_bytes,
  unsigned response_code
){
  BOOST_LOG_TRIVIAL(info) << "[Response] " <<
    "Client: " << client_ip <<
    " | Status: " << response_code <<
    " | Request: " << req.summary <<
    " | Received: " << req.bytes << " B"
    " | Sent: " << res_bytes << " B";
  if (req.invalid_req.length() > 0)
    BOOST_LOG_TRIVIAL(warning) << "[Request]  " << req.invalid_req;
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
  BOOST_LOG_TRIVIAL(trace) << source << msg;
}


void Log::warn(const std::string& source, const std::string& msg){
  BOOST_LOG_TRIVIAL(warning) << source << msg;
}