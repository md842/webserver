#include <boost/asio.hpp> // io_context, tcp
#include <boost/asio/ssl.hpp> // ssl::context
#include <boost/bind/bind.hpp> // bind

#include "log.h"
#include "server/https_server.h"
#include "session/https_session.h"

// Standardized log prefix for this source
#define LOG_PRE "[Server]   "

using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;


/// Initializes the server and starts listening for incoming connections.
https_server::https_server(Config& config, io_context& io_context, ssl::context& ssl_context)
  : server(config, io_context), // Call superclass constructor
    ssl_context_(ssl_context){
  Log::info(LOG_PRE, "HTTPS server listening on port " + std::to_string(config.port));
  start_accept();
}


/// Accepts incoming connection, creates new session, then calls handle_accept.
void https_server::start_accept(){
  session* new_session = new https_session(config_, io_context_, ssl_context_);
  acceptor_.async_accept(new_session->socket(),
                         boost::bind(&https_server::handle_accept, this,
                                     new_session, placeholders::error));
}