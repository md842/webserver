#include "log.h"
#include "registry.h"

RequestHandlerFactory* Registry::get_factory(const std::string& name){
  // Returns the corresponding factory for a given request handler type.
  return registry[name].factory_ptr;
}

std::map<std::string, std::string> Registry::get_map(const std::string& name){
  // Returns the corresponding URI map for a given request handler type.
  return registry[name].uri_map;
}

std::vector<std::string> Registry::get_types(){
  // Returns a list of types of handlers that have been registered.
  std::vector<std::string> out;
  for (auto entry : registry)
    out.push_back(entry.first);
  return out;
}

Registry& Registry::inst(){
  // Returns a reference to the static instance of the registry.
  static Registry instRef;
  return instRef;
}

void Registry::register_handler(const std::string& name,
                                RequestHandlerFactory* factory){
  // Registers a new handler type and corresponding factory. Initializes an
  // empty URI map for the new handler type.
  RegEntry entry;
  entry.factory_ptr = factory;
  entry.uri_map = std::map<std::string, std::string>();
  registry[name] = entry;
  Log::trace("Registry: " + name + " successfully registered");
}

void Registry::register_mapping(const std::string& name,
                                const std::string& uri,
                                const std::string& mapping){
  // Registers a new URI mapping for the given handler type and values.
  registry[name].uri_map[uri] = mapping;
}