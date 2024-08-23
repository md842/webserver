#include <boost/algorithm/string/replace.hpp> // replace_all
#include <boost/asio.hpp> // io_service
#include <boost/process.hpp> // child, system, std_out, std_err
#include <boost/property_tree/ptree.hpp> // ptree
#include <boost/property_tree/json_parser.hpp> // read_json

#include "post_request_handler.h"
#include "log.h"
#include "nginx_config_parser.h" // Config::inst()
#include "registry.h" // Registry::inst(), REGISTER_HANDLER macro

namespace http = boost::beast::http;

Response* PostRequestHandler::handle_request(const Request& req){
  // Returns a pointer to an HTTP response object for the given HTTP request.
  http::status status = http::status::ok; // Response status code 200
  std::string output;

  // Parse JSON data received in req.body()
  boost::property_tree::ptree req_json;
  std::istringstream req_body(req.body());
  read_json(req_body, req_json);

  // TODO: Support Python as well?

  try{
    // Throws boost::property_tree::ptree_error if requested nodes not present
    std::string input = req_json.get<std::string>("input");
    bool input_as_file = req_json.get<bool>("input_as_file");
    std::string source = req_json.get<std::string>("source");

    // Complete the path for the executable specified by source
    source = Config::inst().root() + "frontend/public/simulations/" + source;

    /* TODO: Additional verification of request legitimacy, need to be careful
       because this request type allows running an executable on the server.
       Make sure source doesn't contain ".." to leave the intended dir. */

    boost::asio::io_service async_pipe;
    std::future<std::string> out_future;
    std::future<std::string> err_future;

    if (input_as_file){ // Sim expects file input, write parsed input to file.
      std::string input_file = Config::inst().root() +
                              "frontend/public/simulations/temp_input.txt";
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
  catch(boost::property_tree::ptree_error){ // Thrown by ptree.get()
    output = "Error 400: Bad Request";
    status = http::status::bad_request; // Response status code 400
  }
  catch(boost::process::process_error){ // Thrown by boost::process::system()
    output = "Error 500: Internal Server Error";
    status = http::status::internal_server_error; // Response status code 500
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
  res->set(http::field::content_type, "application/json"); 

  // Set body
  res->body() = "{\"output\":\"" + output + "\"}";
  res->prepare_payload();
  
  return res;
}

RequestHandler* PostRequestHandlerFactory::create(){
  // Returns a pointer to a new file request handler for the given path.
  return new PostRequestHandler;
}

// Register FileRequestHandler and corresponding factory. Runs before main().
REGISTER_HANDLER("PostRequestHandler", PostRequestHandlerFactory)
