#include <boost/asio.hpp> // io_service, tcp
#include <boost/bind/bind.hpp> // bind
#include <boost/log/trivial.hpp> // BOOST_LOG_TRIVIAL

#include "log.h"
#include "nginx_config_parser.h" // Config::inst()
#include "registry.h" // Registry::inst()
#include "session.h"

// Standardized log prefix for this source
#define LOG_PRE "[Session]            "

using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;
namespace http = boost::beast::http;


RequestHandler* dispatch(Request& req);
Request parse_req(const std::string& received);
int verify_req(Request& req);
std::string req_as_string(Request req); // Helper function for debugging
// std::string res_as_string(Response res); // Helper function for debugging


/// Sets up the session socket.
session::session(io_service& io_service) : socket_(io_service){}


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
    close_session(0, "Client disconnected from session, shutting down.");
    return;
  }
  if (!error){
    // Append incoming data from read buffer (data_) to total received data
    total_received_data_ += std::string(data_, bytes);

    // Treat excessively long requests as malicious
    if (total_received_data_.length() > max_length * 8)
      create_response(error,
                      "(Invalid, payload size is excessive)",
                      413); // 413 Payload Too Large

    Request req = parse_req(total_received_data_);
    
    /* Check if the parsed request is complete. If so, process it. 
       Because only max_length bytes can be read at a time, it is possible to
       receive an incomplete request from one read. */
    if (req.has_content_length()){
      /* If the Content-Length header is present, we know the complete request
         has been read when payload size matches Content-Length. */
      try{
        // Throws std::invalid_argument
        int content_length = std::stoi(req.at(http::field::content_length));
        int payload_size = 0;

        boost::optional<uint64_t> payload_size_opt = req.payload_size();
        if (payload_size_opt) // boost::optional has value
          payload_size = *payload_size_opt; // Extract and set value

        if (payload_size < content_length) // Request is incomplete
          do_read(); // Continue reading incoming data
        else if (payload_size == content_length) // Request is complete
          create_response(error, req); // Create response
        else{
          create_response(error,
                          "(Invalid, payload size exceeds Content-Length)",
                          400); // 400 Bad Request
        }
      }
      catch(std::invalid_argument){ // Thrown by std::stoi() for non-numeric
        create_response(error,
                        "(Invalid, non-numeric Content-Length argument)",
                        400); // 400 Bad Request
      }
    }
    else{
      /* If the Content-Length header is not present, there should be no body,
         so we know the complete request has been read when bytes < max_length
         or the request ends with \r\n\r\n. */
      if (bytes < max_length)
        create_response(error, req); // Request is complete, create response
      else if (total_received_data_.substr(
        total_received_data_.length() - 4) == "\r\n\r\n")
        /* This condition would suffice for all requests with no body. However,
        there is a rare case (complete request length perfectly divisible by
        max_length) that is slower than comparing bytes < max_length, so the
        above comparison is preferred and this is left as a fallback. */
        create_response(error, req); // Request is complete, create response
      else // Request is incomplete
        do_read(); // Continue reading incoming data
    }
  }
  else if (error == error::eof)
    // Client sends EOF when closed or keep-alive times out.
    // Expected behavior, log as info rather than error and shut down.
    close_session(0, "Keep-alive connection closed by client, shutting down.");
  else // Unknown read error, log as error and shut down.
    close_session(2, "Got error \"" + error.message() + "\" while reading request, shutting down.");
}


/* Overload 1 of 2:
   Create an error response to a given reason and status. */
void session::create_response(const error_code& error,
                              const std::string& reason, int status){
  size_t req_bytes = total_received_data_.length();

  total_received_data_ = ""; // Clear total received data
  Response* res = new Response();
  res->result(status); // Set response status code to specified error status
  res->version(11);

  do_write(error, res, req_bytes, reason, "");
}


/* Overload 2 of 2:
   Create a response to a request that parsed successfully (may not be valid!)
   For a valid request, dispatches a RequestHandler to create the response.
   For an invalid request, create an error response and log the request. */
void session::create_response(const error_code& error, Request& req){
  size_t req_bytes = total_received_data_.length();
  std::string req_summary = req.method_string();
  req_summary += " " + std::string(req.target());
  std::string invalid_req = "";

  total_received_data_ = ""; // Clear total received data
  Response* res = nullptr; // To be defined by the request handler

  int req_error = verify_req(req); // Returns 0 if request valid, else err code
  if (req_error){ // Invalid request, generate an error response
    res = new Response();
    res->result(req_error); // Set response status code to verify_req() output
    res->version(11);
    req_summary = "(Invalid)"; // Indicate invalid request in log
    invalid_req = req_as_string(req);
  }
  else{ // Valid request, dispatch a request handler to obtain response
    RequestHandler* handler = dispatch(req);
    res = handler->handle_request(req);
    free(handler); // Free memory used by request handler
  }

  do_write(error, res, req_bytes, req_summary, invalid_req);
}


/// Given a pointer to a Response object, writes the response to the client.
void session::do_write(const error_code& error, Response* res,
                       size_t req_bytes, const std::string& req_summary,
                       const std::string& invalid_req){

  // async_write returns immediately, so res must be kept alive.
  // Lambda write handler captures res and deletes it after write finishes.
  http::async_write(socket_, *res,
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
        close_session(2, "Got error \"" + ec.message() + "\" while writing response, shutting down.");
      }
    }
  );
}


/// Helper function that logs a message and closes the session.
void session::close_session(int severity, const std::string& message){
  std::string full_msg = "Client: " + client_ip_ + " | " + message;
  if (severity == 0) // info
    Log::info(LOG_PRE, full_msg);
  else if (severity == 1) // warning
    Log::warn(LOG_PRE, full_msg);
  else if (severity == 2) // error
    Log::error(LOG_PRE, full_msg);

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
    case http::verb::head:
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


/// Helper function for invalid request logging. Converts req to a string.
std::string req_as_string(Request req){
  std::string method = req.method_string();
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


/*
/// Helper function for debugging. Converts res to a string.
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