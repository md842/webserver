#include <boost/asio.hpp> // io_service, tcp
#include <boost/bind/bind.hpp> // bind

#include "log.h"
#include "server.h"
#include "session.h"

server::server(io_service& io_service, short port)
  : io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port)){
  Log::info("Server starting on port " + std::to_string(port));
  start_accept();
}

void server::start_accept(){
  session* new_session = new session(io_service_);
  acceptor_.async_accept(new_session->socket(),
                         boost::bind(&server::handle_accept, this, new_session,
                         placeholders::error));
}

void server::handle_accept(session* new_session, const error_code& error){
  if (!error){
    new_session->start();
  }
  else{
    Log::error("Error accepting connection: " + error.message());
    delete new_session;
  }
  start_accept();
}