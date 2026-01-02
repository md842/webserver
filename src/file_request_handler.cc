#include <boost/filesystem.hpp> // exists, is_directory, path
#include <boost/filesystem/fstream.hpp> // ifstream
#include <boost/lexical_cast.hpp> // lexical_cast
#include <iomanip> // put_time
// #include <regex> // regex_search

#include "file_request_handler.h"
#include "log.h"
#include "registry.h" // Registry::inst(), REGISTER_HANDLER macro

// Standardized log prefix for this source
#define LOG_PRE "[FileRequestHandler] "

namespace fs = boost::filesystem;


std::string last_modified_time(fs::path file_obj);
std::string mime_type(fs::path file_obj);
LocationBlock* get_location(const std::string& req_target, Config* config);
bool resolve_path(const std::string& target, const std::string& index,
                  fs::path& file_obj);
bool get_file_from_loc(const std::string& req_target, LocationBlock* location,
                       fs::path& file_obj, http::status& status);


/// Generates a response to a given GET request.
Response* FileRequestHandler::handle_request(const Request& req){
  http::status status = http::status::ok; // Response status code 200
  std::ostringstream file_contents;
  std::string content_type = "text/html"; // Overwritten if valid file opened
  std::string last_modified = "";
  fs::path file_obj;

  // Attempt to match req_target to a location block in the web server config
  LocationBlock* location = get_location(std::string(req.target()), config_);

  if (location != nullptr){ // Matching location block found
    // Attempt to match req_target to a file given matched location block
    if (!get_file_from_loc(std::string(req.target()), location, file_obj, status)){
      // Matching file not found. get_file_from_loc sets status, fall through to res
      // Log::trace(LOG_PRE, "get_file_from_loc returned false. Status: " + std::to_string(static_cast<int>(status)));
    }
  }
  else{ // No matching location block found
    // Attempt to resolve relative path to a file object
    // Log::trace(LOG_PRE, "No location block, trying " + config_->root + std::string(req.target()));
    if (!resolve_path(config_->root + std::string(req.target()), config_->index, file_obj))
      status = http::status::not_found; 
  }

  fs::ifstream fstream(file_obj); // Attempt to open the file
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
  else if (status == http::status::ok){
    /* File exists, but failed to open it for some reason. Since the server
       does not support client-side writes, this is unlikely to occur with
       inadequate resources being the only failure condition. */
    Log::error(LOG_PRE, "Failed to open file (possibly inadequate resources)");
    status = http::status::internal_server_error; // Response status code 500
    file_contents << "<h1>Internal Server Error (Error 500).</h1>";
  }
  /* If status is not 200 OK, it was set by get_file_from_loc; file does not
     exist and fstream returning false is not an error. Fall through to res. */

  // Construct and return pointer to HTTP response object
  Response* res = new Response();
  res->result(status);
  res->version(11);

  // Set Keep-Alive header for all response types.
  if (req.keep_alive()) // Use same keep-alive option as incoming request
    res->set(http::field::connection, "keep-alive");
  else
    res->set(http::field::connection, "close");

  if (file_contents.str().length() > 0){ // If response body exists
    // Set Cache-Control, Content-Type, and Last-Modified headers
    res->set(http::field::cache_control, "public, max-age=604800, immutable");
    res->set(http::field::content_type, content_type); 
    res->set(http::field::last_modified, last_modified);
    res->body() = file_contents.str(); // Set response body
    res->prepare_payload(); // Set Content-Length
  }
  else if (status == http::status::not_modified) // 304 Not Modified
    // Set Cache-Control header only
    res->set(http::field::cache_control, "public, max-age=604800, immutable");

  return res;
}


/** 
 * Returns the last modified time for the given file. Used for HTTP caching.
 *
 * @param file_obj A path object for the target file.
 * @returns The last modified time for the file in HTTP time string format.
 * @relatesalso FileRequestHandler
 */
