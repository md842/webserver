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
  unsigned short port = 0; // Range: 0 - 65535
  // Standard parameters
  std::string index = "";
  std::string root = "";
  std::string host = "";
  // Return statement parameters
  short ret = 0;
  std::string ret_val = "";
  // SSL parameters
  std::string certificate = "";
  std::string private_key = "";
};
