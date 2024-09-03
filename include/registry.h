#pragma once

#include <map>

#include "request_handler_interface.h"

/// A data structure used by Registry that wraps a factory and URI map.
struct RegEntry{
  RequestHandlerFactory* factory_ptr;
  std::map<std::string, std::string> uri_map;
};

/// A macro for registering a RequestHandler type before main() runs.
#define REGISTER_HANDLER(name, factory)\
  static struct Register##factory{\
    Register##factory(){\
      Registry::inst().register_handler(name, new factory);\
    }\
  }register##factory;

class Registry final{ // Singleton class (only one instance)
public:
  // Deleting the copy and assignment operators due to being a singleton class
  Registry(const Registry&) = delete;
  Registry& operator=(const Registry&) = delete;

  /** 
   * Returns the corresponding factory for a given RequestHandler type.
   *
   * @param name The name of a RequestHandler type.
   * @returns A pointer to the appropriate RequestHandlerFactory object.
   */
  RequestHandlerFactory* get_factory(const std::string& name);

  /** 
   * Returns the corresponding URI map for a given RequestHandler type.
   *
   * @param name The name of a RequestHandler type.
   * @returns A map containing URI to relative path pairs.
   */
  std::map<std::string, std::string> get_map(const std::string& name);

  /// Returns a list of names of registered RequestHandler types.
  std::vector<std::string> get_types();

  /// Returns a static reference to the singleton instance of Registry.
  static Registry& inst();

  /** 
   * Registers a new handler type and its corresponding factory.
   *
   * @param name The name of the new RequestHandler type.
   * @param factory A pointer to the factory for the new RequestHandler type.
   */
  void register_handler(const std::string& name, RequestHandlerFactory* factory);

  /** 
   * Registers a new URI mapping for the given handler type and values.
   *
   * @param name The name of the RequestHandler type to add the mapping to.
   * @param uri The URI associated with the new mapping.
   * @param value The relative path to map to uri.
   */
  void register_mapping(const std::string& name, const std::string& uri,
                        const std::string& value);

private:
  Registry(){}; // Making constructor private due to being a singleton class
  // Maps handler type name to corresponding registry entry
  std::map<std::string, RegEntry> registry = std::map<std::string, RegEntry>();
};