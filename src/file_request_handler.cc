#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "file_request_handler.h"
#include "log.h"
#include "request_handler_registry.h" // Required for REGISTER_HANDLER macro

namespace fs = boost::filesystem;
namespace http = boost::beast::http;

FileRequestHandler::FileRequestHandler(const std::string& path)
  : RequestHandler(path){}

Response* FileRequestHandler::handle_request(const Request& req){
  // Returns a pointer to an HTTP response object for the given HTTP request.
  fs::path file_obj(path_);
  if (!exists(file_obj) || is_directory(file_obj)){ // Nonexistent or directory
    Log::error("File not found: " + path_);
    // TODO: Handle 404 Not Found
  }

  fs::ifstream fstream(file_obj);
  if (!fstream){ // File exists, but failed to open it for some reason
    Log::error("Could not open file: " + path_);
    // TODO: Handle 404 Not Found
  }

  Log::info("Writing " + file_obj.filename().string());
  std::ostringstream file_contents;
  file_contents << fstream.rdbuf(); // Read file into string stream

  // Temporary hardcoded value for testing purposes
  std::string content_type = "text/html";
  // TODO: Implement MIME type parsing

  // Construct and return pointer to HTTP response object
  Response* response = new Response();
  response->result(http::status::ok);
  response->version(11);
  response->set(http::field::content_type, content_type);
  response->body() = file_contents.str();
  response->prepare_payload();
  return response;
}

RequestHandler* FileRequestHandlerFactory::create(const std::string& path){
  // Returns a pointer to a new file request handler for the given path.
  return new FileRequestHandler(path);
}

// Register FileRequestHandler and corresponding factory. Runs before main().
REGISTER_HANDLER("FileRequestHandler", FileRequestHandlerFactory)
