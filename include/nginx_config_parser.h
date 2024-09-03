#pragma once

#include <boost/filesystem/fstream.hpp> // ifstream
#include <string>

struct NginxConfig{
  short port;
  std::string index;
  std::string root;
};

class Config final{ // Singleton class (only one instance)
 public:
  // Deleting the copy and assignment operators due to being a singleton class
  Config(const Config&) = delete;
  Config& operator=(const Config&) = delete;

  /// Returns a static reference to the singleton instance of Config.
  static Config& inst();

  /** 
   * Returns the path to the index page specified by Config.
   * 
   * @pre parse() succeeded.
   * @returns NginxConfig.index
   */
  std::string index();

  /** 
   * Returns the port number specified by Config.
   * 
   * @pre parse() succeeded.
   * @returns NginxConfig.port
   */
  short port();

  /** 
   * Returns the path to the root directory specified by Config.
   * 
   * @pre parse() succeeded.
   * @returns NginxConfig.root
   */
  std::string root();

  /** 
   * Converts the relative root directory in NginxConfig to an absolute root.
   * 
   * @pre parse() succeeded.
   * @param absolute_root A string containing the absolute path of the web
   *   server root directory.
   */
  void set_absolute_root(const std::string& absolute_root);
  
  /** 
   * Parses the specified config file and populates NginxConfig.
   * 
   * @param file_path A string containing the path to the config file.
   * @returns true on successful parse, false on parse error.
   */
  bool parse(const std::string& file_path);

 private:
  Config(){}; // Making constructor private due to being a singleton class

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
  std::string name;
  std::string uri;

  bool parse(boost::filesystem::ifstream& cfg_in);
  bool parse_block_start(std::vector<std::string>& statement);
  bool parse_block_end(std::vector<std::string>& statement);
  bool parse_statement(std::vector<std::string>& statement);
  void register_mapping(const std::string& arg);
  bool validate_config();

  TokenType get_token(boost::filesystem::ifstream& cfg_in, std::string& token);
};