std::string last_modified_time(fs::path file_obj){
  std::time_t last_write_time = fs::last_write_time(file_obj);
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
 * @param file_obj A path object for the target file.
 * @returns The MIME type of the file as a string.
 * @relatesalso FileRequestHandler
 */
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


/** 
 * Resolves request target URI to a location block in the web server config.
 *
 * @pre ConfigParser::parse() succeeded.
 * @param req_target The target URI of the incoming request.
 * @param config The parameters of the session that dispatched this handler.
 * @returns A pointer to a parsed location block from the web server config.
 * @relatesalso FileRequestHandler
 */
LocationBlock* get_location(const std::string& req_target, Config* config){
  // Log::trace(LOG_PRE, "Resolving path for request target \"" + req_target + "\".");

  // Step 1. Search location blocks for exact matches
  for (LocationBlock* location : config->locations[LocationBlock::ModifierType::EXACT_MATCH]){
    if (req_target == location->uri){
      // Log::trace(LOG_PRE, req_target + " is an exact match with URI: " + location->uri);
      return location; // Match found, stop searching
    }
  }

  // Step 2. Search location blocks for longest prefix match
  LocationBlock* longest_prefix_match = nullptr;
  LocationBlock* longest_prefix_match_stop = nullptr;

  // Search location blocks for prefix match with stop modifier
  for (LocationBlock* location : config->locations[LocationBlock::ModifierType::PREFIX_MATCH_STOP]){
    if (req_target.find(location->uri) == 0){ // Prefix match
      // Log::trace(LOG_PRE, req_target + " prefix match with stop modifier: " + location->uri);
      if (longest_prefix_match_stop == nullptr || // First prefix match OR
          // Matched URI longer than previous longest prefix match
          location->uri.length() > longest_prefix_match_stop->uri.length())
        longest_prefix_match_stop = location; // Save longest prefix match
    }
  }

  // Search location blocks for prefix match with no modifier
  for (LocationBlock* location : config->locations[LocationBlock::ModifierType::NONE]){
    if (req_target.find(location->uri) == 0){ // Prefix match
      // Log::trace(LOG_PRE, req_target + " prefix match with no modifier: " + location->uri);
      if (longest_prefix_match == nullptr || // First prefix match OR
          // Matched URI longer than previous longest prefix match
          location->uri.length() > longest_prefix_match->uri.length())
        longest_prefix_match = location; // Save longest prefix match
    }
  }

  // Prefix match with stop modifier exists
  if (longest_prefix_match_stop != nullptr){
    if (longest_prefix_match == nullptr || // No prefix match w/o modifier OR
        // Prefix match with stop modifier is longer
        longest_prefix_match_stop->uri.length() > longest_prefix_match->uri.length()){
      // Longest prefix match has a stop modifier
      // Log::trace(LOG_PRE, "Longest prefix match has a stop modifier: " + longest_prefix_match_stop->uri);
      return longest_prefix_match_stop; // Match found, stop searching
    }
  }

  // Prefix match with stop modifier does not exist OR is not longest
  if (longest_prefix_match != nullptr){ // Longest prefix match has no modifier
    // Log::trace(LOG_PRE, "Longest prefix match has no modifier: \"" + longest_prefix_match->uri + "\".");

    /* TODO: Maybe implement regex matching?
    // Log::trace(LOG_PRE, "Longest prefix match has no modifier: \"" + longest_prefix_match->uri + "\". Continuing to regex matching.");
    // Step 3. Search location blocks for regex match. Regex match doesn't care about length, first match wins.
    for (LocationBlock* location : config->locations[LocationBlock::ModifierType::REGEX_MATCH]){
      // TODO: Case sensitivity
      std::smatch matched;
      if (std::regex_search(req_target, matched, std::regex(location->uri))){
        for (std::string match : matched)
          // Log::trace(LOG_PRE, "Found regex match: " + match);
        // TODO: STOP and resolve this to a file path
      }
    }
    // Step 4. Fallback to longest prefix match with no stop modifier
    // Log::trace(LOG_PRE, "No regex match found, using longest prefix match with no stop modifier.");
    */

    return longest_prefix_match; // Match found, stop searching
  }
  return nullptr;
}


/** 
 * Helper function for get_file_from_loc, tests target path for matching file.
 *
 * @pre ConfigParser::parse() succeeded.
 * @param[in] target The target path in the current context (not req_target)
 * @param[in] index The value of the index directive in the current context.
 * @param[out] file_obj A path object for the target file.
 * @returns Boolean true if matching file found, false otherwise.
 * @relatesalso FileRequestHandler
 */
bool resolve_path(const std::string& target, const std::string& index,
                  fs::path& file_obj){
  file_obj = target; // Set path to target being tested

  if (exists(file_obj)){
    if (is_directory(file_obj)){
      file_obj += '/' + index; // If directory, look for index
      if (exists(file_obj))
        return true; // Matched file, get_file_from_loc returns early
    }
    else // Exists and is not a directory
      return true; // Matched file, get_file_from_loc returns early
  }
  return false; // get_file_from_loc continues searching
}


/** 
 * Resolves request target URI to a location block in the web server config.
 *
 * @pre ConfigParser::parse() succeeded.
 * @param[in] req_target The target URI of the incoming request.
 * @param[in] location   A parsed location block in the web server config that
 *                       matches req_target.
 * @param[out] file_obj  A path object for the target file.
 * @param[out] status    HTTP status code associated with the resolved file.
 * @returns Boolean true if matching file found, false otherwise.
 * @relatesalso FileRequestHandler
 */
bool get_file_from_loc(const std::string& req_target, LocationBlock* location,
                       fs::path& file_obj, http::status& status){
  // Log::trace(LOG_PRE, "get_file_from_loc for req_target: \"" + req_target + "\"");
  if (location->try_files_args.size()){ // try_files directive present
    // Try all relative paths specified by the try_files directive
    for (std::string try_files_arg : location->try_files_args){
      // Resolve $uri variable within try_files_arg if present
      std::size_t uri_arg_pos = try_files_arg.find("$uri");
      if (uri_arg_pos == std::string::npos){ // arg does not contain $uri
        // Log::trace(LOG_PRE, "try_files_arg \"" + try_files_arg + "\" does not contain $uri, serving as-is.");
        if (resolve_path(location->root + "/" + try_files_arg, location->index, file_obj))
          return true; // Return early if matching file found
      }
      else{ // arg contains $uri, resolve
        std::string target = try_files_arg; // Copy try_files_arg for in-place replace
        target.replace(uri_arg_pos, 4, req_target);
        // Log::trace(LOG_PRE, "try_files_arg contains $uri, resolved to \"" + target + "\"");
        if (resolve_path(location->root + "/" + target, location->index, file_obj))
          return true; // Return early if matching file found
      }
    }
    // If no early return, no matching file found, use fallback parameter
    if (location->try_files_fallback[0] == '='){ // Fallback is a return code
      // Extract status code from parameter (e.g., "=404")
      std::string fallback_status = location->try_files_fallback.substr(1, 3);
      try{ // Convert fallback status code to boost http::status
        // Throws boost::bad_lexical_cast if not valid integer
        int fallback_status_int = boost::lexical_cast<int>(fallback_status);
        // No throw on invalid status code, just sets http::status::unknown
        status = http::int_to_status(fallback_status_int);
      } 
      catch (boost::bad_lexical_cast){ // Thrown by lexical_cast()
        status = http::status::unknown;
        Log::error(LOG_PRE, "try_files fallback return code is not a valid "
                   "integer, please check the config file.");
      }
    }
    else{ // Fallback parameter is an internal redirect URI
      // Log::trace(LOG_PRE, "Fallback " + location->root + '/' + location->try_files_fallback);
      status = http::status::not_found;
      // Attempt to resolve fallback URI to a file object
      if (resolve_path(location->root + '/' + location->try_files_fallback,
          location->index, file_obj))
        return true; // Return early if matching file found
    }
  }
  else{ // try_files directive not present, serve static file using root/index
    std::string target = req_target; // Copy req_target for in-place replace
    // Substitute location URI with location root
    target.replace(0, location->uri.length(), location->root);
    // Attempt to resolve static file path to a file object
    if (resolve_path(target, location->index, file_obj))
      return true; // Return early if matching file found
  }
  return false; // If no early return, failed to find any matching file
}


/// Returns a pointer to a new file request handler.
RequestHandler* FileRequestHandlerFactory::create(){
  return new FileRequestHandler;
}


/// Register FileRequestHandler and corresponding factory. Runs before main().
REGISTER_HANDLER("FileRequestHandler", FileRequestHandlerFactory)
