#include "registry.h"


/// Returns the corresponding factory pointer for a given RequestHandler name.
RequestHandlerFactory* Registry::get_factory(const std::string& name){
  return registry[name];
}


/// Returns a list of names of registered RequestHandler types.
std::vector<std::string> Registry::get_types(){
  std::vector<std::string> out;
  for (const std::pair<std::string, RequestHandlerFactory*>& entry : registry)
    out.push_back(entry.first);
  return out;
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