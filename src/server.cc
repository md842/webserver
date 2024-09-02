#include <boost/asio.hpp> // io_service, tcp
#include <boost/bind/bind.hpp> // bind

#include "log.h"
#include "nginx_config_parser.h" // Config::inst()
#include "server.h"

using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;

server::server(io_service& io_service)
  : io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), Config::inst().port())){
  Log::info("Server: Listening on port " +
            std::to_string(Config::inst().port()));
  session_id = 0;
  start_accept();
}

void server::start_accept(){
  // Listens for and accepts incoming connection, then calls accept handler.
  session_id++;
  session* new_session = new session(io_service_, session_id);
  acceptor_.async_accept(new_session->socket(),
                         boost::bind(&server::handle_accept, this, new_session,
                         placeholders::error));
}

void server::handle_accept(session* new_session, const error_code& error){
  // Accept handler, called after start_accept() accepts incoming connection.
  if (!error){
    Log::info("Server: Starting a new session (ID " + std::to_string(session_id) + ").");
    new_session->do_read();
  }
  else{
    Log::error("Server: Error accepting connection: " + error.message());
    delete new_session;
  }
  start_accept();
}