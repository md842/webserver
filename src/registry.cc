#include "registry.h"


/// Returns the corresponding factory pointer for a given RequestHandler name.
RequestHandlerFactory* Registry::get_factory(const std::string& name){
  return registry[name];
}


/// Returns a static reference to the singleton instance of Registry.
Registry& Registry::inst(){
  static Registry instRef;
  return instRef;
}


/// Registers a new handler type and its corresponding factory.
void Registry::register_handler(const std::string& name,
                                RequestHandlerFactory* factory){
  registry[name] = factory;
}