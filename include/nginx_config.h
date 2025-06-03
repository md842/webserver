#pragma once

#include <string>

class Config{
 public:
  /// Validates the individual server block stored by the Config object.
  bool validate();

  short port; // Defined by all server blocks
  // Standard parameters
  std::string index;
  std::string root;
  // Redirect parameters
  short ret;
  std::string ret_uri;
};
