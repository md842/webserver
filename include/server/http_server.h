#pragma once

#include "server/server.h" // server
#include "typedefs/socket.h" // http_socket

class http_server : public server<http_socket>{
public:
  /** 
   * Initializes the server and starts listening for incoming connections.
   *
   * @pre ConfigParser::parse() succeeded.
   * @param config A pointer to a parsed Config object that supplies server parameters.
   * @param io_context A reference to boost::asio::io_context supplied by main.
   */
  http_server(Config* config, boost::asio::io_context& io_context);

private:
  void start_accept() override;
};