#pragma once

#include <map>

#include "request_handler_interface.h" // RequestHandlerFactory


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

  /// Returns a static reference to the singleton instance of Registry.
  static Registry& inst();

  /** 
   * Registers a new handler type and its corresponding factory.
   *
   * @param name The name of the new RequestHandler type.
   * @param factory A pointer to the factory for the new RequestHandler type.
   */
  void register_handler(const std::string& name, RequestHandlerFactory* factory);

private:
  Registry(){}; // Making constructor private due to being a singleton class
  // Maps handler type name to corresponding registry entry
  std::map<std::string, RequestHandlerFactory*> registry = std::map<std::string, RequestHandlerFactory*>();
};