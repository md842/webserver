#pragma once

#include "session/session.h" // session

class http_session : public session<boost::asio::ip::tcp::socket>{
public:
  /** 
   * Sets up the session socket.
   *
   * @pre ConfigParser::parse() succeeded.
   * @param config A pointer to a parsed Config object that supplies session parameters.
   * @param io_context A reference to boost::asio::io_context supplied by main.
   */
  http_session(Config* config, boost::asio::io_context& io_context)
    : session(config){ // Call superclass constructor
    socket_ = new boost::asio::ip::tcp::socket(io_context);
  }

  /// Returns a reference to the TCP socket used by this session.
  boost::asio::ip::tcp::socket& socket() override{
    return *socket_;
  };

  /// The entry point for http_session is do_read()
  void start() override{
    do_read();
  };

private:
  /// Closes the current session.
  void do_close() override{
    boost::system::error_code ec;
    socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    delete this;
  }
};