#include <boost/asio.hpp> // io_service

#include "log.h"
#include "server.h"

int main(int argc, char* argv[]){
  try{
    if (argc != 2)
      Log::fatal("Usage: server <port>");

    boost::asio::io_service io_service;

    server s(io_service, std::atoi(argv[1]));

    io_service.run();
  }
  catch (std::exception& e){
    Log::fatal("Exception: " + std::string(e.what()));
  }
  Log::info("Server successfully shut down.");
  return 0;
}
