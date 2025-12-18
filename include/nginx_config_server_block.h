#pragma once

#include <string>
#include <vector>

#include "nginx_config_location_block.h" // LocationBlock

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

  // Standard parameters
  unsigned short port = 80; // Default value, may be overriden. 0 - 65535
  std::string index = "index.html"; // Default value, may be overriden.
  std::string root = "html"; // Default value, may be overriden.
  std::string host = "";
  // Return statement parameters
  short ret = 0;
  std::string ret_val = "";
  // SSL parameters
  std::string certificate = "";
  std::string private_key = "";

  // location directives defined within this server block
  // 0: Exact match (=)
  // 1: Prefix match with stop modifier (^~)
  // 2: Regex match (case-sensitive or case-insensitive) (~, ~*)
  // 3: No modifier
  std::vector<LocationBlock*> locations[4];
};