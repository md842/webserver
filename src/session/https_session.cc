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
  ssl_socket_.async_handshake(ssl::stream_base::server,
                              boost::bind(&https_session::handle_handshake,
                                          this, placeholders::error));
}


/// Checks if the SSL handshake succeeded, then calls do_read.
void https_session::handle_handshake(const error_code& error){
  if (error && error != ssl::error::stream_truncated) // Ignore stream truncated
    do_close(2, "Got error \"" + error.message() + "\" while performing SSL handshake, shutting down.");
  else
    do_read();
}


/// Asynchronously reads incoming data from socket_, then calls handle_read.
void https_session::do_read(){
  ssl_socket_.async_read_some(buffer(data_, max_length),
                              boost::bind(&https_session::handle_read, this,
                              placeholders::error,
                              placeholders::bytes_transferred));
}


/// Given a pointer to a Response object, writes the response to the client.
void https_session::do_write(Response* res, size_t req_bytes,
                             const std::string& req_summary,
                             const std::string& invalid_req){
  // async_write returns immediately, res must be kept alive for handle_write.
  http::async_write(ssl_socket_, *res,
                    boost::bind(&https_session::handle_write, this,
                                placeholders::error,
                                placeholders::bytes_transferred,
                                res, req_bytes, req_summary, invalid_req));
}


/// Helper function that logs a message and closes the session.
void https_session::do_close(int severity, const std::string& message){
  // Shut down gracefully
  ssl_socket_.async_shutdown(boost::bind(&https_session::handle_close, this,
                             placeholders::error));
}


void https_session::handle_close(const error_code& error){
  if (error)
    Log::error(LOG_PRE, error.message());
  delete this;
}