#pragma once

#include <boost/beast.hpp> // http::request, http::response

namespace http = boost::beast::http;

typedef http::request<http::string_body> Request;
typedef http::response<http::string_body> Response;