#pragma once

#include "session/session.h" // session

class http_session : public session{
public:
  /** 
   * Sets up the session socket.
   *
   * @pre ConfigParser::parse() succeeded.
   * @param config A pointer to a parsed Config object that supplies session parameters.
   * @param io_context A reference to boost::asio::io_context supplied by main.
   */
  http_session(Config* config, boost::asio::io_context& io_context)
    : session(config, io_context){} // Call superclass constructor

  /// Returns a reference to the TCP socket used by this session.
  boost::asio::ip::tcp::socket::lowest_layer_type& socket() override{
    return socket_.lowest_layer();
  };

  /// The entry point for http_session is do_read()
  void start() override{do_read();};
};