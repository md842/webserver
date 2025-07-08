#include <boost/asio.hpp> // buffer, placeholders
#include <boost/bind/bind.hpp> // bind

#include "session/session.h"
#include "typedefs/socket.h" // http_socket, https_socket

// Standardized log prefix for this source
#define LOG_PRE "[Session]  "

using namespace boost::asio;


/// Asynchronously reads incoming data from socket_, then calls handle_read.
template <class AsyncWriteStream>
void session<AsyncWriteStream>::do_read(){
  socket_->async_read_some(buffer(data_, max_length),
                           boost::bind(&session::handle_read, this,
                                       placeholders::error,
                                       placeholders::bytes_transferred));
}


/// Given a pointer to a Response object, writes the response to the client.
template <class AsyncWriteStream>
void session<AsyncWriteStream>::do_write(Response* res,
                                         Log::req_info& req_info){
  total_received_data_ = ""; // Clear total received data
  // async_write returns immediately, res must be kept alive for handle_write.
  http::async_write(*socket_, *res,
                    boost::bind(&session::handle_write, this,
                                placeholders::error,
                                placeholders::bytes_transferred,
                                res, req_info));
}


// Explicit instantiation of template types
template class session<http_socket>;
template class session<https_socket>;