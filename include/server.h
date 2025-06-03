#pragma once

#include "session.h"

class server{
public:
  /** 
   * Initializes the server and starts listening for incoming connections.
   *
   * @pre ConfigParser::parse() succeeded.
   * @param config A parsed Config object that supplies server parameters.
   * @param io_context The boost::asio::io_context supplied by main.
   * @param ssl_context The boost::asio::ssl::context supplied by main.
   */
  server(Config& config, boost::asio::io_context& io_context,
         boost::asio::ssl::context& ssl_context);

private:
  void start_accept();
  void handle_accept(session* new_session,
                     const boost::system::error_code& error);
  
  boost::asio::ip::tcp::acceptor acceptor_;
  Config config_;
  boost::asio::io_context& io_context_;
  boost::asio::ssl::context& ssl_context_;
};