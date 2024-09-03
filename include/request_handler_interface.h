#pragma once

#include <boost/beast.hpp> // http::request, http::response

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
};

class RequestHandlerFactory{ // Pure virtual class (interface)
public:
  /// A pure virtual create() function for interface use. Must override.
  virtual RequestHandler* create() = 0;
};