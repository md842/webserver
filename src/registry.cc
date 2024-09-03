#include "log.h"
#include "registry.h"


/// Returns the corresponding factory for a given RequestHandler type.
RequestHandlerFactory* Registry::get_factory(const std::string& name){
  // Returns the corresponding factory for a given request handler type.
  return registry[name].factory_ptr;
}


/// Returns the corresponding URI map for a given RequestHandler type.
std::map<std::string, std::vector<std::string>> Registry::get_map(const std::string& name){
  return registry[name].uri_map;
}


/// Returns a list of names of registered RequestHandler types.
std::vector<std::string> Registry::get_types(){
  std::vector<std::string> out;
  for (const std::pair<std::string, RegEntry>& entry : registry)
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
  RegEntry entry;
  entry.factory_ptr = factory;
  // Initialize an empty URI map for the new handler type.
  entry.uri_map = std::map<std::string, std::vector<std::string>>();
  registry[name] = entry;
}


/// Registers a new URI mapping for the given handler type and values.
void Registry::register_mapping(const std::string& name,
                                const std::string& uri,
                                const std::string& rel_path){
  registry[name].uri_map[uri].push_back(rel_path);
}