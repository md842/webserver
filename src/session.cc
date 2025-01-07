#include <boost/asio.hpp> // io_service, tcp
#include <boost/bind/bind.hpp> // bind

#include "log.h"
#include "nginx_config_parser.h" // Config::inst()
#include "registry.h" // Registry::inst()
#include "session.h"

// Standardized log prefix for this source
#define LOG_PRE "[Session]  "

using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;
namespace http = boost::beast::http;


RequestHandler* dispatch(Request& req);
Request parse_req(const std::string& received);
int verify_req(Request& req);
std::string proc_invalid_req(const std::string& received); // Helper function for invalid request logging


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
    close_session(0, "Client disconnected from session.");
    return;
  }
  if (!error){
    // Append incoming data from read buffer (data_) to total received data
    total_received_data_ += std::string(data_, bytes);
    // Treat excessively large requests as malicious
    if (total_received_data_.length() >= max_length * 4){
      create_response(error, 413); // 413 Payload Too Large
      return;
    }

    Request req = parse_req(total_received_data_);
    
    /* Check if the parsed request is complete. If so, process it. 
       Because only max_length bytes can be read at a time, it is possible to
       receive an incomplete request from one read. */
    if (req.has_content_length()){
      /* If the Content-Length header is present, we know the complete request
         has been read when payload size matches Content-Length. */

      /* Throws std::invalid_argument if non-numeric, but has_content_length()
         ensures it is numeric, no further exception handling required. */
      int content_length = std::stoi(req.at(http::field::content_length));
      int payload_size = 0;
      
      // Treat excessively large requests as malicious
      if (content_length >= max_length * 4){
        create_response(error, 413); // 413 Payload Too Large
        return;
      }

      boost::optional<uint64_t> payload_size_opt = req.payload_size();
      if (payload_size_opt) // boost::optional has value
        payload_size = *payload_size_opt; // Extract and set value

      if (payload_size < content_length) // Request is incomplete
        do_read(); // Continue reading incoming data
      else // Request is complete
        create_response(error, req); // Create response
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
   Create an error response to a given status code. */
void session::create_response(const error_code& error, int status){
  size_t req_bytes = total_received_data_.length();

  Response* res = new Response();
  res->result(status); // Set response status code to specified error status
  res->version(11);

  total_received_data_ = ""; // Clear total received data
  do_write(error, res, req_bytes, "(Invalid)", ""); // Write response
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

  Response* res = nullptr; // To be defined by the request handler

  int req_error = verify_req(req); // Returns 0 if request valid, else err code
  if (req_error){ // Invalid request, generate an error response
    res = new Response();
    res->result(req_error); // Set response status code to verify_req() output
    res->version(11);
    req_summary = "(Invalid)"; // Log invalid request
    invalid_req = proc_invalid_req(total_received_data_);
  }
  else{ // Valid request, dispatch a request handler to obtain response
    RequestHandler* handler = dispatch(req);
    res = handler->handle_request(req);
    free(handler); // Free memory used by request handler
  }

  total_received_data_ = ""; // Clear total received data
  do_write(error, res, req_bytes, req_summary, invalid_req); // Write response
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
        if (res->result_int() == 413){ // 413 Payload Too Large
          free(res); // Free memory used by HTTP response object
          close_session(1, "Client attempted to send an excessive payload, shutting down.");
        }
        else if (res->keep_alive()){ // Connection: keep-alive was requested
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


/// Dynamically dispatches a RequestHandler based on the given request.
RequestHandler* dispatch(Request& req){
  if (req.method() == http::verb::get){ [[likely]]
    if (req.target() == "/health")
      return Registry::inst().get_factory("HealthRequestHandler")->create();
    else
      return Registry::inst().get_factory("FileRequestHandler")->create();
  } // Only GET and POST requests are supported
  return Registry::inst().get_factory("PostRequestHandler")->create();
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
  if (req.target().find("..") != std::string::npos) [[unlikely]]
    return 403; // 403 Forbidden
  /* These HTML encodings resolve to single '.'s, but it is difficult to
     imagine a legitmate use case for these. Assume the request is malicious
     if one is present so we don't have to check all possible permutations. */
  if (req.target().find("%2e") != std::string::npos) [[unlikely]]
    return 403; // 403 Forbidden
  if (req.target().find("%%32%65") != std::string::npos) [[unlikely]]
    return 403; // 403 Forbidden

  // Verify HTTP version: HTTP/0.9, HTTP/1.0, HTTP/1.1, HTTP/2.0, or HTTP/3.0
  switch(req.version()){
    case 11: [[likely]]
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
    if (!req.has_content_length()) [[unlikely]]
      return 411; // 411 Length Required
  }
  return 0;
}


/// Helper function for invalid request logging.
/// Converts CRLF characters in received data to keep the log to a single line.
std::string proc_invalid_req(const std::string& received){
  std::string out = "";
  for (int i = 0; i < received.length(); i++){
    if (received[i] == '\r')
      out += "\\r";
    else if (received[i] == '\n')
      out += "\\n";
    else [[likely]]
      out += received[i];
  }
  return out;
}