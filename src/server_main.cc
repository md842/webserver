#include <boost/asio.hpp> // io_service
#include <boost/filesystem.hpp> // system_complete
#include <regex>

#include "log.h"
#include "nginx_config_parser.h" // Config::inst()
#include "registry.h" // Registry::inst()
#include "server.h"

namespace fs = boost::filesystem;

// Global so it can be stopped gracefully by signal_handler
boost::asio::io_service io_service_;

void signal_handler(const boost::system::error_code& ec, int sig){
  if (sig == SIGINT)
    Log::info("Main: SIGINT received, shutting down gracefully.");
  else
    Log::info("Main: SIGTERM received, shutting down gracefully.");
  io_service_.stop(); // Causes io_service_.run() to stop blocking main
}

int main(int argc, char* argv[]){
  try{
    if (argc != 2){ // Check args
      Log::fatal("Usage: server <config>");
      return 1; // Exit with non-zero exit code
    }

    Log::enable_trace(); // Remove to suppress trace logs
    // TODO: Make this configurable rather than hardcoded

    // Find root directory from binary path argv[0], works regardless of cwd
    std::string binary_path = fs::system_complete(argv[0]).string();
    std::string root_dir = "";
    std::string target_dir = "webserver"; // Project directory name
    size_t found = binary_path.find(target_dir); // Search for substring
    if (found != std::string::npos) // Found, extract root dir
      root_dir = binary_path.substr(0, found + target_dir.length());

    // Parse the config file given in argv[1] (Config is a singleton).
    // If a parse error occurs, Config will handle the fatal log, so just exit.
    if (!Config::inst().parse(root_dir + "/" + argv[1]))
      return 1; // Exit with non-zero exit code

    // Config's root dir is relative, so set the absolute root_dir found above.
    Config::inst().set_absolute_root(root_dir);
    Log::info("Main: Found root directory: " + Config::inst().root());
    
    // Log mapping that was extracted from the config
    for (const std::string& type : Registry::inst().get_types()){
      for (auto& pair : Registry::inst().get_map(type))
        Log::info(type + ": Mapping URI prefix " + pair.first +
                  " to relative path " + pair.second);
    }

    // Register signal_handler to handle SIGINT and SIGTERM.
    boost::asio::signal_set signals(io_service_, SIGINT, SIGTERM);
    signals.async_wait(signal_handler);

    server s(io_service_);
    io_service_.run(); // Blocks until signal_handler calls io_service_.stop()
  }
  catch (std::exception& e){
    Log::fatal("Main: Exception " + std::string(e.what()));
    return 1; // Exit with non-zero exit code
  }
  Log::info("Main: Server successfully shut down.");
  return 0;
}