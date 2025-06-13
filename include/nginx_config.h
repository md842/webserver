#pragma once

#include <string>

class Config{
 public:
  /// Validates the individual server block stored by the Config object.
  bool validate();

  enum ServerType{
    HTTP_SERVER = 0,
    HTTPS_SERVER = 1
  };

  // Defined by all server blocks
  ServerType type = HTTP_SERVER;
  short port = 0;
  // Standard parameters
  std::string index = "";
  std::string root = "";
  std::string host = "";
  // Redirect parameters
  short ret = 0;
  std::string ret_uri = "";
  // SSL parameters
  std::string certificate = "";
  std::string private_key = "";
};
