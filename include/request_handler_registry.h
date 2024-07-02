#pragma once

#include <map>

#include "request_handler_interface.h"

// A macro for registering a handler before main() runs.
#define REGISTER_HANDLER(name, factory)\
  static struct Register##factory{\
    Register##factory(){\
      RequestHandlerRegistry::inst().register_handler(name, new factory);\
    }\
  }register##factory;

class RequestHandlerRegistry final{ // Singleton class (only one instance)
public:
  // Singleton should delete copy and assignment operators
  RequestHandlerRegistry(const RequestHandlerRegistry&) = delete;
  RequestHandlerRegistry& operator=(const RequestHandlerRegistry&) = delete;

  RequestHandlerFactory* get_handler(const std::string& name);
  static RequestHandlerRegistry& inst();
  void register_handler(const std::string& name, RequestHandlerFactory* factory);

private:
  RequestHandlerRegistry(){}; // Singleton should have private constructor
  std::map<std::string, RequestHandlerFactory*> registry =
    std::map<std::string, RequestHandlerFactory*>(); // map<URI, factory>
};