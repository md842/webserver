#pragma once

#include "session/session.h"

template <typename T>
class server{
public:
  /** 
   * Initializes the server instance.
   *
   * @pre ConfigParser::parse() succeeded.
   * @param config A pointer to a parsed Config object that supplies server parameters.
   * @param io_context A reference to boost::asio::io_context supplied by main.
   */
  server(Config* config, boost::asio::io_context& io_context);

protected:
  virtual void start_accept() = 0; // Must override
  void handle_accept(session<T>* new_session,
                     const boost::system::error_code& error);
  
  boost::asio::ip::tcp::acceptor acceptor_;
  Config* config_;
  boost::asio::io_context& io_context_;
};