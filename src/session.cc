#include <boost/bind/bind.hpp> // bind

#include "log.h"
#include "request_handler_interface.h"
#include "request_handler_registry.h"
#include "session.h"

using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;
namespace http = boost::beast::http;

Request parse_req(char* data, int max_length);
std::string req_as_string(Request req); // Temp helper for debug logging
std::string res_as_string(Response res); // Temp helper for debug logging

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
    Request req = parse_req(data_, max_length);

    Log::trace("Incoming HTTP request:\n\n" + req_as_string(req)); // Temp debug log

    // Temporary hardcoded values for testing purposes
    std::string type = "FileRequestHandler";
    std::string path = "/files_to_serve";
    std::string file_name = "/index.html";
    // TODO: Extract path and filename from target

    // Dispatch the correct request handler type using the handler registry
    Log::trace("Dispatched " + type);
    RequestHandlerFactory* factory = RequestHandlerRegistry::inst().get_handler(type);
    RequestHandler* handler = factory->create(root_dir_ + path + file_name);
    Response* res = handler->handle_request(req);

    Log::trace("Outgoing HTTP response:\n\n" + res_as_string(*res)); // Temp debug log

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

Request parse_req(char* data, int max_length){
  Request req;
  http::parser<true, http::string_body> parser{std::move(req)};
  error_code ec;
  parser.put(buffer(data, max_length), ec);
  req = parser.release();
  return req;
}

std::string req_as_string(Request req){ // Temp helper for debug logging
  std::string method = std::string(http::to_string(req.method()));
  std::string target = std::string(req.target());
  std::string ver = std::to_string(req.version());
  std::string out = method + ' ' + target + " HTTP/" + ver[0] + '.' + ver[1] + '\n';
  for (auto& header : req.base()){
    out += std::string(header.name_string()) + ": " +
           std::string(header.value()) + "\n";
  }
  return out;
}

std::string res_as_string(Response res){ // Temp helper for debug logging
  std::string result = std::to_string(res.result_int()) + " " + std::string(res.reason());
  std::string out = "HTTP/1.1 " + result + '\n';
  for (auto& header : res.base()){
    out += std::string(header.name_string()) + ": " +
           std::string(header.value()) + "\n";
  }
  return out;
}