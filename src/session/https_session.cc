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


/// Asynchronously performs SSL handshake, then calls do_read.
void https_session::do_handshake(){
  ssl_socket_.async_handshake(ssl::stream_base::server,
                              boost::bind(&https_session::do_read, this,
                                          placeholders::error));
}


/// Asynchronously reads incoming data from socket_, then calls handle_read.
void https_session::do_read(const error_code& error){
  if (error && error != ssl::error::stream_truncated) // Ignore stream truncated
    do_close(2, "Got error \"" + error.message() + "\" while performing SSL handshake, shutting down.");
  else{
    ssl_socket_.async_read_some(buffer(data_, max_length),
                                boost::bind(&https_session::handle_read, this,
                                placeholders::error,
                                placeholders::bytes_transferred));
  }
}


// TODO: See if it's possible to externalize the lambda write handler to have less to override
/// Given a pointer to a Response object, writes the response to the client.
void https_session::do_write(const error_code& error, Response* res,
                       size_t req_bytes, const std::string& req_summary,
                       const std::string& invalid_req){

  // async_write returns immediately, so res must be kept alive.
  // Lambda write handler captures res and deletes it after write finishes.
  http::async_write(ssl_socket_, *res,
    [this, res, req_bytes, req_summary, invalid_req]
    (error_code ec, size_t res_bytes){
      if (!ec){ // Successful write
        Log::res_metrics( // Write machine-parseable formatted log
          client_ip_,
          req_bytes,
          res_bytes,
          req_summary,
          invalid_req,
          res->result_int()
        );
        if (res->result_int() == 413){ // 413 Payload Too Large
          free(res); // Free memory used by HTTP response object
          do_close(1, "Client attempted to send an excessive payload, shutting down.");
        }
        else if (res->keep_alive()){ // Connection: keep-alive was requested
          free(res); // Free memory used by HTTP response object
          do_read(ec); // Continue listening for requests
        }
        else{ // Connection: close was requested
          free(res); // Free memory used by HTTP response object
          do_close(0, "Connection: close specified, shutting down.");
        }
      }
      else{ // Error during write
        free(res); // Free memory used by HTTP response object
        do_close(2, "Got error \"" + ec.message() + "\" while writing response, shutting down.");
      }
    }
  );
}


/// Helper function that logs a message and closes the session.
void https_session::do_close(int severity, const std::string& message){
  std::string full_msg = "Client: " + client_ip_ + " | " + message;
  if (severity == 0) // info
    Log::info(LOG_PRE, full_msg);
  else if (severity == 1) // warning
    Log::warn(LOG_PRE, full_msg);
  else if (severity == 2) // error
    Log::error(LOG_PRE, full_msg);

  // Shut down gracefully
  ssl_socket_.async_shutdown(boost::bind(&https_session::handle_close, this,
                         placeholders::error));
}


void https_session::handle_close(const error_code& error){
  if (error)
    Log::error(LOG_PRE, error.message());
  delete this;
}