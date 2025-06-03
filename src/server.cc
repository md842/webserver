#include <boost/asio.hpp> // io_context, tcp
#include <boost/asio/ssl.hpp> // ssl::context
#include <boost/bind/bind.hpp> // bind

#include "log.h"
#include "nginx_config.h" // Config
#include "server.h"

// Standardized log prefix for this source
#define LOG_PRE "[Server]   "

using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;


/// Initializes the server and starts listening for incoming connections.
server::server(Config& config, io_context& io_context, ssl::context& ssl_context)
  : acceptor_(io_context, tcp::endpoint(tcp::v4(), config.port)),
    config_(config),
    io_context_(io_context),
    ssl_context_(ssl_context){
  Log::info(LOG_PRE, "Listening on port " + std::to_string(config.port));
  start_accept();
}


/// Listens for and accepts an incoming connection, then calls handle_accept.
void server::start_accept(){
  session* new_session = new session(config_, io_context_, ssl_context_);
  acceptor_.async_accept(new_session->socket(),
                         boost::bind(&server::handle_accept, this, new_session,
                         placeholders::error));
}


/// Accept handler, called after start_accept() accepts incoming connection.
void server::handle_accept(session* new_session, const error_code& error){
  if (!error)
    new_session->do_handshake();
  else{
    Log::error(LOG_PRE, "Error accepting connection: " + error.message());
    delete new_session;
  }
  start_accept();
}