#include <boost/algorithm/string/replace.hpp> // replace_all
#include <boost/asio.hpp> // io_service
#include <boost/process.hpp> // child, system, std_out, std_err
#include <boost/property_tree/json_parser.hpp> // read_json
#include <boost/property_tree/ptree.hpp> // ptree

#include "post_request_handler.h"
#include "log.h"
#include "nginx_config_parser.h" // Config::inst()
#include "registry.h" // Registry::inst(), REGISTER_HANDLER macro

// Standardized log prefix for this source
#define LOG_PRE "[PostRequestHandler] "

namespace http = boost::beast::http;


/// Generates a response to a given POST request.
Response* PostRequestHandler::handle_request(const Request& req){
  http::status status = http::status::ok; // Response status code 200
  std::string output;

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
      std::string source = req_json.get<std::string>("source");

      // Ensure that the request does not try to leave the intended directory
      if (source.find("../") == std::string::npos){
        // Complete the path for the executable specified by source
        source = Config::inst().root() + "/simulations/" + source;

        boost::asio::io_service async_pipe;
        std::future<std::string> out_future;
        std::future<std::string> err_future;

        if (input_as_file){ // Sim expects file input, write input to file.
          std::string input_file = Config::inst().root() +
                                  "/simulations/temp_input.txt";
          std::ofstream input_file_stream(input_file);
          input_file_stream << input;
          input_file_stream.close();

          // Pipe stdout and stderr of child process to futures declared above
          boost::process::child c(
            // Throws std::system_error if executable not found
            boost::process::system(source, input_file,
                                  boost::process::std_out > out_future,
                                  boost::process::std_err > err_future));
          async_pipe.run(); // Blocks until child process finishes

          // Clear input file; don't want user-supplied input to persist
          std::ofstream clear_input_file(input_file);
          clear_input_file << "";
          clear_input_file.close();
        }
        else{ // Sim expects raw input, provide parsed input directly.
          // Pipe stdout and stderr of child process to futures declared above
          boost::process::child c(
            // Throws std::system_error if executable not found
            boost::process::system(source, input,
                                  boost::process::std_out > out_future,
                                  boost::process::std_err > err_future));
          async_pipe.run(); // Blocks until child process finishes
        }

        std::string err_data = err_future.get();
        // Escape control characters in output JSON
        boost::replace_all(err_data, "\n", "\\n");
        boost::replace_all(err_data, "\t", "\\t");

        std::string out_data = out_future.get();
        // Escape control characters in output JSON
        boost::replace_all(out_data, "\n", "\\n");
        boost::replace_all(out_data, "\t", "\\t");

        output = err_data + out_data;
      }
      else{ // Request tried to leave intended directory
        output = "Error 403: Forbidden";
        status = http::status::forbidden; // Response status code 403
      }
    }
    catch(boost::property_tree::ptree_error e){ // Thrown by ptree.get()
      Log::error(LOG_PRE, "Property tree error: " + std::string(e.what()));
      output = "Error 400: Bad Request";
      status = http::status::bad_request; // Response status code 400
    }
    catch(boost::process::process_error e){ // Thrown by boost::process::system()
      Log::error(LOG_PRE, "Process error: " + std::string(e.what()));
      output = "Error 500: Internal Server Error";
      status = http::status::internal_server_error; // Response status code 500
    }
  }
  catch(boost::property_tree::json_parser_error e){ // Thrown by read_json()
    Log::error(LOG_PRE, "JSON parser error: " + std::string(e.what()));
    output = "Error 400: Bad Request";
    status = http::status::bad_request; // Response status code 400
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

  // Set body
  res->body() = "{\"output\":\"" + output + "\"}";
  res->prepare_payload();
  
  return res;
}


/// Returns a pointer to a new POST request handler.
RequestHandler* PostRequestHandlerFactory::create(){
  return new PostRequestHandler;
}


/// Register PostRequestHandler and corresponding factory. Runs before main().
REGISTER_HANDLER("PostRequestHandler", PostRequestHandlerFactory)
