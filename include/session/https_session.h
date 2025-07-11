#pragma once

#include "session/session.h" // session
#include "typedefs/socket.h" // http_socket, https_socket

class https_session : public session<https_socket>{
public:
  /** 
   * Sets up the session socket.
   *
   * @pre ConfigParser::parse() succeeded.
   * @param config A pointer to a parsed Config object that supplies session parameters.
   * @param io_context A reference to boost::asio::io_context supplied by main.
   * @param ssl_context A reference to the boost::asio::ssl::context supplied by https_server.
   */
  https_session(Config* config, boost::asio::io_context& io_context,
                boost::asio::ssl::context& ssl_context)
    : session(config){ // Call superclass constructor
    socket_ = new https_socket(io_context, ssl_context);
  }

  /// Returns a reference to the TCP socket used by this session.
  http_socket& socket() override{
    return socket_->next_layer();
  };

  /// The entry point for https_session is do_handshake()
  void start() override{
    do_handshake();
  };

private:
  void do_handshake();
  void handle_handshake(const boost::system::error_code& error);
  void do_close() override;
};