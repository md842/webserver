#include <boost/algorithm/string/replace.hpp> // replace_all
#include <boost/asio.hpp> // io_context, basic_readable_pipe
#include <boost/process/v2/process.hpp> // process::proc
#include <boost/process/v2/stdio.hpp> // process_stdio
#include <boost/property_tree/json_parser.hpp> // read_json
#include <boost/property_tree/ptree.hpp> // ptree

#include "analytics.h"
#include "post_request_handler.h"
#include "log.h"
#include "nginx_config.h" // Config
#include "registry.h" // Registry::inst(), REGISTER_HANDLER macro

// Standardized log prefix for this source
#define LOG_PRE "[PostRequestHandler] "

namespace http = boost::beast::http;
namespace procv2 = boost::process::v2;


/// Generates a response to a given POST request.
Response* PostRequestHandler::handle_request(const Request& req){
  http::status status = http::status::ok; // Response status code 200
  std::string stdout_data, stderr_data;

  // Parse JSON data received in req.body()
  boost::property_tree::ptree req_json;
  std::istringstream req_body(req.body());

  try{
    // Throws boost::property_tree::json_parser_error
    read_json(req_body, req_json);

    try{
      // Throws boost::property_tree::ptree_error if named nodes not present
      std::string input = req_json.get<std::string>("input");
      bool input_as_file = req_json.get<bool>("input_as_file");
      std::string binary_path = req_json.get<std::string>("source");

      // Ensure that the request does not try to leave the intended directory
      if (binary_path.find("../") == std::string::npos){
        // Complete the path for the executable specified by source
        binary_path = config_.root + "/simulations/" + binary_path;

        boost::asio::io_context child_proc_io_context;
        boost::asio::basic_readable_pipe stdout_pipe(child_proc_io_context);
        boost::asio::basic_readable_pipe stderr_pipe(child_proc_io_context);

        if (input_as_file){ // Sim expects file input, write raw input to file
          std::string input_file = config_.root + "/simulations/temp_input.txt";
          std::ofstream input_file_stream(input_file);
          input_file_stream << input;
          input_file_stream.close();
          // Done with raw input, overwrite for convenience in proc call below
          input = input_file;
        }

        try{
          // Throws boost::system::system_error if binary_path not found
          // Launch child process with stdout and stderr piped
          procv2::process proc(child_proc_io_context, binary_path, {input},
                               procv2::process_stdio{{}, // default stdin
                                                     stdout_pipe,
                                                     stderr_pipe});
          proc.wait(); // Block until child process finishes

          boost::system::error_code ec; // ec == eof indicates successful read

          // Read piped stderr from child process into stderr_data buffer
          boost::asio::read(stderr_pipe, boost::asio::dynamic_buffer(stderr_data), ec);
          if (ec == boost::asio::error::eof){ // Successful read
            // Escape control characters in output JSON
            boost::replace_all(stderr_data, "\n", "\\n");
            boost::replace_all(stderr_data, "\t", "\\t");
          }
          else{ // Unsuccessful read
            Log::error(LOG_PRE, "Failed to read stderr_pipe.");
            stdout_data = "Error 500: Internal Server Error";
            status = http::status::internal_server_error; // Response status code 500
          }
          
          // Read piped stderr from child process into stdout_data buffer
          boost::asio::read(stdout_pipe, boost::asio::dynamic_buffer(stdout_data), ec);
          if (ec == boost::asio::error::eof){ // Successful read
            // Escape control characters in output JSON
            boost::replace_all(stdout_data, "\n", "\\n");
            boost::replace_all(stdout_data, "\t", "\\t");
          }
          else{ // Unsuccessful read
            Log::error(LOG_PRE, "Failed to read stdout_pipe.");
            stdout_data = "Error 500: Internal Server Error";
            status = http::status::internal_server_error; // Response status code 500
          }
          Analytics::inst().posts++; // Log valid POST request in analytics
        }
        catch(boost::system::system_error e){ // Thrown by procv2::process::proc()
          Log::warn(LOG_PRE, "POST request specified unknown executable (likely malicious).");
          stdout_data = "Error 404: Not Found";
          status = http::status::not_found; // Response status code 404
          Analytics::inst().malicious++; // Log malicious request in analytics
        }

        stderr_pipe.close(); // Ensure pipes closed regardless of success/fail
        stdout_pipe.close();

        if (input_as_file){ // Clear input file; user input shouldn't persist
          std::ofstream clear_input_file(input);
          clear_input_file << "";
          clear_input_file.close();
        }
      }
      else{ // Request tried to leave intended directory
        Log::warn(LOG_PRE, "Malicious POST request detected.");
        stdout_data = "Error 403: Forbidden";
        status = http::status::forbidden; // Response status code 403
        Analytics::inst().malicious++; // Log malicious request in analytics
      }
    }
    catch(boost::property_tree::ptree_error e){ // Thrown by ptree.get()
      Log::error(LOG_PRE, "Property tree error: " + std::string(e.what()));
      stdout_data = "Error 400: Bad Request";
      status = http::status::bad_request; // Response status code 400
      Analytics::inst().invalid++; // Log invalid request in analytics
    }
  }
  catch(boost::property_tree::json_parser_error e){ // Thrown by read_json()
    Log::error(LOG_PRE, "JSON parser error: " + std::string(e.what()));
    stdout_data = "Error 400: Bad Request";
    status = http::status::bad_request; // Response status code 400
    Analytics::inst().invalid++; // Log invalid request in analytics
  }

  // Construct and return pointer to HTTP response object
  Response* res = new Response();
  res->result(status);
  res->version(11);

  /* Set headers; Content-Type should be set to JSON even for an error response
     because PostRequestHandler uses JSON for error reporting to the client */
  res->set(http::field::cache_control, "public, max-age=604800, immutable");
  res->set(http::field::content_type, "application/json");

  if (req.keep_alive()) // Use same keep-alive option as incoming request
    res->set(http::field::connection, "keep-alive");
  else
    res->set(http::field::connection, "close");

  // Populate JSON body with cout and cerr output by the simulation
  res->body() = "{"
    R"("cout":")" + stdout_data + "\","
    R"("cerr":")" + stderr_data + "\""
  "}";
  res->prepare_payload();
  
  return res;
}


/// Returns a pointer to a new POST request handler.
RequestHandler* PostRequestHandlerFactory::create(){
  return new PostRequestHandler;
}


/// Register PostRequestHandler and corresponding factory. Runs before main().
REGISTER_HANDLER("PostRequestHandler", PostRequestHandlerFactory)
