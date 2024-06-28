#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/log/trivial.hpp>

#include "session.h"

session::session(io_service& io_service) : socket_(io_service){
}

tcp::socket& session::socket(){
  return socket_;
}

void session::start(){
  std::string client_addr = socket_.remote_endpoint().address().to_string();
  BOOST_LOG_TRIVIAL(info) << "Connection started with client at IP: " + client_addr;
  socket_.async_read_some(buffer(data_, max_length),
                          boost::bind(&session::handle_read, this,
                          placeholders::error,
                          placeholders::bytes_transferred));
}

void session::handle_read(const error_code& error, size_t bytes){
  if (!error){
    std::string response = "HTTP/1.1 200 OK\r\n"
                           "Content-Length: 45\r\n"
                           "Content-Type: text/plain\r\n"
                           "\r\n"
                           "Placeholder Response. Pending implementation!";
    async_write(socket_, buffer(response, response.length()),
                boost::bind(&session::handle_write, this, placeholders::error));
  }
  else{
    delete this;
  }
}

void session::handle_write(const error_code& error){
  if (!error){
    socket_.async_read_some(buffer(data_, max_length),
                            boost::bind(&session::handle_read, this,
                            placeholders::error,
                            placeholders::bytes_transferred));
  }
  else{
    delete this;
  }
}