#include <boost/lexical_cast.hpp>
#include <regex>

#include "log.h"
#include "nginx_config_parser.h"
#include "registry.h" // Registry::inst()

namespace fs = boost::filesystem;

std::string clean(const std::string& path);

Config& Config::inst(){
  // Returns a reference to the static instance of the config.
  static Config instRef;
  return instRef;
}

std::string Config::index(){
  // Returns index from the parsed config object.
  // Should not be called before parse().
  return config.index;
}

short Config::port(){
  // Returns port from the parsed config object.
  // Should not be called before parse().
  return config.port;
}

std::string Config::root(){
  // Returns root from the parsed config object.
  // Should not be called before parse().
  return config.root;
}

void Config::set_absolute_root(const std::string& absolute_root){
  // Prepends the absolute root to config.root (relative) and cleans the path.
  // Should be called by main after parse().
  config.root = clean(absolute_root + config.root);
}

bool Config::parse(const std::string& file_path){
  // Sets up a file stream for a given file path, then calls the config parser.
  // Returns true if parsing was successful, false otherwise.
  fs::path file_obj(file_path);

  if (!exists(file_obj) || is_directory(file_obj)){ // Nonexistent or directory
    Log::fatal("Config: " + file_path + " not found, aborting.");
    return false;
  }
  else{ // Non-directory file found
    fs::ifstream fstream(file_obj); // Attempt to open the file
    if (!fstream){ // File exists, but failed to open it for some reason.
      Log::fatal("Config: " + file_path + " not found, aborting.");
      return false;
    }
    else{ // File opened successfully
      Log::info("Config: Parsing " + file_path);
      return parse(fstream);
    }
  }
}

bool Config::parse(fs::ifstream& cfg_in){
  // Parses the config file given by the file stream cfg_in.
  // Returns true if parsing was successful, false otherwise.
  TokenType prev_type = INIT;
  TokenType token_type = INIT;
  std::vector<std::string> statement;
  std::string token;

  while (true){
    token = "";
    token_type = get_token(cfg_in, token); // Populates token string

    if (token_type == INVALID){ // Encountered error while parsing token
      Log::fatal("Config: Invalid token \"" + token + "\", aborting.");
      return false;
    }

    if (token_type == QUOTE_WORD){
      token = token.substr(1, token.length() - 2); // Trim quotes
      token_type = WORD;
    }

    if (token_type == WORD){
      statement.push_back(token); // Add word to statement
      prev_type = WORD;           // Set prev_type
    }

    else if (token_type == BLOCK_START){
      if (prev_type == WORD){ // WORD must precede BLOCK_START
        statement.push_back(token);
        if (!parse_block_start(statement)) // Parse block start, check error
          return false; // Only return if an error occurred, otherwise continue
        prev_type = BLOCK_START;
      }
      else // Invalid config file structure
        break;
    }

    else if (token_type == BLOCK_END){
      // SEMICOLON or BLOCK_END must precede BLOCK_END
      if (prev_type == SEMICOLON || prev_type == BLOCK_END){
        statement.push_back(token);
        if (!parse_block_end(statement)) // Parse block end, check error
          return false; // Only return if an error occurred, otherwise continue
        prev_type = BLOCK_END;
      }
      else // Invalid config file structure
        break;
    }

    else if (token_type == SEMICOLON){
      if (prev_type == WORD){ // WORD must precede SEMICOLON
        statement.push_back(token);
        if (!parse_statement(statement)) // Parse statement, check error
          return false; // Only return if an error occurred, otherwise continue
        prev_type = SEMICOLON;
      }
      else // Invalid config file structure
        break;
    }

    else if (token_type == EOF_){
      if (prev_type == BLOCK_END){ // BLOCK_END must precede EOF
        return validate_config(); // Valid config file structure
      }
      else // Invalid config file structure
        break;
    }
  }

  Log::fatal("Config: Invalid transition to token \"" + token + "\".");
  return false;
}

