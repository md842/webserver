#pragma once

#include "server/server.h"

class https_server : public server{
public:
  /** 
   * Initializes the server and starts listening for incoming connections.
   *
   * @pre ConfigParser::parse() succeeded.
   * @param config A parsed Config object that supplies server parameters.
   * @param io_context The boost::asio::io_context supplied by main.
   * @param ssl_context The boost::asio::ssl::context supplied by main.
   */
  https_server(Config& config, boost::asio::io_context& io_context,
               boost::asio::ssl::context& ssl_context);

private:
  void start_accept();
  
  boost::asio::ssl::context& ssl_context_;
};