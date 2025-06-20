#pragma once

#include "session/session.h" // session

class https_session : public session{
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
  void handle_handshake(const boost::system::error_code& error);
  void do_read();
  void do_write(Response* res, size_t req_bytes,
                const std::string& req_summary,
                const std::string& invalid_req);
  void do_close(int severity, const std::string& message);
  void handle_close(const boost::system::error_code& error);

  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket_;
};