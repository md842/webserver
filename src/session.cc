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
Request parse_req(const std::string& received);
int verify_req(Request& req);
// std::string req_as_string(Request req); // Helper function for debugging
// std::string res_as_string(Response res); // Helper function for debugging


/// Sets up the session socket.
session::session(io_service& io_service, int id) : socket_(io_service){
  id_ = std::to_string(id);
}


/// Returns a reference to the TCP socket used by this session.
tcp::socket& session::socket(){
  return socket_;
}


/// Asynchronously reads incoming data from socket_, then calls handle_read.
void session::do_read(){
  socket_.async_read_some(buffer(data_, max_length),
                          boost::bind(&session::handle_read, this,
                          placeholders::error,
                          placeholders::bytes_transferred));
}


/// Read handler, called after start() reads incoming data.
void session::handle_read(const error_code& error, size_t bytes){
  try{ // Throws boost::system::system_error
    client_ip_ = socket_.remote_endpoint().address().to_string(); 
  }
  catch(boost::system::system_error){ // Thrown by socket::remote_endpoint()
    close_session(1, "Connection to " + client_ip_ + " unexpectedly terminated.");
    return;
  }

  Log::info("Session (ID " + id_ + "): " +
            "Received " + std::to_string(bytes) + " bytes from " + client_ip_);
  
  if (!error){
    // Append incoming data from read buffer (data_) to total received data
    total_received_data_ += std::string(data_, bytes);

    // Treat excessively long requests as malicious
    if (total_received_data_.length() > max_length * 8)
      create_response(error, 413); // Create 413 Payload Too Large response

    Request req = parse_req(total_received_data_);
    /* Because only max_length bytes can be read at a time, it is possible to
       receive an incomplete request from one read. We need to ensure that the
       full request has been read before processing it. */

    /* If the Content-Length header is present, we know the complete request
       has been read when payload size matches Content-Length. */
    if (req.has_content_length()){
      try{
        // Throws std::invalid_argument
        int content_length = std::stoi(req.at(http::field::content_length));
        int payload_size = 0;

        boost::optional<uint64_t> payload_size_opt = req.payload_size();
        if (payload_size_opt) // boost::optional has value
          payload_size = *payload_size_opt; // Extract and set value

        if (payload_size < content_length)
          do_read(); // Request is incomplete, continue reading incoming data
        else if (payload_size == content_length)
          create_response(error, req); // Request is complete, handle it
        else{ // Payload size exceeds Content-Length; should not happen!
          create_response(error, 400); // Create 400 Bad Request response
        }
      }
      catch(std::invalid_argument){ // Thrown by std::stoi()
        create_response(error, 400); // Create 400 Bad Request response
      }
    }

    /* Otherwise, there should be no body, so we know the complete request has
       been read when bytes < max_length or the request ends with \r\n\r\n. */
    else{
      if (bytes < max_length)
        create_response(error, req); // Request is complete, handle it
      else if (total_received_data_.substr(
        total_received_data_.length() - 4) == "\r\n\r\n")
        /* This condition would suffice for all requests with no body. However,
         it is a rare case (complete request length perfectly divisible by
         max_length) that is slower than comparing bytes < max_length, so the
         above comparison is preferred and this is left as a fallback. */
        create_response(error, req); // Request is complete, process it
      else
        do_read(); // Request is incomplete, continue reading incoming data
    }
  }
  else if (error == error::eof)
    // Client sends EOF when closed or keep-alive times out.
    // Expected behavior, log as info rather than error and shut down.
    close_session(0, "Keep-alive connection closed by client, shutting down.");
  else // Unknown read error, log as error and shut down.
    close_session(2, error.message() + "while reading request, shutting down.");
}


/// Creates an error response to an invalid request.
void session::create_response(const error_code& error, int status){
  Log::info("Session (ID " + id_ + "): " +
            "Received invalid request from " + client_ip_);

  total_received_data_ = ""; // Clear total received data
  Response* res = new Response();
  res->result(status); // Set response status code to specified error status
  res->version(11);

  do_write(error, res);
}


