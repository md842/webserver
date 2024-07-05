#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "file_request_handler.h"
#include "log.h"
#include "registry.h" // Required for REGISTER_HANDLER macro

namespace fs = boost::filesystem;
namespace http = boost::beast::http;

FileRequestHandler::FileRequestHandler(const std::string& root_path)
  : RequestHandler(root_path){}

Response* FileRequestHandler::handle_request(const Request& req){
  // Assumes dispatcher has performed relative path substitution on request
  // target; saves work because dispatcher must interpret request target anyway
  std::string full_path = root_path_ + std::string(req.target());
  
  // Returns a pointer to an HTTP response object for the given HTTP request.
  http::status status = http::status::ok; // Default response status is 200 OK
  std::ostringstream file_contents;
  // Default to text/html for 404 page, overwritten if valid file is opened
  std::string content_type = "text/html";

  fs::path file_obj(full_path);
  if (!exists(file_obj) || is_directory(file_obj)){ // Nonexistent or directory
    Log::error("File not found: " + full_path);
    status = http::status::not_found; // Set response status to 404 Not Found
    fs::path file_obj(root_path_ + "/files_to_serve/404.html"); // Open 404 page
    fs::ifstream fstream(file_obj);
    Log::info("Writing 404 page (" + root_path_ + "/files_to_serve/404.html)");
    file_contents << fstream.rdbuf(); // Read 404 page into string stream
  }
  else{ // Non-directory file found
    fs::ifstream fstream(file_obj); // Attempt to open the file
    if (!fstream){ // File exists, but failed to open it for some reason.
      // Since the server does not support client-side writes, this is unlikely
      // to occur with inadequate resources being the only failure condition.
      Log::error("Could not open file: " + full_path);
      // Set response status to 500 Internal Server Error
      status = http::status::internal_server_error;
      content_type = "text/plain";
      file_contents << "500 Internal Server Error";
    }
    else{ // File opened successfully
      Log::info("Writing " + full_path);
      // Temporary hardcoded value for testing purposes
      content_type = "text/html";
      // TODO: Implement MIME type parsing
      file_contents << fstream.rdbuf(); // Read file into string stream
    }
  }

  // Construct and return pointer to HTTP response object
  Response* response = new Response();
  response->result(status);
  response->version(11);
  response->set(http::field::content_type, content_type);
  response->body() = file_contents.str();
  //if (req.keep_alive()) // If keep-alive requested, agree and set keep-alive
  //  response->set("Connection", "keep-alive");
  response->prepare_payload();
  return response;
}

RequestHandler* FileRequestHandlerFactory::create(const std::string& path){
  // Returns a pointer to a new file request handler for the given path.
  return new FileRequestHandler(path);
}

// Register FileRequestHandler and corresponding factory. Runs before main().
REGISTER_HANDLER("FileRequestHandler", FileRequestHandlerFactory)
