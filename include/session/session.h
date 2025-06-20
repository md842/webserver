#pragma once

#include "request_handler_interface.h" // RequestHandler

class session{
public:
  /** 
   * Sets up the session socket.
   * 
   * @pre ConfigParser::parse() succeeded.
   * @param config A pointer to a parsed Config object that supplies session parameters.
   * @param io_context The boost::asio::io_context supplied by main.
   */
  session(Config* config, boost::asio::io_context& io_context)
    : config_(config), socket_(io_context){}

  /// Returns a reference to the TCP socket used by this session.
  virtual boost::asio::ip::tcp::socket::lowest_layer_type& socket() = 0;

  /// Must be overriden with the appropriate entrypoint for the session type.
  virtual void start() = 0;

protected:
  virtual void do_read();
  void handle_read(const boost::system::error_code& error, size_t bytes);
  void create_response(int status);
  void create_response(Request& req);
  void create_return_response(Request& req);
  virtual void do_write(Response* res, size_t req_bytes,
                        const std::string& req_summary,
                        const std::string& invalid_req);
  void handle_write(const boost::system::error_code& error, size_t res_bytes,
                    Response* res, size_t req_bytes,
                    const std::string& req_summary,
                    const std::string& invalid_req);
  void close(int severity, const std::string& message);
  virtual void do_close();
  RequestHandler* dispatch(Request& req);
  
  std::string client_ip_;
  Config* config_;
  enum{max_length = 1024};
  char data_[max_length];
  boost::asio::ip::tcp::socket socket_;
  std::string total_received_data_ = "";
};