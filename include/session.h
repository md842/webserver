#pragma once

#include "request_handler_interface.h" // RequestHandler*

class session{
public:
  session(boost::asio::io_service& io_service, int id);
  boost::asio::ip::tcp::socket& socket();
  void start();

private:
  RequestHandler* dispatch(Request& req);
  void handle_read(const boost::system::error_code& error, size_t bytes);
  boost::asio::ip::tcp::socket socket_;
  enum{max_length = 1024};
  char data_[max_length];
  std::string id_;
};