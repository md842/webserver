#pragma once

#include <boost/beast.hpp>

using Request = boost::beast::http::request<boost::beast::http::string_body>;
using Response = boost::beast::http::response<boost::beast::http::string_body>;

class RequestHandler{ // Pure virtual class (interface)
public:
  virtual Response* handle_request(const Request& req) = 0;
};

class RequestHandlerFactory{ // Pure virtual class (interface)
public:
  virtual RequestHandler* create() = 0;
};