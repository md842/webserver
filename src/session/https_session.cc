#include <boost/asio.hpp> // io_context, tcp
#include <boost/asio/ssl.hpp> // ssl::context, ssl::stream
#include <boost/bind/bind.hpp> // bind

#include "log.h"
#include "session/https_session.h"

// Standardized log prefix for this source
#define LOG_PRE "[Session]  "

using namespace boost::asio;
using boost::system::error_code;
namespace http = boost::beast::http;


/// Asynchronously performs SSL handshake, then calls handle_handshake.
void https_session::do_handshake(){
  socket_->async_handshake(ssl::stream_base::server,
                           boost::bind(&https_session::handle_handshake,
                                       this, placeholders::error));
}


/// Checks if the SSL handshake succeeded, then calls do_read.
void https_session::handle_handshake(const error_code& error){
  if (error && error != ssl::error::stream_truncated) // Ignore stream truncated
    close(2, "Got error \"" + error.message() + "\" while performing SSL handshake, shutting down.");
  else
    do_read();
}

/// Closes the current session.
void https_session::do_close(){
  // Shut down gracefully
  socket_->async_shutdown(boost::bind(&https_session::handle_close, this,
                             placeholders::error));
}


void https_session::handle_close(const error_code& error){
  if (error)
    Log::error(LOG_PRE, error.message());
  delete this;
}