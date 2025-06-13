#pragma once

#include <boost/filesystem/fstream.hpp> // ifstream
#include <vector>

#include "nginx_config.h" // Config

class ConfigParser final{ // Singleton class (only one instance)
 public:
  // Deleting the copy and assignment operators due to being a singleton class
  ConfigParser(const ConfigParser&) = delete;
  ConfigParser& operator=(const ConfigParser&) = delete;

  /// Returns a static reference to the singleton instance of ConfigParser.
  static ConfigParser& inst();

  /** 
   * Returns the parsed Config objects.
   * 
   * @pre parse() succeeded.
   * @returns ConfigParser.configs_
   */
  std::vector<Config*> configs();

  /** 
   * Sets the absolute root directory for conversion of the relative root.
   * 
   * @param absolute_root A string containing the absolute path of the web
   *   server root directory.
   */
  void set_absolute_root(const std::string& absolute_root);
  
  /** 
   * Parses the specified config file and populates ConfigParser.configs_.
   * 
   * @param file_path A string containing the path to the config file.
   * @returns true on successful parse, false on parse error.
   */
  bool parse(const std::string& file_path);

 private:
  ConfigParser(){}; // Making constructor private due to being a singleton class
  bool parse(boost::filesystem::ifstream& cfg_in);
  bool parse_block_start(std::vector<std::string>& statement);
  bool parse_block_end(std::vector<std::string>& statement);
  bool parse_statement(std::vector<std::string>& statement);
  void register_mapping(const std::string& arg);
  bool validate_config();

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

  TokenType get_token(boost::filesystem::ifstream& cfg_in, std::string& token);

  enum TokenParserState{
    INIT_STATE = 0,
    COMMENT_STATE = 1,
    SINGLE_QUOTE_STATE = 2,
    DOUBLE_QUOTE_STATE = 3,
    END_QUOTE_STATE = 4,
    WORD_STATE = 5
  };

  std::string absolute_root_;
  std::vector<Config*> configs_;
  Context context = MAIN_CONTEXT;
  Config* cur_config;
  std::string uri;
};
