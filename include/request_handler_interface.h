#pragma once

#include <boost/beast.hpp>
#include <string>

using Request = boost::beast::http::request<boost::beast::http::string_body>;
using Response = boost::beast::http::response<boost::beast::http::string_body>;

class RequestHandler{ // Pure virtual class (interface)
public:
  RequestHandler(const std::string& root_path){root_path_ = root_path;}
  virtual Response* handle_request(const Request& req) = 0;

protected:
  std::string root_path_;
};

class RequestHandlerFactory{ // Pure virtual class (interface)
public:
  virtual RequestHandler* create(const std::string& root_path) = 0;
};