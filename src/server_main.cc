#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

#include "server.h"

int main(int argc, char* argv[]){
  try{
    if (argc != 2)
      BOOST_LOG_TRIVIAL(fatal) << "Usage: server <port>";

    boost::asio::io_service io_service;

    server s(io_service, std::atoi(argv[1]));

    io_service.run();
  }
  catch (std::exception& e){
    BOOST_LOG_TRIVIAL(fatal) << "Exception: " + std::string(e.what());
  }
  BOOST_LOG_TRIVIAL(info) << "Server successfully shut down.";
  return 0;
}
