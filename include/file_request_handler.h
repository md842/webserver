#pragma once

#include "request_handler_interface.h"

class FileRequestHandler : public RequestHandler{
public:
  Response* handle_request(const Request& req) override;
};

class FileRequestHandlerFactory : public RequestHandlerFactory{
public:
  virtual RequestHandler* create() override;
};