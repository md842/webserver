#pragma once

#include <boost/beast.hpp> // http::request, http::response

#include "nginx_config.h" // Config

using Request = boost::beast::http::request<boost::beast::http::string_body>;
using Response = boost::beast::http::response<boost::beast::http::string_body>;

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