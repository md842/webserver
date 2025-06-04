#include <boost/asio.hpp> // io_context
#include <boost/asio/ssl.hpp> // ssl::context
#include <boost/filesystem.hpp> // system_complete

#include "log.h"
#include "nginx_config_parser.h" // Config, ConfigParser
#include "registry.h" // Registry::inst()
#include "server/http_server.h"
#include "server/https_server.h"

// Standardized log prefix for this source
#define LOG_PRE "[Main]     "

namespace fs = boost::filesystem;
namespace ssl = boost::asio::ssl;

// Made global so that it can be stopped gracefully by signal_handler.
boost::asio::io_context io_context_;


// Used by signals.async_wait, stops the IO context upon receiving a signal.
void signal_handler(const boost::system::error_code& ec, int sig){
  if (sig == SIGINT)
    Log::info(LOG_PRE, "SIGINT received, shutting down gracefully.");
  else
    Log::info(LOG_PRE, "SIGTERM received, shutting down gracefully.");
  io_context_.stop(); // Causes io_context_.run() to stop blocking main
}


int main(int argc, char* argv[]){
  try{
    if (argc != 2){ // Check args
      Log::fatal(LOG_PRE, "Usage: server <config>");
      return 1; // Exit with non-zero exit code
    }

    // Register signal_handler to handle SIGINT and SIGTERM.
    boost::asio::signal_set signals(io_context_, SIGINT, SIGTERM);
    signals.async_wait(signal_handler);

    // Find root directory from binary path argv[0], works regardless of cwd
    std::string binary_path = fs::system_complete(argv[0]).string();
    std::string root_dir = "";
    std::string target_dir = "webserver"; // Project directory name
    size_t found = binary_path.find(target_dir); // Search for substring
    if (found != std::string::npos) // Found, extract root dir
      root_dir = binary_path.substr(0, found + target_dir.length());

    // Config's root dir is relative, so provide absolute root_dir found above.
    ConfigParser::inst().set_absolute_root(root_dir);

    // Parse the config file given in argv[1] (Config is a singleton).
    // If a parse error occurs, Config will handle the fatal log, so just exit.
    if (!ConfigParser::inst().parse(root_dir + "/" + argv[1]))
      return 1; // Exit with non-zero exit code

    /* Keep all server instances in a server* vector to prevent them from going
       out of scope (manifests as error message "Operation canceled"). */
    std::vector<server*> servers;
    // For each config, launch a server instance
    for (Config &config : ConfigParser::inst().configs()){
      // TODO: Should these use separate classes?
      if (config.type == Config::ServerType::REDIRECT ||
          config.type == Config::ServerType::HTTP_SERVER)
        servers.push_back(new http_server(config, io_context_));
      else{
        // Create and configure SSL context
        ssl::context ssl_context_(ssl::context::tlsv12_server);
        // TODO: Get certificate paths from config instead of hardcoding
        // TODO: Figure out how to use real certificates from outside of the repository
        ssl_context_.use_certificate_file(root_dir + "/tests/certs/localhost.crt", ssl::context::pem);
        ssl_context_.use_private_key_file(root_dir + "/tests/certs/localhost.key", ssl::context::pem);
        // Launch HTTPS server instance
        servers.push_back(new https_server(config, io_context_, ssl_context_));
      }
    }

    io_context_.run(); // Blocks until signal_handler calls io_context_.stop()

    for (server* server : servers) // Free memory reserved by server instances
      delete server;
  }
  catch (std::exception& e){
    Log::fatal(LOG_PRE, "Exception " + std::string(e.what()));
    return 1; // Exit with non-zero exit code
  }
  Log::info(LOG_PRE, "Server successfully shut down.");
  return 0;
}