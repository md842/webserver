#pragma once

#include "nginx_config.h" // Config
#include "request_handler_interface.h" // RequestHandler

class session{
public:
  /** 
   * Sets up the session socket.
   *
   * @pre ConfigParser::parse() succeeded.
   * @param config A parsed Config object that supplies session parameters.
   * @param io_context The boost::asio::io_context supplied by main.
   * @param ssl_context The boost::asio::ssl::context supplied by main.
   */
  session(Config& config, boost::asio::io_context& io_context,
          boost::asio::ssl::context& ssl_context);

  /// Returns a reference to the TCP socket used by this session.
  boost::asio::ip::tcp::socket::lowest_layer_type& socket();

  /// Asynchronously reads incoming data from socket_, then calls handle_read.
  void do_handshake();

private:
  void do_read(const boost::system::error_code& error);
  void handle_read(const boost::system::error_code& error, size_t bytes);
  void create_response(const boost::system::error_code& error, int status);
  void create_response(const boost::system::error_code& error, Request& req);
  void do_write(const boost::system::error_code& error, Response* res,
                size_t req_bytes, const std::string& req_summary,
                const std::string& invalid_req);
  void do_close(int severity, const std::string& message);
  void handle_close(const boost::system::error_code& error);
  RequestHandler* dispatch(Request& req);
  
  std::string client_ip_;
  Config config_;
  enum{max_length = 1024};
  char data_[max_length];
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket_;
  std::string total_received_data_ = "";
};