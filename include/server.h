#pragma once

#include "session.h"

class server{
public:
  server(boost::asio::io_service& io_service);

private:
  void start_accept();
  void handle_accept(session* new_session,
                     const boost::system::error_code& error);
  boost::asio::io_service& io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
  int session_id;
};