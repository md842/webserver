#include <boost/asio.hpp> // io_context, signal_set
#include <boost/filesystem.hpp> // parent_path, system_complete

#include "log.h"
#include "nginx_config_parser.h" // Config, ConfigParser, LocationBlock
#include "server/http_server.h" // http_server
#include "server/https_server.h" // https_server

// Standardized log prefix for this source
#define LOG_PRE "[Main]     "

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

    /* Find root directory from binary path argv[0], works regardless of cwd.
       Binary is built at <root>/build/bin/server, so calling parent_path()
       thrice from binary always lands in the webserver root directory. */
    std::string root_dir = boost::filesystem::system_complete(argv[0]).
      parent_path().parent_path().parent_path().string();

    // Config may provide relative paths, set working directory as found above.
    ConfigParser::inst().set_working_directory(root_dir);

    /* Parse the config file given in argv[1]. If a parse error occurs,
       ConfigParser will handle the fatal log, so just exit here. */
    if (!ConfigParser::inst().parse(root_dir + "/" + argv[1]))
      return 1; // Exit with non-zero exit code

    /* Dynamically allocate server instances to prevent lifetime from expiring
       while still in use (manifests as error message "Operation canceled"). */
    std::vector<server*> servers;

    // For each config, launch a server instance
    for (Config* config : ConfigParser::inst().configs()){
      switch (config->type){
        case Config::ServerType::HTTP_SERVER:
          servers.push_back(new http_server(config, io_context_));
          break;
        case Config::ServerType::HTTPS_SERVER:
          servers.push_back(new https_server(config, io_context_));
      }
    }

    io_context_.run(); // Blocks until signal_handler calls io_context_.stop()

    // After IO context stops blocking, free all dynamically allocated memory.
    for (server* server : servers)
      delete server;
    for (Config* config : ConfigParser::inst().configs()){
      for (std::vector<LocationBlock*> location_block_vec : config->locations){
        for (LocationBlock* location : location_block_vec)
          delete location;
      }
      delete config;
    }
  }
  catch (std::exception& e){
    Log::fatal(LOG_PRE, "Exception " + std::string(e.what()));
    return 1; // Exit with non-zero exit code
  }
  Log::info(LOG_PRE, "Server successfully shut down.");
  return 0;
}