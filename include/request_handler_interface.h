#pragma once

#include "nginx_config_server_block.h" // Config
#include "typedefs/http.h"

class RequestHandler{ // Pure virtual class (interface)
public:
  /** 
   * A pure virtual handle_request function for interface use. Must override.
   *
   * @param req A parsed HTTP request.
   * @returns A pointer to a parsed HTTP response.
   */
  virtual Response* handle_request(const Request& req) = 0;

  /** 
   * Initializes the config object for this handler. Override not required.
   *
   * @pre ConfigParser::parse() succeeded.
   * @param config A parsed Config object.
   * @returns A pointer to a parsed HTTP response.
   */
  void init_config(Config* config){config_ = config;}

protected:
  Config* config_;
};

class RequestHandlerFactory{ // Pure virtual class (interface)
public:
  /// A pure virtual create() function for interface use. Must override.
  virtual RequestHandler* create() = 0;
};