#include <boost/asio.hpp> // io_context, placeholders
#include <boost/bind/bind.hpp> // bind

#include "log.h"
#include "server/http_server.h"
#include "session/http_session.h" // http_session

// Standardized log prefix for this source
#define LOG_PRE "[Server]   "

using namespace boost::asio;


/// Initializes the server and starts listening for incoming connections.
http_server::http_server(Config* config, io_context& io_context)
  : server(config, io_context){ // Call superclass constructor
  Log::info(LOG_PRE, "HTTP server listening on port " + std::to_string(config->port));
  start_accept();  // Start listening for incoming connections
}


/// Accepts incoming connection, creates new session, then calls handle_accept.
void http_server::start_accept(){
  session<http_socket>* new_session = new http_session(config_, io_context_);
  acceptor_.async_accept(new_session->socket(),
                         boost::bind(&http_server::handle_accept, this,
                                     new_session, placeholders::error));
}