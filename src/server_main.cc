#include <boost/asio.hpp> // io_service
#include <boost/filesystem.hpp> // system_complete

#include "log.h"
#include "server.h"

namespace fs = boost::filesystem;

int main(int argc, char* argv[]){
  try{
    if (argc != 2)
      Log::fatal("Usage: server <port>");

    boost::asio::io_service io_service;
    short port = std::atoi(argv[1]); // TODO: Get from config parse

    // Find root directory from binary path argv[0], works regardless of cwd
    std::string binary_path = fs::system_complete(argv[0]).string();
    std::string root_dir = "";
    std::string target_dir = "webserver"; // Project directory name
    size_t found = binary_path.find(target_dir); // Search for substring
    if (found != std::string::npos){ // Found, extract root dir
      root_dir = binary_path.substr(0, found + target_dir.length());
      Log::info("Main: Found root directory " + root_dir);
    }

    server s(io_service, port, root_dir);
    io_service.run();
  }
  catch (std::exception& e){
    Log::fatal("Exception: " + std::string(e.what()));
  }
  Log::info("Server successfully shut down.");
  return 0;
}
