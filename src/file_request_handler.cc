#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iomanip>

#include "file_request_handler.h"
#include "log.h"
#include "nginx_config_parser.h" // Config::inst()
#include "registry.h" // Registry::inst(), REGISTER_HANDLER macro

namespace fs = boost::filesystem;
namespace http = boost::beast::http;

std::string last_modified_time(fs::path file_obj);
std::string mime_type(fs::path file_obj);
std::string resolve_path(const std::string& req_target);

Response* FileRequestHandler::handle_request(const Request& req){
  // Returns a pointer to an HTTP response object for the given HTTP request.
  std::string target = resolve_path(std::string(req.target()));
  std::string full_path = Config::inst().root() + target; // Add root to target

  http::status status = http::status::ok; // Default response status is 200 OK
  std::ostringstream file_contents;
  // Default to text/html for 404 page, overwritten if valid file is opened
  std::string content_type = "text/html";
  std::string last_modified = "";

  fs::path file_obj(full_path);
  if (!exists(file_obj) || is_directory(file_obj)){ // Nonexistent or directory
    status = http::status::not_found; // Set response status to 404 Not Found
    fs::path file_obj(Config::inst().root() + Config::inst().index());
    fs::ifstream fstream(file_obj); // Open index page
    last_modified = last_modified_time(file_obj);

    Log::info("FileRequestHandler: ." + target + " not found. Serving index page.");
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
      last_modified = last_modified_time(file_obj);
      try{
        std::string cached_time = std::string(
          req.at(http::field::if_modified_since));
        if (last_modified == cached_time){
          Log::info("FileRequestHandler: Writing 304 Not Modified");
          status = http::status::not_modified; // 304 Not Modified
        } // If no match, last_modified is newer, continue
      }
      catch(std::out_of_range){} // req.at throws if not validation request

      content_type = mime_type(file_obj);

      if (status != http::status::not_modified){
        Log::info("FileRequestHandler: Writing ./" + target);
        file_contents << fstream.rdbuf(); // Read file into string stream
      }
    }
  }

  // Construct and return pointer to HTTP response object
  Response* res = new Response();
  res->result(status);
  res->version(11);

  // Set headers
  res->set(http::field::cache_control, "public, max-age=604800, immutable");
  if (req.keep_alive()) // Use same keep-alive option as incoming request
    res->set(http::field::connection, "keep-alive");
  else
    res->set(http::field::connection, "close");
  res->set(http::field::content_type, content_type); 
  res->set(http::field::last_modified, last_modified);

  // Set body (ignore if 304 Not Modified)
  if (status != http::status::not_modified){
    res->body() = file_contents.str();
    res->prepare_payload();
  }
  
  return res;
}

std::string last_modified_time(fs::path file_obj){
  std::time_t last_write_time = fs::last_write_time(file_obj);
  tm* gm = std::gmtime(&last_write_time);

  std::stringstream ss; // std::put_time must be used with a stream
  // Conversion specifiers: https://en.cppreference.com/w/cpp/io/manip/put_time
  // HTTP Spec: <day-name>, <day> <month> <year> <hour>:<minute>:<second> GMT
  ss << std::put_time(gm, "%a, %d %b %Y %T %Z");
  return ss.str();
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

std::string resolve_path(const std::string& req_target){
  // Performs relative path substitution on req_target using config mapping.
  std::string target = req_target;

  // Configure server to serve index.html for paths handled by React Router
  // https://create-react-app.dev/docs/deployment#serving-apps-with-client-side-routing
  std::vector<std::string> react_router_pages = {"/", "/projects", "/resume"};
  if (std::any_of(react_router_pages.begin(), react_router_pages.end(),
    [target](std::string page){
      return (target == page);
    })){
    target = Config::inst().index(); // Serve index page
  }
  else{ // Request is for a path not handled by React Router
    int longest_match = 0;
    std::string mapping = "";
    for (auto& pair : Registry::inst().get_map("FileRequestHandler")){
      std::string key = pair.first;
      // New longest match found, save information
      if ((target.find(key) == 0) && key.length() > longest_match){
        longest_match = key.length();
        mapping = pair.second;
      }
    }
    target.replace(0, longest_match, mapping); // Substitute path
  }
  return target;
}

RequestHandler* FileRequestHandlerFactory::create(){
  // Returns a pointer to a new file request handler for the given path.
  return new FileRequestHandler;
}

// Register FileRequestHandler and corresponding factory. Runs before main().
REGISTER_HANDLER("FileRequestHandler", FileRequestHandlerFactory)