/// Creates a response by dispatching a RequestHandler for a valid request.
void session::create_response(const error_code& error, Request& req){
  Log::info("Session (ID " + id_ + "): " +
            "Received valid request from " + client_ip_);

  // Log::trace("Incoming HTTP request:\n\n" + req_as_string(req)); // Temp debug log

  total_received_data_ = ""; // Clear total received data
  Response* res = nullptr; // To be defined based on result of verify_req

  int req_error = verify_req(req); // Returns 0 if request valid, else err code
  if (req_error){ // Invalid request, generate an error response
    res = new Response();
    res->result(req_error); // Set response status code to verify_req() output
    res->version(11);
  }
  else{ // Valid request, dispatch a request handler to obtain response
    RequestHandler* handler = dispatch(req);
    res = handler->handle_request(req);
    free(handler); // Free memory used by request handler
  }
  do_write(error, res);
}


/// Given a pointer to a Response object, writes the response to the client.
void session::do_write(const error_code& error, Response* res){
  // Log::trace("Outgoing HTTP response:\n\n" + res_as_string(*res)); // Temp debug log

  // async_write returns immediately, so res must be kept alive.
  // Lambda write handler captures res and deletes it after write finishes.
  http::async_write(socket_, *res,
    [this, res](error_code ec, size_t bytes){
      if (!ec){ // Successful write
        Log::info("Session (ID " + id_ + "): " +
                  "Wrote " + std::to_string(bytes) + " bytes.");
        if (res->keep_alive()){ // Connection: keep-alive was requested
          free(res); // Free memory used by HTTP response object
          do_read(); // Continue listening for requests
        }
        else{ // Connection: close was requested
          free(res); // Free memory used by HTTP response object
          close_session(0, "Connection: close specified, shutting down.");
        }
      }
      else{ // Error during write
        free(res); // Free memory used by HTTP response object
        close_session(2, ec.message() + " error while writing response, shutting down.");
      }
    });
}


/// Helper function that logs a message and closes the session.
void session::close_session(int severity, const std::string& message){
  std::string full_msg = "Session (ID " + id_ + "): " + message;
  if (severity == 0) // info
    Log::info(full_msg);
  else if (severity == 1) // warning
    Log::warn(full_msg);
  else if (severity == 2) // error
    Log::error(full_msg);

  error_code ec;
  socket_.shutdown(tcp::socket::shutdown_both, ec); // Close connection
  delete this;
}


/// Dynamically dispatches a RequestHandler based on the given request target.
RequestHandler* dispatch(Request& req){
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


/// Parses a Request object given data received from the client.
Request parse_req(const std::string& received){
  Request req;
  http::parser<true, http::string_body> parser{std::move(req)};
  error_code ec;
  parser.eager(true); // Tells parser to read request body for POST requests
  parser.put(buffer(received, received.length()), ec);
  req = parser.release();
  return req;
}


/// Verifies a given Request object. Returns status code if an error is found.
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

  // Verify target does not try to access unintended files using "../" in URI
  if (req.target().find("../") != std::string::npos)
    return 403; // 403 Forbidden

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
  /* Verify POST request has Content-Length header. Payload length limit and
     matching are already enforced in handle_read, no need to handle here. */
  if (method == http::verb::post){
    if (!req.has_content_length())
      return 411; // 411 Length Required
  }
  return 0;
}


/*
/// Helper function for debugging. Converts given Request object to a string.
std::string req_as_string(Request req){
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


/// Helper function for debugging. Converts given Response object to a string.
std::string res_as_string(Response res){ // Temp helper for debug logging
  std::string result = std::to_string(res.result_int()) + " " + std::string(res.reason());
  std::string out = "HTTP/1.1 " + result + '\n';
  for (auto& header : res.base()){
    out += std::string(header.name_string()) + ": " +
           std::string(header.value()) + "\n";
  }
  return out;
}
*/