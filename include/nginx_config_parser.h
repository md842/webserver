#pragma once

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <string>

struct NginxConfig{
  short port;
  std::string index;
  std::string root;
};

class Config final{ // Singleton class (only one instance)
 public:
  // Singleton should delete copy and assignment operators
  Config(const Config&) = delete;
  Config& operator=(const Config&) = delete;

  static Config& inst(); // Get static instance

  std::string index();
  short port();
  std::string root();
  void set_absolute_root(const std::string& absolute_root);
  
  bool parse(const std::string& file_name);

 private:
  Config(){}; // Singleton should have private constructor

  enum Context{
    MAIN_CONTEXT = 0,
    HTTP_CONTEXT = 1,
    SERVER_CONTEXT = 2,
    LOCATION_CONTEXT = 3
  };

  enum TokenType{
    INVALID = -1,
    INIT = 0,
    BLOCK_START = 1,
    BLOCK_END = 2,
    SEMICOLON = 3,
    COMMENT = 4,
    WORD = 5,
    QUOTE_WORD = 6,
    EOF_ = 7
  };

  enum TokenParserState{
    INIT_STATE = 0,
    COMMENT_STATE = 1,
    SINGLE_QUOTE_STATE = 2,
    DOUBLE_QUOTE_STATE = 3,
    END_QUOTE_STATE = 4,
    WORD_STATE = 5
  };

  Context context = MAIN_CONTEXT;
  NginxConfig config;
  std::string mapping;
  std::string name;
  std::string uri;

  bool parse(boost::filesystem::ifstream& cfg_in);
  bool parse_block_start(std::vector<std::string>& statement);
  bool parse_block_end(std::vector<std::string>& statement);
  bool parse_statement(std::vector<std::string>& statement);
  void process_mapping(const std::string& arg);
  bool validate_config();

  TokenType get_token(boost::filesystem::ifstream& cfg_in, std::string& token);
};
