#pragma once

#include "session.h"

class server{
public:
  /** 
   * Initializes the server and starts listening for incoming connections.
   *
   * The port to listen on is extracted from the config.
   *
   * @param io_service The boost::asio::io_service supplied by main.
   */
  server(boost::asio::io_service& io_service);

private:
  void start_accept();
  void handle_accept(session* new_session,
                     const boost::system::error_code& error);
  boost::asio::io_service& io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
  int session_id;
};