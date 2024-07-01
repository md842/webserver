#pragma once

#include <boost/asio.hpp> // io_service, tcp

class session{
public:
  session(boost::asio::io_service& io_service);
  boost::asio::ip::tcp::socket& socket();
  void start();

private:
  void handle_read(const boost::system::error_code& error, size_t bytes);
  boost::asio::ip::tcp::socket socket_;
  enum{max_length = 1024};
  char data_[max_length];
};