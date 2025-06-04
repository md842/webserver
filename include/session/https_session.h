#pragma once

#include "nginx_config.h" // Config
#include "request_handler_interface.h" // RequestHandler
#include "session/session.h" // session

class https_session : public session{
public:
  /** 
   * Sets up the session socket.
   *
   * @pre ConfigParser::parse() succeeded.
   * @param config A parsed Config object that supplies session parameters.
   * @param io_context The boost::asio::io_context supplied by main.
   * @param ssl_context The boost::asio::ssl::context supplied by main.
   */
  https_session(Config& config, boost::asio::io_context& io_context,
                boost::asio::ssl::context& ssl_context)
  : session(config, io_context), // Call superclass constructor
    ssl_socket_(io_context, ssl_context){}

  /// Returns a reference to the TCP socket used by this session.
  boost::asio::ip::tcp::socket::lowest_layer_type& socket() override{
    return ssl_socket_.lowest_layer();
  };

  /// The entry point for https_session is do_handshake()
  void start() override{do_handshake();};

private:
  void do_handshake();
  void do_read(const boost::system::error_code& error);
  void do_close(int severity, const std::string& message) override;
  void do_write(const boost::system::error_code& error, Response* res,
                size_t req_bytes, const std::string& req_summary,
                const std::string& invalid_req) override;
  void handle_close(const boost::system::error_code& error);

  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket_;
};