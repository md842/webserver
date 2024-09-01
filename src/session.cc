#include <boost/asio.hpp> // io_service, tcp
#include <boost/bind/bind.hpp> // bind

#include "log.h"
#include "nginx_config_parser.h" // Config::inst()
#include "registry.h" // Registry::inst()
#include "session.h"

using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;
namespace http = boost::beast::http;

RequestHandler* dispatch(Request& req);
Request parse_req(char* data, int max_length);
int verify_req(Request& req);

std::string req_as_string(Request req); // Temp helper for debug logging
std::string res_as_string(Response res); // Temp helper for debug logging

session::session(io_service& io_service, int id) : socket_(io_service){
  id_ = std::to_string(id);
}

tcp::socket& session::socket(){
  return socket_;
}

void session::start(){
  // Reads an incoming request, then calls read handler.
  socket_.async_read_some(buffer(data_, max_length),
                          boost::bind(&session::handle_read, this,
                          placeholders::error,
                          placeholders::bytes_transferred));
}

void session::handle_read(const error_code& error, size_t bytes){
  // Read handler, called after start() reads incoming request.
  if (!error){
    std::string client = socket_.remote_endpoint().address().to_string();
    Log::info("Session (ID " + id_ + "): " +
              "Received request from client " + client);

    Log::trace("Bytes received: " + std::to_string(bytes));
    Log::trace("Incoming raw data: " + std::string(data_));

    Request req = parse_req(data_, max_length);
    Response* res = nullptr; // To be defined based on result of verify_req

    Log::trace("Incoming HTTP request:\n\n" + req_as_string(req)); // Temp debug log

    int error = verify_req(req); // Returns 0 if request valid, else error code
    if (error){ // Invalid request, generate an error response
      res = new Response();
      res->result(error); // Set response status code to output of verify_req()
      res->version(11);
    }
    else{ // Valid request, dispatch a request handler to obtain response
      RequestHandler* handler = dispatch(req);
      res = handler->handle_request(req);
      free(handler); // Free memory used by request handler
    }

    Log::trace("Outgoing HTTP response:\n\n" + res_as_string(*res)); // Temp debug log

    // async_write returns immediately, so res must be kept alive.
    // Lambda write handler captures res and deletes it after write finishes.
    http::async_write(socket_, *res,
      [this, res](error_code ec, size_t bytes){
        if (!ec){ // Successful write
          Log::info("Session (ID " + id_ + "): " +
                    "Wrote " + std::to_string(bytes) + " bytes.");
          if (res->keep_alive()){ // Connection: keep-alive was requested
            free(res); // Free memory used by HTTP response object
            start(); // Continue listening for requests
          }
          else{ // Connection: close was requested
            Log::info("Session (ID " + id_ + "): " +
                      "Connection: close specified, shutting down.");
            socket_.shutdown(tcp::socket::shutdown_both, ec); // Close connection
            free(res); // Free memory used by HTTP response object
            delete this;
          }
        }
        else{ // Error during write
          Log::error("Session (ID " + id_ + "): " +
                     ec.message() + " error while writing response, shutting down.");
          socket_.shutdown(tcp::socket::shutdown_both, ec); // Close connection
          free(res); // Free memory used by HTTP response object
          delete this;
        }
      });

  }
  else if (error == error::eof){
    // Client sends EOF when closed or keep-alive times out.
    // Expected behavior, log as info rather than error, and shut down.
    Log::info("Session (ID " + id_ + "): " +
              "Keep-alive connection closed by client, shutting down.");
    error_code ec;
    socket_.shutdown(tcp::socket::shutdown_both, ec); // Close connection
    delete this;
  }
  else{ // Unknown read error, log as error and shut down
    Log::error("Session (ID " + id_ + "): " +
               error.message() + "while reading request, shutting down.");
    error_code ec;
    socket_.shutdown(tcp::socket::shutdown_both, ec); // Close connection
    delete this;
  }
}

RequestHandler* dispatch(Request& req){
  // Returns a pointer to a dynamically dispatched RequestHandler based on the
  // incoming request's target.
  std::string type = "";

  if (req.method() == http::verb::post)
    type = "PostRequestHandler"; // Handles all POST requests
  else{
    // Search for longest match between target and URI
    int longest_match = 0;
    std::string target = std::string(req.target());
    // Search all handler types
    for (const std::string& cur_type : Registry::inst().get_types()){
      // Search the URI map for the current handler type
      for (auto& pair : Registry::inst().get_map(cur_type)){
        std::string key = pair.first;
        // New longest match found, save information
        if ((target.find(key) == 0) && key.length() > longest_match){
          longest_match = key.length();
          type = cur_type;
        }
      }
    }
  }
  return Registry::inst().get_factory(type)->create();
}

Request parse_req(char* data, int max_length){
  Request req;
  http::parser<true, http::string_body> parser{std::move(req)};
  error_code ec;
  parser.eager(true); // Tells parser to read request body for POST requests
  parser.put(buffer(data, max_length), ec);
  req = parser.release();
  return req;
}

int verify_req(Request& req){
  http::verb method = req.method();
  // Verify HTTP method: GET, POST are allowed
  switch(method){
    case http::verb::unknown:
      return 400; // 400 Bad Request
    case http::verb::delete_:
    case http::verb::put:
    case http::verb::connect:
    case http::verb::options:
    case http::verb::trace:
      return 405; // 405 Method Not Allowed
  }
  // Verify HTTP version: HTTP/0.9, HTTP/1.0, HTTP/1.1, HTTP/2.0, or HTTP/3.0
  switch(req.version()){
    case 11:
      break;
    case 20:
      break;
    case 30:
      break;
    case 10:
      break;
    case 9:
      break;
    default:
      return 505; // 505 HTTP Version Not Supported
  }
  // Verify length of payload for POST request: Must be present and under 4096
  if (method == http::verb::post){
    if (req.has_content_length()){
      Log::trace("Content Length: " + std::string(req.at(http::field::content_length)));
      try{
        // Throws std::invalid_argument
        int content_length = std::stoi(req.at(http::field::content_length));
        if (content_length > 4096)
          return 413; // 413 Payload Too Large

        /* As a security measure, we will also examine the payload size instead
           of simply trusting the Content-Length header to be accurate. */
        boost::optional<uint64_t> payload_size = req.payload_size();
        if (payload_size){ // boost::optional has value
          if (*payload_size > 4096) // value exceeds 4096
            return 413; // 413 Payload Too Large
        }
      }
      catch(std::invalid_argument){ // Thrown by std::stoi()
        return 400; // 400 Bad Request
      }
    }
    else
      return 411; // 411 Length Required
  }
  return 0;
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
  std::string body = req.body();
  if (body.length()){
    out += '\n' + body + '\n';
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