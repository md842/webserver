#pragma once

#include "request_handler_interface.h" // RequestHandler, RequestHandlerFactory

class HealthRequestHandler : public RequestHandler{
public:
  /** 
   * Generates a response to a given GET request.
   *
   * @param req A parsed HTTP request.
   * @returns A pointer to a parsed HTTP response.
   */
  Response* handle_request(const Request& req) override;
};

class HealthRequestHandlerFactory : public RequestHandlerFactory{
public:
  /// Returns a pointer to a new file request handler.
  virtual RequestHandler* create() override;
};