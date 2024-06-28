#pragma once

#include "session.h"

using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;

class server{
public:
  server(io_service& io_service, short port);

private:
  void start_accept();
  void handle_accept(session* new_session, const error_code& error);
  io_service& io_service_;
  tcp::acceptor acceptor_;
};