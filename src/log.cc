#include <boost/log/trivial.hpp> // BOOST_LOG_TRIVIAL

#include "log.h"

// Convenience wrappers for BOOST_LOG_TRIVIAL macros.

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
  BOOST_LOG_TRIVIAL(trace) << msg;
}

void Log::warn(const std::string& msg){
  BOOST_LOG_TRIVIAL(warning) << msg;
}