bool Config::parse_block_start(std::vector<std::string>& statement){
  // Parses a block start statement within the config.
  // Returns true if parsing was successful, false otherwise.
  std::string new_context = statement.at(0); // First token is target context

  // Verify statement size; 2 tokens for http/server, 4 tokens for location
  if (new_context == "http" || new_context == "server"){
    if (statement.size() != 2){
      Log::fatal("Config: Malformed " + new_context + " block (size " +
                 std::to_string(statement.size()) + ", expected size 2)");
      return false;
    }
  }
  else if (new_context == "location"){
    if (statement.size() != 4){
      Log::fatal("Config: Malformed location block (size " +
                 std::to_string(statement.size()) + ", expected size 4)");
      return false;
    }
  }
  else{
    Log::fatal("Config: Unknown target context " + new_context);
    return false;
  }

  // Verify and perform context transition
  if (context == MAIN_CONTEXT && new_context == "http")
    context = HTTP_CONTEXT;
  else if (context == HTTP_CONTEXT && new_context == "server")
    context = SERVER_CONTEXT;
  else if (context == SERVER_CONTEXT && new_context == "location"){
    context = LOCATION_CONTEXT;
    uri = statement.at(1); // Get URI
    name = statement.at(2); // Get handler name
  }
  else{
    Log::fatal("Config: Invalid context transition to " + new_context);
    return false;
  }

  statement.clear(); // Reset statement after parsing
  return true;
}

bool Config::parse_block_end(std::vector<std::string>& statement){
  // Parses a block end statement within the config.
  // Returns true if parsing was successful, false otherwise.
  if (statement.size() != 1){ // Verify statement size; 1 token "}"
    Log::fatal("Config: Malformed block end (size " +
                std::to_string(statement.size()) + ", expected size 1)");
    return false;
  }

  // Verify and perform context transition
  if (context == LOCATION_CONTEXT)
    context = SERVER_CONTEXT;
  else if (context == SERVER_CONTEXT)
    context = HTTP_CONTEXT;
  else if (context == HTTP_CONTEXT)
    context = MAIN_CONTEXT;
  else{
    Log::fatal("Config: Invalid context exit from main; "
               "check number of closing brackets");
    return false;
  }

  statement.clear(); // Reset statement after parsing
  return true;
}

bool Config::parse_statement(std::vector<std::string>& statement){
  // Parses a general statement within the config.
  // Returns true if parsing was successful, false otherwise.
  std::string arg = statement.at(0); // First token is argument type

  // Valid in server context: listen, index, root, server_name, return
  if (context == SERVER_CONTEXT){
    if (arg == "listen"){
      try{
        config.port = boost::lexical_cast<short>(statement.at(1));
        Log::trace("Config: Got port " + std::to_string(config.port));
      }
      catch(boost::bad_lexical_cast&){ // Out of range, not a number, etc.
        Log::fatal("Config: Invalid port \"" + statement.at(1) + "\"");
        return false;
      }
    }
    else if (arg == "index"){
      config.index = statement.at(1);
      Log::trace("Config: Got relative index \"" + config.index + "\"");
    }
    else if (arg == "root"){
      config.root = statement.at(1);
      Log::trace("Config: Got relative root \"" + config.root + "\"");
    }
    else if (arg == "server_name"){
      // Not implemented - don't do anything with it, but don't error
      Log::trace("Config: Got server name (not implemented)");
    }
    else if (arg == "return"){
      // Not implemented - don't do anything with it, but don't error
      Log::trace("Config: Got return (not implemented)");
    }
    else{
      Log::fatal("Config: Unknown server argument: \"" + arg + "\"");
      return false;
    }
  }
  // Valid in location context: try_files
  else if (context == LOCATION_CONTEXT){
    if (arg == "try_files"){
      Log::trace("Config: Got " + name + " with URI \"" + uri + "\"");
      for (int i = 1; i < statement.size() - 1; i++){
        if (statement.at(i) != "=404") // Don't add 404 fallback as a mapping
          process_mapping(statement.at(i));
      }
    }
    else{
      Log::fatal("Config: Unknown location argument: \"" + arg + "\"");
      return false;
    }
  }
  else{ // No valid arguments in http or main context
    Log::fatal("Config: Unexpected argument: \"" + arg +
               "\" in http or main context (expected block)");
    return false;
  }

  statement.clear(); // Reset statement after parsing
  return true;
}

void Config::process_mapping(const std::string& arg){
  // Processes a try_files arg into a mapping, then registers the mapping.

  // Match "$uri" in token and replace with URI for this location, then clean.
  mapping = clean(std::regex_replace(arg, std::regex("\\$uri"), uri));

  Registry::inst().register_mapping(name, uri, mapping);
}

bool Config::validate_config(){
  // Returns true if the config is valid. Called at the end of parse().
  if (config.port == 0)
    return false;
  if (config.index == "")
    return false;
  if (config.root == "")
    return false;
  if (context != MAIN_CONTEXT)
    return false;
  return true; // No errors
}

