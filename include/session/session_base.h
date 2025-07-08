#pragma once

#include "log.h" // req_info
#include "nginx_config.h" // Config
#include "typedefs/http.h" // Request, Response

class session_base{
public:
  /** 
   * Sets up the session socket.
   * 
   * @pre ConfigParser::parse() succeeded.
   * @param config A pointer to a parsed Config object that supplies session parameters.
   */
  session_base(Config* config) : config_(config){}

  /// Returns a reference to the TCP socket used by this session.
  virtual boost::asio::ip::tcp::socket& socket() = 0; // Must be overriden

  /// Must be overriden with the appropriate entrypoint for the session type.
  virtual void start() = 0; // Must be overriden

protected:
  virtual void do_read() = 0; // Must be overriden
  void handle_read(const boost::system::error_code& error, size_t bytes);
  void create_response(int status);
  void create_response(Request& req);
  void create_return_response(Request& req);
  virtual void do_write(Response* res, Log::req_info& req_info) = 0; // Must be overriden
  void handle_write(const boost::system::error_code& error, size_t res_bytes,
                    Response* res, Log::req_info& req_info);
  void close(int severity, const std::string& message);
  virtual void do_close() = 0; // Must be overriden
  
  std::string client_ip_;
  Config* config_; // Belongs to server, should not be deleted by destructor
  enum{max_length = 1024};
  char data_[max_length];
  std::string total_received_data_ = "";
};