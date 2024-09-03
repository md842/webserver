#pragma once

#include "request_handler_interface.h" // RequestHandler, RequestHandlerFactory

class PostRequestHandler : public RequestHandler{
public:
  /** 
   * Generates a response to a given POST request.
   *
   * @param req A parsed HTTP request.
   * @returns A pointer to a parsed HTTP response.
   */
  Response* handle_request(const Request& req) override;
};

class PostRequestHandlerFactory : public RequestHandlerFactory{
public:
  /// Returns a pointer to a new POST request handler.
  virtual RequestHandler* create() override;
};