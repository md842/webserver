#include "log.h"
#include "request_handler_registry.h"

RequestHandlerFactory* RequestHandlerRegistry::get_handler(const std::string& name){
  // Returns the corresponding factory for a given request handler type.
  return registry[name];
}

RequestHandlerRegistry& RequestHandlerRegistry::inst(){
  // Returns a reference to the static instance of the registry.
  static RequestHandlerRegistry instRef;
  return instRef;
}

void RequestHandlerRegistry::register_handler(const std::string& name,
                                              RequestHandlerFactory* factory){
  // Registers a new handler type and corresponding factory.
  registry[name] = factory;
  Log::trace("Registry: " + name + " successfully registered");
}