Config::TokenType Config::get_token(fs::ifstream& cfg_in, std::string& token){
  // Returns next token type, puts contents in token (passed by reference).
  TokenParserState state = INIT_STATE; // DFA state of the token parser
  bool escaped = false; // If true, previous character was escape character

  while (cfg_in.good()){ // While not at end of file
    const char c = cfg_in.get(); // Get next single character from config
    if (!cfg_in.good()){ // End of file reached
      // Reject state: End of file, unterminated quotation
      if (state == SINGLE_QUOTE_STATE || state == DOUBLE_QUOTE_STATE)
        return INVALID;
      else // Accept state: End of file, no unterminated quotation
        return EOF_;
    }

    switch (state){ // Read a character successfully, do something
      // Creating a new token
      case INIT_STATE:
        switch (c){
          case '{': // Accept state: '{' is a complete token
            token = c;
            return BLOCK_START;
          case '}': // Accept state: '}' is a complete token
            token = c;
            return BLOCK_END;
          case ';': // Accept state: ';' is a complete token
            token = c;
            return SEMICOLON;
          case '#': // Create a COMMENT token
            token = c;
            state = COMMENT_STATE; // State change
            continue;
          case '\'': // Create a QUOTE_WORD token with single quotes
            token = c;
            state = SINGLE_QUOTE_STATE; // State change
            continue;
          case '"': // Create a QUOTE_WORD token with double quotes
            token = c;
            state = DOUBLE_QUOTE_STATE; // State change
            continue;
          case ' ': // Ignore whitespaces and newlines
          case '\t':
          case '\r':
          case '\n':
            continue;
          default: // Create a WORD token
            if (c == '\\') // Escape character
              escaped = true; // Set flag, don't add to word
            else // Non-escape character
              token += c; // Add character to word
            state = WORD_STATE; // State change
            continue;
        }

      // Building a COMMENT token
      case COMMENT_STATE:
        // Comment implicitly escapes characters other than newlines
        if (c == '\r' || c == '\n') // Accept state: Newline ends comment
          return COMMENT;
        token += c; // No newline, add character to comment
        continue;

      // Building a QUOTE_WORD token with single quotes
      case SINGLE_QUOTE_STATE:
        if (c == '\\'){ // Escape character
          if (escaped){ // Already escaped
            token += c; // Add character to word
            escaped = false; // Reset flag
          }
          else // Not already escaped
            escaped = true; // Set flag, don't add to word
        }
        else{ // Non-escape character
          token += c; // Add character to word
          if (c == '\''){
            if (!escaped) // Accept state: Single quote termination
              state = END_QUOTE_STATE; // State change
          }
          escaped = false; // Read non-escape character
        }
        continue;

      // Building a QUOTE_WORD token with double quotes
      case DOUBLE_QUOTE_STATE:
        if (c == '\\'){ // Escape character
          if (escaped){ // Already escaped
            token += c; // Add character to word
            escaped = false; // Reset flag
          }
          else // Not already escaped
            escaped = true; // Set flag, don't add to word
        }
        else{ // Non-escape character
          token += c; // Add character to word
          if (c == '"'){
            if (!escaped) // Accept state: Double quote termination
              state = END_QUOTE_STATE; // State change
          }
          escaped = false; // Read non-escape character, reset flag
        }
        continue;

      // Checking for whitespace or semicolon after a QUOTE_WORD token
      case END_QUOTE_STATE:
        // Accept state: Any whitespace ends quoted word
        if (c == ' ' || c == '\r' || c == '\n' || c == '\t')
          return QUOTE_WORD;
        else if (c == ';'){ // Accept state: Semicolon ends quoted word
          cfg_in.unget(); // "Put c back" to process as a separate token
          return QUOTE_WORD;
        }
        else // Reject state: Invalid next character after end quote
          return INVALID; // (must be whitespace or semicolon)
        continue;

      // Building a WORD token
      case WORD_STATE:
        if (c == '\\'){ // Escape character
          if (escaped){ // Already escaped
            token += c; // Add character to word
            escaped = false; // Reset flag
          }
          else // Not already escaped
            escaped = true; // Set flag, don't add to word
        }
        else{ // Non-escape character
          // Accept state: Any whitespace ends word
          if (c == ' ' || c == '\r' || c == '\n' || c == '\t'){
            if (!escaped)
              return WORD;
          }
          // Accept state: Any block start, block end, or semicolon ends a word
          else if (c == '{' || c == '}' || c == ';'){
            if (!escaped){
              cfg_in.unget(); // "Put c back" to process as a separate token
              return WORD;
            }
          }
          token += c; // Add character to word
          escaped = false; // Read non-escape character, reset flag
        }
        continue;
    }
  }
  return EOF_;
}

std::string clean(const std::string& path){
  // Returns a cleaned up version of path with proper '/'s.
  std::string out = "/" + path + "/";
  // Remove duplicate slashes
  out = std::regex_replace(out, std::regex("/+"), "/");
  return out;
}