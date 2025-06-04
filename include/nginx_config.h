#pragma once

#include <string>

class Config{
 public:
  /// Validates the individual server block stored by the Config object.
  bool validate();

  enum ServerType{
    REDIRECT = 0,
    HTTP_SERVER = 1,
    HTTPS_SERVER = 2
  };

  ServerType type = HTTP_SERVER; // Default value
  short port; // Defined by all server blocks
  // Standard parameters
  std::string index;
  std::string root;
  // Redirect parameters
  short ret;
  std::string ret_uri;
};
