#include "log.h"
#include "request_handler_registry.h"

RequestHandlerRegistry* RequestHandlerRegistry::instPtr = NULL;

RequestHandlerFactory* RequestHandlerRegistry::get_handler(const std::string& name){
  // Returns the corresponding factory for a given request handler type.
  return registry[name];
}

RequestHandlerRegistry* RequestHandlerRegistry::inst(){
  // Returns a pointer to the static instance of the registry.
  if (instPtr == NULL) // Prevents initializing more than once.
    instPtr = new RequestHandlerRegistry();
  return instPtr;
}

void RequestHandlerRegistry::register_handler(const std::string& name,
                                              RequestHandlerFactory* factory){
  // Registers a new handler type and corresponding factory.
  registry[name] = factory;
  Log::trace("Registry: " + name + " successfully registered");
}