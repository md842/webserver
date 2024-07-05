#pragma once

#include <map>

#include "request_handler_interface.h"

struct RegEntry{
  RequestHandlerFactory* factory_ptr;
  std::map<std::string, std::string> uri_map;
};

// A macro for registering a handler before main() runs.
#define REGISTER_HANDLER(name, factory)\
  static struct Register##factory{\
    Register##factory(){\
      Registry::inst().register_handler(name, new factory);\
    }\
  }register##factory;

class Registry final{ // Singleton class (only one instance)
public:
  // Singleton should delete copy and assignment operators
  Registry(const Registry&) = delete;
  Registry& operator=(const Registry&) = delete;

  RequestHandlerFactory* get_factory(const std::string& name);
  std::map<std::string, std::string> get_map(const std::string& name);
  std::vector<std::string> get_types();
  static Registry& inst();
  void register_handler(const std::string& name, RequestHandlerFactory* factory);
  void register_mapping(const std::string& name, const std::string& uri,
                        const std::string& value);

private:
  Registry(){}; // Singleton should have private constructor
  // Maps handler type name to corresponding registry entry
  std::map<std::string, RegEntry> registry = std::map<std::string, RegEntry>();
};