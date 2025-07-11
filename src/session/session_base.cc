#include <boost/algorithm/string/replace.hpp> // replace_all
#include <boost/asio.hpp> // buffer
#include <boost/asio/ssl.hpp> // ssl::error

#include "analytics.h"
#include "log.h"
#include "registry.h" // Registry::inst()
#include "request_handler_interface.h" // RequestHandler
#include "session/session_base.h"

// Standardized log prefix for this source
#define LOG_PRE "[Session]  "

using namespace boost::asio;
using boost::system::error_code;


Request parse_req(const std::string& received);
int verify_req(Request& req);
RequestHandler* dispatch(Request& req, Config* config_);
std::string proc_invalid_req(const std::string& received); // Helper function for invalid request logging


/// Parses incoming data from do_read() into HTTP request and creates response.
void session_base::handle_read(const error_code& error, size_t bytes){
  try{ // Throws boost::system::system_error
    client_ip_ = socket().remote_endpoint().address().to_string();
  }
  catch(boost::system::system_error){ // Thrown by socket::remote_endpoint()
    close(0, "Client disconnected from session.");
    return; // close() deletes this session, return to avoid segfault
  }

  if (!error){ // do_read successfully read data
    // Append incoming data from read buffer (data_) to total received data
    total_received_data_ += std::string(data_, bytes);

    // Treat excessively large requests as malicious
    if (total_received_data_.length() >= max_length * 4){
      create_response(413); // 413 Content Too Large
      return; // Do not continue processing the request
    }

    Request req = parse_req(total_received_data_);

    /* Config defines return directive, ignore all other processing and create
       appropriate response. Validation offloaded to destination server. */
    if (config_->ret){
      create_return_response(req);
      return; // Do not continue processing the request
    }

    /* Check parsed request for completeness. Because only max_length bytes may
       be read at a time, it is possible to read an incomplete request. */
    if (req.has_content_length()){
      /* If the Content-Length header is present, we know the complete
         request has been read when payload size matches Content-Length. */

      /* Throws std::invalid_argument if non-numeric, but has_content_length()
         ensures it is numeric, so no further handling required. */
      int content_length = std::stoi(req.at(http::field::content_length));
      
      // Treat excessively large requests as malicious
      if (content_length >= max_length * 4){
        create_response(413); // 413 Content Too Large
        return; // Do not continue processing the request
      }

      // Compare payload size to value in Content-Length header
      boost::optional<uint64_t> payload_size_opt = req.payload_size();
      if (payload_size_opt && *payload_size_opt < content_length)
        do_read(); // Request is incomplete, continue reading incoming data
      else
        create_response(req); // Request is complete, create response
    }
    else{
      /* If the Content-Length header is not present, there should be no body,
         so we know the complete request has been read when bytes < max_length
         or the request ends with \r\n\r\n. */
      if (bytes < max_length)
        create_response(req); // Request is complete, create response
      else if (total_received_data_.substr(
        total_received_data_.length() - 4) == "\r\n\r\n")
        /* This condition would suffice for all requests with no body. However,
           there is a very problematic edge case (complete request length
           perfectly divisible by max_length), so the above comparison is
           preferred and this is left as a fallback. */
        create_response(req); // Request is complete, create response
      else
        do_read(); // Request is incomplete, continue reading incoming data
    }
  }
  else if (error == error::eof)
    /* Client sends EOF when closed or keep-alive times out.
       Expected behavior, log as info rather than error and shut down. */
    close(0, "Keep-alive connection closed by client, shutting down.");
  else if (error == ssl::error::stream_truncated)
    /* Many HTTP clients do not exchange SSL shutdown notifications correctly.
       Log as info rather than error and shut down. */
    close(0, "SSL stream truncated by client, shutting down.");
  else // Unknown read error, log as error and shut down.
    close(2, "Got error \"" + error.message() + "\" while reading request, shutting down.");
}


/* Overload 1 of 2:
   Create an error response to a given status code. */
void session_base::create_response(int status){
  Response* res = new Response();
  res->result(status); // Set response status code to specified error status
  res->version(11);

  std::string summary, invalid;
  switch(status){
    case 413: // Content Too Large
      Analytics::inst().malicious++;
      summary = "(Content Too Large)";
      break; // Don't log invalid request body to avoid flooding log
    case 403: // Forbidden
      Analytics::inst().malicious++;
      summary = "(Forbidden)";
      invalid = proc_invalid_req(total_received_data_);
      break;
    default:
      Analytics::inst().invalid++;
      summary = "(Invalid)";
      invalid = proc_invalid_req(total_received_data_);
  }

  // Initializer list for request info struct for logging
  Log::req_info req_info = {total_received_data_.length(), summary, invalid};

  do_write(res, req_info); // Continue to write response
}


/* Overload 2 of 2:
   Create a response to a request that parsed successfully (may not be valid!)
   For a valid request, dispatches a RequestHandler to create the response.
   For an invalid request, create an error response and log the request. */
