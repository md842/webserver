#include <boost/asio.hpp> // io_context, placeholders
#include <boost/bind/bind.hpp> // bind

#include "log.h"
#include "server/https_server.h"
#include "session/https_session.h" // https_session

// Standardized log prefix for this source
#define LOG_PRE "[Server]   "

using namespace boost::asio;


/// Initializes the server and starts listening for incoming connections.
https_server::https_server(Config* config, io_context& io_context)
  : server(config, io_context), // Call superclass constructor
    ssl_context_(ssl::context::tlsv12_server){

  // Configure SSL context
  ssl_context_.use_certificate_file(config->certificate, ssl::context::pem);
  ssl_context_.use_private_key_file(config->private_key, ssl::context::pem);

  Log::info(LOG_PRE, "HTTPS server listening on port " + std::to_string(config->port));
  start_accept();
}


/// Accepts incoming connection, creates new session, then calls handle_accept.
void https_server::start_accept(){
  session<https_socket>* new_session = new https_session(
    config_, io_context_, ssl_context_);
  acceptor_.async_accept(new_session->socket(),
                         boost::bind(&https_server::handle_accept, this,
                                     new_session, placeholders::error));
}