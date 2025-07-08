#pragma once

#include "session/session_base.h" // session_base

template <class AsyncWriteStream>
class session : public session_base{
public:
  /** 
   * Sets up the session socket.
   * 
   * @pre ConfigParser::parse() succeeded.
   * @param config A pointer to a parsed Config object that supplies session parameters.
   */
  session(Config* config) : session_base(config){}

  /// Delete the dynamically allocated socket upon session deletion.
  ~session(){delete socket_;}

protected:
  void do_read() override;
  void do_write(Response* res, Log::req_info& req_info) override;
  
  /* Must be a pointer, otherwise constructor will complain that socket_ is
     missing from initializer list. Can't include in initializer list because
     the two different socket types have different constructor params. */
  AsyncWriteStream* socket_; // Belongs to session, should be deleted by destructor
};