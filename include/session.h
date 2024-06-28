#pragma once

using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;

class session{
public:
  session(io_service& io_service);
  tcp::socket& socket();
  void start();

private:
  void handle_read(const error_code& error, size_t bytes);
  void handle_write(const error_code& error);
  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
};