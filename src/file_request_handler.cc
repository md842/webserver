#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "file_request_handler.h"
#include "log.h"
#include "nginx_config_parser.h" // Config::inst()
#include "registry.h" // Required for REGISTER_HANDLER macro

namespace fs = boost::filesystem;
namespace http = boost::beast::http;

std::string mime_type(fs::path file_obj);

Response* FileRequestHandler::handle_request(const Request& req){
  // Assumes dispatcher has performed relative path substitution on request
  // target; saves work because dispatcher must interpret request target anyway
  std::string target = std::string(req.target());
  std::string full_path = Config::inst().root() + target;
  
  // Returns a pointer to an HTTP response object for the given HTTP request.
  http::status status = http::status::ok; // Default response status is 200 OK
  std::ostringstream file_contents;
  // Default to text/html for 404 page, overwritten if valid file is opened
  std::string content_type = "text/html";

  fs::path file_obj(full_path);
  if (!exists(file_obj) || is_directory(file_obj)){ // Nonexistent or directory
    status = http::status::not_found; // Set response status to 404 Not Found
    fs::path file_obj(Config::inst().root() + "/files_to_serve/404.html");
    fs::ifstream fstream(file_obj);
    Log::warn("FileRequestHandler: ." + target + " not found. Serving 404.");
    file_contents << fstream.rdbuf(); // Read 404 page into string stream
  }
  else{ // Non-directory file found
    fs::ifstream fstream(file_obj); // Attempt to open the file
    if (!fstream){ // File exists, but failed to open it for some reason.
      // Since the server does not support client-side writes, this is unlikely
      // to occur with inadequate resources being the only failure condition.
      Log::error("FileRequestHandler: Could not open file: ./" + target);
      // Set response status to 500 Internal Server Error
      status = http::status::internal_server_error;
      content_type = "text/plain";
      file_contents << "500 Internal Server Error";
    }
    else{ // File opened successfully
      Log::info("FileRequestHandler: Writing ./" + target);
      content_type = mime_type(file_obj);
      file_contents << fstream.rdbuf(); // Read file into string stream
    }
  }

  // Construct and return pointer to HTTP response object
  Response* response = new Response();
  response->result(status);
  response->version(11);
  response->set(http::field::content_type, content_type);
  response->body() = file_contents.str();
  if (req.keep_alive()) // Use same keep-alive option as incoming request
    response->set("Connection", "keep-alive");
  else
    response->set("Connection", "close");
  response->prepare_payload();
  return response;
}

std::string mime_type(fs::path file_obj){
  std::string extension = file_obj.extension().string();
  std::map<std::string, std::string> types = {
    {".html", "text/html"},
    {".htm", "text/html"},
    {".css", "text/css"},
    {".js", "text/javascript"},
    {".json", "application/json"},
    {".gif", "image/gif"},
    {".ico", "image/vnd.microsoft.icon"},
    {".jpeg", "image/jpeg"},
    {".jpg", "image/jpeg"},
    {".png", "image/png"},
    {".svg", "image/svg+xml"},
    {".webp", "image/webp"}, 
    {".txt", "text/plain"}, 
    {".xml", "application/xml"},
    {".zip", "application/zip"}
  };
  if (types.count(extension)) // 1 if key in map, 0 otherwise
    return types[extension];
  return "application/octet-stream"; // Default case
}

RequestHandler* FileRequestHandlerFactory::create(){
  // Returns a pointer to a new file request handler for the given path.
  return new FileRequestHandler;
}

// Register FileRequestHandler and corresponding factory. Runs before main().
REGISTER_HANDLER("FileRequestHandler", FileRequestHandlerFactory)
