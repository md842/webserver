#include "analytics.h"
#include "health_request_handler.h"
#include "registry.h" // Registry::inst(), REGISTER_HANDLER macro

namespace http = boost::beast::http;


/// Generates a response to a given GET request.
Response* HealthRequestHandler::handle_request(const Request& req){
  Analytics::inst().health++; // Log health check in analytics

  // Construct and return pointer to HTTP response object
  Response* res = new Response();
  res->result(http::status::ok);
  res->version(11);
  res->set(http::field::connection, "close");
  res->set(http::field::content_type, "text/html"); 
  res->body() = Analytics::inst().report();
  res->prepare_payload();

  return res;
}


/// Returns a pointer to a new file request handler.
RequestHandler* HealthRequestHandlerFactory::create(){
  return new HealthRequestHandler;
}


/// Register HealthRequestHandler and corresponding factory. Runs before main().
REGISTER_HANDLER("HealthRequestHandler", HealthRequestHandlerFactory)
