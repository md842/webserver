#include <boost/bind/bind.hpp> // bind

#include "log.h"
#include "request_handler_interface.h"
#include "request_handler_registry.h"
#include "session.h"

using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;
namespace http = boost::beast::http;

session::session(io_service& io_service, const std::string& root_dir)
  : socket_(io_service), root_dir_(root_dir){}

tcp::socket& session::socket(){
  return socket_;
}

void session::start(){
  // Reads an incoming request, then calls read handler.
  std::string client_addr = socket_.remote_endpoint().address().to_string();
  Log::info("Connection started with client " + client_addr);
  socket_.async_read_some(buffer(data_, max_length),
                          boost::bind(&session::handle_read, this,
                          placeholders::error,
                          placeholders::bytes_transferred));
}

void session::handle_read(const error_code& error, size_t bytes){
  // Read handler, called after start() reads incoming request.
  if (!error){
    // Temporary hardcoded values for testing purposes
    std::string type = "FileRequestHandler";
    std::string path = "/files_to_serve/index.html";

    Request req;
    req.method(http::verb::get);
    req.target("");
    req.version(11);
    req.prepare_payload();
    // TODO: Implement request parsing

    // Dispatch the correct request handler type using the handler registry
    Log::trace("Dispatched " + type);
    RequestHandlerFactory* factory = RequestHandlerRegistry::inst().get_handler(type);
    RequestHandler* handler = factory->create(root_dir_ + path);
    Response* res = handler->handle_request(req);

    // async_write returns immediately, so res must be kept alive.
    // Lambda write handler captures res and deletes it after write finishes.
    http::async_write(socket_, *res,
      [this, res](error_code ec, size_t bytes){
        Log::info("Wrote " + std::to_string(bytes) + " bytes.");
        free(res); // Free memory used by HTTP response object
        socket_.shutdown(tcp::socket::shutdown_both); // Close connection
      });

  }
  else{
    delete this;
  }
}