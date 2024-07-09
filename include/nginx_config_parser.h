#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <string>

struct NginxConfig{
  short port;
  std::string index;
  std::string root;
};

class Parser{
 public:
  bool parse(const std::string& file_name);

 private:
  enum TokenType {
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

  enum ParserState {
    INIT_STATE = 0,
    COMMENT_STATE = 1,
    SINGLE_QUOTE_STATE = 2,
    DOUBLE_QUOTE_STATE = 3,
    END_QUOTE_STATE = 4,
    WORD_STATE = 5
  };

  NginxConfig config;

  TokenType get_token(boost::filesystem::ifstream& cfg_in, std::string& token);
  bool parse(boost::filesystem::ifstream& cfg_in);
  std::string type_str(TokenType type);
};
