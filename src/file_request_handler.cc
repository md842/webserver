#include <boost/filesystem.hpp> // exists, is_directory, path
#include <boost/filesystem/fstream.hpp> // ifstream
#include <iomanip> // put_time

#include "file_request_handler.h"
#include "log.h"
#include "nginx_config_parser.h" // Config::inst()
#include "registry.h" // Registry::inst(), REGISTER_HANDLER macro

namespace fs = boost::filesystem;
namespace http = boost::beast::http;


std::string last_modified_time(fs::path* file_obj);
std::string mime_type(fs::path* file_obj);
std::string resolve_path(const std::string& req_target);


/// Generates a response to a given GET request.
Response* FileRequestHandler::handle_request(const Request& req){
  std::string target = resolve_path(std::string(req.target()));
  std::string full_path = Config::inst().root() + target; // Add root to target

  http::status status = http::status::ok; // Response status code 200
  std::ostringstream file_contents;
  std::string content_type = "text/html"; // Overwritten if valid file opened
  std::string last_modified = "";

  fs::path* file_obj = new fs::path(full_path);
  if (!exists(*file_obj) || is_directory(*file_obj)){ // Nonexistent or dir
    status = http::status::not_found; // Response status code 404
    delete file_obj; // Free old file_obj before replacing
    file_obj = new fs::path(Config::inst().root() + Config::inst().index());
  }

  fs::ifstream fstream(*file_obj); // Attempt to open the file
  if (fstream){ // Successfully opened the file
    last_modified = last_modified_time(file_obj);
    try{ // If validation request, compare last_modified to cached time
      std::string cached_time = std::string(
        req.at(http::field::if_modified_since));
      if (last_modified == cached_time) // Else last_modified is newer
        status = http::status::not_modified; // Response status code 304
    }
    // req.at() throws if not validation request. Safe to catch and ignore.
    catch(std::out_of_range){}

    content_type = mime_type(file_obj);

    if (status != http::status::not_modified)
      file_contents << fstream.rdbuf(); // Read file into string stream
  }
  else{ // File exists, but failed to open it for some reason.
    // Since the server does not support client-side writes, this is unlikely
    // to occur with inadequate resources being the only failure condition.
    Log::error("FileRequestHandler: Could not open file: ./" + target);
    status = http::status::internal_server_error; // Response status code 500
    file_contents << "<h1>Internal Server Error (Error 500).</h1>\n";
  }

  // Construct and return pointer to HTTP response object
  Response* res = new Response();
  res->result(status);
  res->version(11);

  // Set Keep-Alive header for all response types.
  if (req.keep_alive()) // Use same keep-alive option as incoming request
    res->set(http::field::connection, "keep-alive");
  else
    res->set(http::field::connection, "close");

  switch(status){
    case http::status::ok: // 200 OK
    case http::status::not_found: // 404 Not Found
      // Set Cache-Control, Content-Type, Last-Modified, and body
      res->set(http::field::cache_control, "public, max-age=604800, immutable");
      res->set(http::field::last_modified, last_modified);
    case http::status::internal_server_error: // 500 Internal Server Error
      // Set Content-Type and body (200, 404 responses fall through to here)
      res->set(http::field::content_type, content_type); 
      res->body() = file_contents.str();
      res->prepare_payload();
      break;
    case http::status::not_modified: // 304 Not Modified
      // Set Cache-Control only
      res->set(http::field::cache_control, "public, max-age=604800, immutable");
  }
  
  delete file_obj; // Free file_obj pointer after use
  return res;
}


/** 
 * Returns the last modified time for the given file. Used for HTTP caching.
 *
 * @param file_obj A pointer to a path object for the target file.
 * @returns The last modified time for the file in HTTP time string format.
 * @relatesalso FileRequestHandler
 */
std::string last_modified_time(fs::path* file_obj){
  std::time_t last_write_time = fs::last_write_time(*file_obj);
  tm* gm = std::gmtime(&last_write_time);

  std::stringstream ss; // std::put_time must be used with a stream
  // Conversion specifiers: https://en.cppreference.com/w/cpp/io/manip/put_time
  // HTTP Spec: <day-name>, <day> <month> <year> <hour>:<minute>:<second> GMT
  ss << std::put_time(gm, "%a, %d %b %Y %T %Z");
  return ss.str();
}


/** 
 * Returns the MIME type of the given file. Used to set Content-Type header.
 *
 * @param file_obj A pointer to a path object for the target file.
 * @returns The MIME type of the file as a string.
 * @relatesalso FileRequestHandler
 */
std::string mime_type(fs::path* file_obj){
  std::string extension = file_obj->extension().string();
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


/** 
 * Resolves the target URI of the request to a relative path on the web server.
 *
 * @param req_target The target URI of the incoming request.
 * @returns The relative path on the web server as a string.
 * @relatesalso FileRequestHandler
 */
std::string resolve_path(const std::string& req_target){
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


/// Returns a pointer to a new file request handler.
RequestHandler* FileRequestHandlerFactory::create(){
  return new FileRequestHandler;
}


/// Register FileRequestHandler and corresponding factory. Runs before main().
REGISTER_HANDLER("FileRequestHandler", FileRequestHandlerFactory)
