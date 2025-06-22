#pragma once

#include <boost/asio.hpp> // ip::tcp
#include <boost/asio/ssl.hpp> // ssl::stream

typedef boost::asio::ip::tcp::socket http_socket;
typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> https_socket;