void session_base::create_response(Request& req){
  int req_error = verify_req(req); // Returns 0 if request valid, else err code

  if (req_error) // Invalid request
    create_response(req_error); // Create error response for given status code
  else{ // Valid request, dispatch a request handler to obtain response
    RequestHandler* handler = dispatch(req, config_);
    Response* res = handler->handle_request(req);
    delete handler; // Free memory used by request handler

    std::string summary = req.method_string(); // Must convert string_view to
    summary += " " + std::string(req.target()); // string before adding target

    // Initializer list for request info struct for logging
    Log::req_info req_info = {total_received_data_.length(), summary, ""};

    do_write(res, req_info); // Continue to write response
  }
  /* Invalid request invokes create_response(int) which calls do_write(...),
     so we simply allow it to fall through and end this branch here. */
}


/// Create an appropriate response based on a return directive.
void session_base::create_return_response(Request& req){
  /* Redirect server doesn't care about validating the request, the request
     will be verified by destination server if applicable. */

  Response* res = new Response();
  res->result(config_->ret); // Set response status code to return status
  res->version(11);
  
  // Set up response headers and body
  if (config_->ret / 100 == 3){ // 301, 302, 303, 307, 308 (redirect)
    std::string resolved_ret_uri = config_->ret_val;
    // Resolve ret_uri $scheme, $host, $request_uri in config value
    if (config_->type == Config::ServerType::HTTP_SERVER)
      boost::replace_all(resolved_ret_uri, "$scheme", "http");
    else
      boost::replace_all(resolved_ret_uri, "$scheme", "https");
    boost::replace_all(resolved_ret_uri, "$host", config_->host);
    boost::replace_all(resolved_ret_uri, "$request_uri", req.target());
    // Set Location header with resolved URI for the redirect
    res->set(http::field::location, resolved_ret_uri);
    // Browser won't redirect correctly without a response body
    res->body() = "Redirecting to " + resolved_ret_uri;
  }
  else // Not a redirect, ret_val is (optional) response body text
    res->body() = config_->ret_val;
  res->prepare_payload();

  std::string summary = req.method_string(); // Must convert string_view to
  summary += " " + std::string(req.target()); // string before adding target

  // Initializer list for request info struct for logging
  Log::req_info req_info = {total_received_data_.length(), summary, ""};

  do_write(res, req_info); // Continue to write response
}


/// Write handler, decides what to do next after writing response to client.
void session_base::handle_write(const error_code& error, size_t res_bytes,
                                Response* res, Log::req_info& req_info){
  int result_int = res->result_int();  // Extract necessary info from HTTP
  bool keep_alive = res->keep_alive(); // response object before freeing
  delete res; // Free memory used by HTTP response object

  if (!error){ // Successful write
    if (keep_alive) // Connection: keep-alive was requested
      do_read(); // Continue listening for requests (bypasses SSL handshake)
    else if (result_int == 413) // 413 Payload Too Large
      close(1, "Client attempted to send an excessive payload, shutting down.");
    else // Connection: close was requested
      close(0, "Connection: close specified, shutting down.");
    // Write machine-parseable formatted log
    Log::res_metrics(client_ip_, req_info, res_bytes, result_int);
  }
  else // Error during write
    close(2, "Got error \"" + error.message() + "\" while writing response, shutting down.");
}


/// Logs information about a closing session, then closes it.
void session_base::close(int severity, const std::string& message){
  std::string full_msg = "Client: " + client_ip_ + " | " + message;
  switch(severity){
    case 0: // info
      Log::info(LOG_PRE, full_msg);
      break;
    case 1: // warning
      Log::warn(LOG_PRE, full_msg);
      break;
    case 2: // error
      Log::error(LOG_PRE, full_msg);
  }
  do_close(); // Close the session
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
  switch(method){ // Verify HTTP method: Only GET, POST are allowed
    case http::verb::get: [[likely]] // Fall through
    case http::verb::post:
      break;
    case http::verb::unknown:
      return 400; // 400 Bad Request
    default: // DELETE, HEAD, PUT, CONNECT, OPTIONS, TRACE
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
    case 11: [[likely]] // Fall through
    case 20:
    case 30:
    case 10:
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


/// Dynamically dispatches a RequestHandler based on the given request.
RequestHandler* dispatch(Request& req, Config* config_){
  std::string factory;

  if (req.method() == http::verb::get){ [[likely]]
    if (req.target() == "/health")
      factory = "HealthRequestHandler";
    else{ [[likely]]
      Analytics::inst().gets++; // Log valid GET request in analytics
      factory = "FileRequestHandler";
    }
  } // Only GET and POST requests are supported
  else
    factory = "PostRequestHandler";

  RequestHandler* handler = Registry::inst().get_factory(factory)->create();
  handler->init_config(config_);
  return handler;
}


/// Helper function for invalid request logging.
/// Converts CRLF characters in received data to keep the log to a single line.
std::string proc_invalid_req(const std::string& received){
  std::string out = received;
  boost::replace_all(out, "\r\n", " | ");
  return out;
}