#pragma once

#include "request_handler_interface.h"

class FileRequestHandler : public RequestHandler{
public:
  FileRequestHandler(const std::string& root_path);
  Response* handle_request(const Request& req) override;
};

class FileRequestHandlerFactory : public RequestHandlerFactory{
public:
  virtual RequestHandler* create(const std::string& root_path) override;
};