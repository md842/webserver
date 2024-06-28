#pragma once

#include <iostream> // std::string

class Log {
public:
  static void debug(const std::string& msg);
  static void error(const std::string& msg);
  static void fatal(const std::string& msg);
  static void info(const std::string& msg);
  static void trace(const std::string& msg);
  static void warn(const std::string& msg);
};
