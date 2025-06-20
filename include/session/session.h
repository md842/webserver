#pragma once

#include <boost/asio/ssl.hpp> // ssl::stream

#include "request_handler_interface.h" // RequestHandler

template <typename T>
class session{
public:
  /** 
   * Sets up the session socket.
   * 
   * @pre ConfigParser::parse() succeeded.
   * @param config A pointer to a parsed Config object that supplies session parameters.
   */
  session(Config* config) : config_(config){}

  /// Delete the dynamically allocated socket upon session deletion.
  ~session(){delete socket_;}

  /// Returns a reference to the TCP socket used by this session.
  virtual boost::asio::ip::tcp::socket& socket() = 0;

  /// Must be overriden with the appropriate entrypoint for the session type.
  virtual void start() = 0;

protected:
  void do_read();
  void handle_read(const boost::system::error_code& error, size_t bytes);
  void create_response(int status);
  void create_response(Request& req);
  void create_return_response(Request& req);
  void do_write(Response* res, size_t req_bytes,
                const std::string& req_summary,
                const std::string& invalid_req);
  void handle_write(const boost::system::error_code& error, size_t res_bytes,
                    Response* res, size_t req_bytes,
                    const std::string& req_summary,
                    const std::string& invalid_req);
  void close(int severity, const std::string& message);
  virtual void do_close() = 0; // Must be overriden
  
  std::string client_ip_;
  Config* config_;
  enum{max_length = 1024};
  char data_[max_length];
  /* Must be a pointer, otherwise constructor will complain that socket_ is
     missing from initializer list. Can't include in initializer list because
     the two different socket types have different constructor params. */
  T* socket_;
  std::string total_received_data_ = "";
};