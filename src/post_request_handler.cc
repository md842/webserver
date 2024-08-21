#include "post_request_handler.h"
#include "log.h"
#include "registry.h" // Registry::inst(), REGISTER_HANDLER macro

namespace http = boost::beast::http;

Response* PostRequestHandler::handle_request(const Request& req){
  // Returns a pointer to an HTTP response object for the given HTTP request.
  http::status status = http::status::ok; // Response status code 200

  // Temporary hardcoded value for testing purposes
  std::string body = "{\"output\":\"Placeholder response from server\"}";
  // TODO: Get body from running requested background task

  // Construct and return pointer to HTTP response object
  Response* res = new Response();
  res->result(status);
  res->version(11);

  // Set headers
  res->set(http::field::cache_control, "public, max-age=604800, immutable");
  if (req.keep_alive()) // Use same keep-alive option as incoming request
    res->set(http::field::connection, "keep-alive");
  else
    res->set(http::field::connection, "close");
  res->set(http::field::content_type, "application/json"); 

  // Set body
  res->body() = body;
  res->prepare_payload();
  
  return res;
}

RequestHandler* PostRequestHandlerFactory::create(){
  // Returns a pointer to a new file request handler for the given path.
  return new PostRequestHandler;
}

// Register FileRequestHandler and corresponding factory. Runs before main().
REGISTER_HANDLER("PostRequestHandler", PostRequestHandlerFactory)
