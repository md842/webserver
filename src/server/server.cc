#include <boost/asio.hpp> // io_context, ip::tcp

#include "log.h"
#include "server/server.h"
#include "typedefs/socket.h" // http_socket, https_socket

// Standardized log prefix for this source
#define LOG_PRE "[Server]   "

using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;


/// Initializes the server instance.
template <typename T>
server<T>::server(Config* config, io_context& io_context)
  : acceptor_(io_context, tcp::endpoint(tcp::v4(), config->port)),
    config_(config), io_context_(io_context){}


/// Accept handler, called after start_accept() accepts incoming connection.
template <typename T>
void server<T>::handle_accept(session<T>* new_session,
                              const error_code& error){
  start_accept(); // Immediately continue listening for incoming connections.
  if (!error) // Connection accepted successfully
    new_session->start(); // Entry point varies based on override
  else{
    Log::error(LOG_PRE, "Error accepting connection on port " +
               std::to_string(config_->port) + ": " + error.message());
    delete new_session;
  }
}


// Explicit instantiation of template types
template class server<http_socket>;
template class server<https_socket>;