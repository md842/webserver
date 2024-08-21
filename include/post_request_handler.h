#pragma once

#include "request_handler_interface.h"

class PostRequestHandler : public RequestHandler{
public:
  Response* handle_request(const Request& req) override;
};

class PostRequestHandlerFactory : public RequestHandlerFactory{
public:
  virtual RequestHandler* create() override;
};