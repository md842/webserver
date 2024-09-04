#pragma once

#include "request_handler_interface.h" // RequestHandler

class session{
public:
  /** 
   * Sets up the session socket.
   *
   * @param io_service The boost::asio::io_service supplied by main.
   */
  session(boost::asio::io_service& io_service);

  /// Returns a reference to the TCP socket used by this session.
  boost::asio::ip::tcp::socket& socket();

  /// Asynchronously reads incoming data from socket_, then calls handle_read.
  void do_read();

private:
  void handle_read(const boost::system::error_code& error, size_t bytes);
  void create_response(const boost::system::error_code& error, int status);
  void create_response(const boost::system::error_code& error, Request& req);
  void do_write(const boost::system::error_code& error, Response* res, size_t req_bytes, const std::string& method);
  void close_session(int severity, const std::string& message);
  boost::asio::ip::tcp::socket socket_;
  enum{max_length = 1024};
  char data_[max_length];
  std::string client_ip_;
  std::string total_received_data_ = "";
};