#include <boost/algorithm/string/replace.hpp> // replace_all
#include <boost/filesystem.hpp> // exists, is_directory, path
#include <boost/lexical_cast.hpp> // lexical_cast
#include <regex> // regex, regex_replace

#include "log.h"
#include "nginx_config_parser.h"
#include "registry.h" // Registry::inst()

// Standardized log prefix for this source
#define LOG_PRE "[Config]   "

namespace fs = boost::filesystem;


/// Returns a static reference to the singleton instance of ConfigParser.
ConfigParser& ConfigParser::inst(){
  static ConfigParser instRef;
  return instRef;
}


/// Returns the parsed Config objects.
std::vector<Config*> ConfigParser::configs(){
  return configs_;
}


/// Sets the working directory for conversion of relative paths.
void ConfigParser::set_working_directory(const std::string& cwd){
  cwd_ = cwd;
}


/// Parses the specified config file and populates ConfigParser.configs_.
bool ConfigParser::parse(const std::string& file_path){
  fs::path file_obj(file_path);

  if (exists(file_obj) && !is_directory(file_obj)){ // Non-directory file found
    fs::ifstream fstream(file_obj); // Attempt to open the file
    if (fstream){ // File opened successfully
      // Log::trace(LOG_PRE, "Parsing " + file_path);
      return parse(fstream);
    }
    else{ // File exists, but failed to open it for some reason.
      Log::fatal(LOG_PRE, file_path + " not found, aborting.");
      return false;
    }
  }
  else{ // Nonexistent or directory
    Log::fatal(LOG_PRE, file_path + " not found, aborting.");
    return false;
  }
}


/// Parses the config pointed to by cfg_in and populates ConfigParser.configs_.
bool ConfigParser::parse(fs::ifstream& cfg_in){
  TokenType prev_type = INIT;
  TokenType token_type = INIT;
  std::vector<std::string> statement;
  std::string token;

  while (true){
    token = "";
    token_type = get_token(cfg_in, token); // Populates token string

    if (token_type == INVALID){ // Encountered error while parsing token
      Log::fatal(LOG_PRE, "Invalid token \"" + token + "\", aborting.");
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
      /* In most contexts, SEMICOLON or BLOCK_END must precede BLOCK_END.
         BLOCK_START preceding BLOCK_END is valid in location block context. */
      if (prev_type == BLOCK_START || prev_type == SEMICOLON || prev_type == BLOCK_END){
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
      if (prev_type == BLOCK_END) // BLOCK_END must precede EOF
        return validate_config(); // If true, valid config file structure
      else // Invalid config file structure
        break;
    }
  }

  Log::fatal(LOG_PRE, "Invalid transition to token \"" + token + "\".");
  return false;
}


/// Parses a block start within the config. Returns bool success status.
bool ConfigParser::parse_block_start(std::vector<std::string>& statement){
  std::string new_context = statement.at(0); // First token is target context

  // Verify that the block start statement has a valid size
  if (new_context == "http" || new_context == "server"){
    // http/server block start contains 2 tokens (e.g., "http {", "server {")
    if (statement.size() != 2){
      Log::fatal(LOG_PRE, "Malformed " + new_context + " block (size " +
                 std::to_string(statement.size()) + ", expected size 2)");
      return false;
    }
  }
  else if (new_context == "location"){
    // Location block start contains 4 tokens if modifier present, else 3
    if (statement.size() != 3 && statement.size() != 4){
      Log::fatal(LOG_PRE, "Malformed location block (size " +
                 std::to_string(statement.size()) + ", expected size 3 or 4)");
      return false;
    }
  }
  else{
    Log::fatal(LOG_PRE, "Unknown target context " + new_context);
    return false;
  }

  // Verify and perform context transition
  if (context == MAIN_CONTEXT && new_context == "http")
    context = HTTP_CONTEXT;
  else if (context == HTTP_CONTEXT && new_context == "server"){
    // Starting to parse a server block, initialize cur_config
    cur_config = new Config();
    context = SERVER_CONTEXT;
  }
  else if (context == SERVER_CONTEXT && new_context == "location"){
    context = LOCATION_CONTEXT;
    if (statement.size() == 3){ // No modifier (e.g., "location [URI] {")
      cur_location_block = new LocationBlock();
      cur_location_block->modifier = LocationBlock::ModifierType::NONE; // 3
      cur_location_block->uri = statement.at(1); // Set URI
      Log::trace(LOG_PRE, "Parsing a new location block: location " + statement.at(1) + " {");
    }
    else{ // Modifier present (e.g., "location [modifier] [URI] {")
      cur_location_block = new LocationBlock();
      std::string modifier = statement.at(1);

      if (modifier == "=") // Exact match
        cur_location_block->modifier = LocationBlock::ModifierType::EXACT_MATCH; // 0
      else if (modifier == "^~") // Prefix match stop
        cur_location_block->modifier = LocationBlock::ModifierType::PREFIX_MATCH_STOP; // 1
      else if (modifier == "~"){ // Case-sensitive regex
        cur_location_block->modifier = LocationBlock::ModifierType::REGEX_MATCH; // 2
        cur_location_block->regex_case_sensitive = true;
      }
      else if (modifier == "~*") // Case-insensitive regex
        cur_location_block->modifier = LocationBlock::ModifierType::REGEX_MATCH; // 2
      else{ // Invalid modifier
        Log::fatal(LOG_PRE, "Invalid location block modifier: " + modifier);
        return false;
      }

      cur_location_block->uri = statement.at(2); // Set URI
      Log::trace(LOG_PRE, "Parsing a new location block: location " + modifier + " " + statement.at(2) + " {");
    }
  }
  else{
    Log::fatal(LOG_PRE, "Invalid context transition to " + new_context);
    return false;
  }

  statement.clear(); // Reset statement after parsing
  return true;
}


/// Parses a block end within the config. Returns bool success status.
bool ConfigParser::parse_block_end(std::vector<std::string>& statement){
  /* No need to check size, any valid preceding tokens also call parse routines
     so it is impossible for statement to have size > 1. */

  // Verify and perform context transition
  if (context == LOCATION_CONTEXT){ // Finished parsing a location block
    cur_config->locations[cur_location_block->modifier].push_back(cur_location_block);
    context = SERVER_CONTEXT;
  }
  else if (context == SERVER_CONTEXT){ // Finished parsing a server block
    if (cur_config->validate()) // If valid, push cur_config to configs_ vector
      configs_.push_back(cur_config);
    else{ // If invalid, return false
      Log::fatal(LOG_PRE, "Parsed server block failed validation");
      return false; // Will cause parse() to return false
    }
    context = HTTP_CONTEXT;
  }
  else if (context == HTTP_CONTEXT)
    context = MAIN_CONTEXT;
  else{
    Log::fatal(LOG_PRE, "Invalid context exit from main; "
               "check number of closing brackets");
    return false;
  }

  statement.clear(); // Reset statement after parsing
  return true;
}


/// Parses a general statement within the config. Returns bool success status.
bool ConfigParser::parse_statement(std::vector<std::string>& statement){
  std::string arg = statement.at(0); // First token is argument type

  /* Valid in server context: listen, index, root, server_name, return,
     ssl_certificate, ssl_certificate_key, ssl_protocols, ssl_ciphers,
     ssl_session_timeout */
  if (context == SERVER_CONTEXT){
    if (arg == "listen"){
      try{
        cur_config->port = boost::lexical_cast<unsigned short>(statement.at(1));
        Log::trace(LOG_PRE, "Got port " + std::to_string(cur_config->port));
      }
      catch(boost::bad_lexical_cast&){ // Out of range, not a number, etc.
        Log::fatal(LOG_PRE, "Invalid port \"" + statement.at(1) + "\"");
        return false;
      }
      if (statement.size() == 3) // e.g., listen 80;
        cur_config->type = Config::ServerType::HTTP_SERVER;
      if (statement.size() == 4){ // e.g., listen 443 ssl;
        if (statement.at(2) == "ssl")
          cur_config->type = Config::ServerType::HTTPS_SERVER;
        else{
          Log::fatal(LOG_PRE, "Invalid argument after port: \"" + statement.at(2) + "\"");
          return false;
        }
      }
    }
    else if (arg == "index"){
      cur_config->index = clean(statement.at(1), FILE_URI);
      Log::trace(LOG_PRE, "Got relative index \"" + cur_config->index + "\"");
    }
    else if (arg == "root"){
      cur_config->root = clean(statement.at(1), DIR_ONLY);
      Log::trace(LOG_PRE, "Got root \"" + cur_config->root + "\"");
    }
    else if (arg == "server_name"){
      cur_config->host = clean(statement.at(1), FILE_URI);
      Log::trace(LOG_PRE, "Got server name " + cur_config->host);
    }
    else if (arg == "return"){
      try{
        cur_config->ret = boost::lexical_cast<short>(statement.at(1));
        Log::trace(LOG_PRE, "Got ret " + std::to_string(cur_config->ret));
      }
      catch(boost::bad_lexical_cast&){ // Out of range, not a number, etc.
        if (statement.size() == 3){ // e.g., return https://$host$request_uri;
          cur_config->ret = 302; // Default for return with only URL provided
          cur_config->ret_val = statement.at(1);
          Log::trace(LOG_PRE, "Got default ret " + std::to_string(cur_config->ret));
          Log::trace(LOG_PRE, "Got ret_val " + cur_config->ret_val);
        }
        else{
          Log::fatal(LOG_PRE, "Invalid ret \"" + statement.at(1) + "\"");
          return false;
        }
      }
      if (statement.size() == 4){ // e.g., return 301 https://$host$request_uri;
        cur_config->ret_val = clean(statement.at(2), FILE_URI);
        Log::trace(LOG_PRE, "Got ret_val " + cur_config->ret_val);
      }
    }
    else if (arg == "ssl_certificate"){
      cur_config->certificate = clean(statement.at(1), DIR_FILE);
      Log::trace(LOG_PRE, "Got ssl_certificate " + cur_config->certificate);
    }
    else if (arg == "ssl_certificate_key"){
      cur_config->private_key = clean(statement.at(1), DIR_FILE);
      Log::trace(LOG_PRE, "Got ssl_certificate_key " + cur_config->private_key);
    }
    else if (arg == "ssl_protocols"){
      // Not implemented - don't do anything with it, but don't error
      Log::trace(LOG_PRE, "Got ssl_protocols (not implemented)");
    }
    else if (arg == "ssl_ciphers"){
      // Not implemented - don't do anything with it, but don't error
      Log::trace(LOG_PRE, "Got ssl_ciphers (not implemented)");
    }
    else if (arg == "ssl_session_timeout"){
      // Not implemented - don't do anything with it, but don't error
      Log::trace(LOG_PRE, "Got ssl_session_timeout (not implemented)");
    }
    else{
      Log::fatal(LOG_PRE, "Unknown server argument: \"" + arg + "\"");
      return false;
    }
  }
  // Valid in location context: index, root, try_files
  else if (context == LOCATION_CONTEXT){
    if (arg == "index"){ // Statement size 3+ (e.g., "index index.html ;")
      cur_location_block->index = clean(statement.at(1), FILE_URI);
      Log::trace(LOG_PRE, "Got relative index override \"" + cur_location_block->index + "\"");
    }
    else if (arg == "root"){ // Statement size 3 (e.g., "root /path ;")
      cur_location_block->root = clean(statement.at(1), DIR_ONLY);
      Log::trace(LOG_PRE, "Got root override \"" + cur_location_block->root + "\"");
    }
    else if (arg == "try_files"){ // Statement size 4+ (e.g., "try_files $uri =404 ;")
      // Process parameters; exclude "try_files", fallback (last) argument, ;
      for (int i = 1; i < statement.size() - 2; i++){
        /* Each token represents a relative path to try. Match "$uri" variable
           and replace with location URI, then store the cleaned rel path. */
        std::string rel_path = clean(std::regex_replace(statement.at(i), std::regex("\\$uri"), cur_location_block->uri), FILE_URI);
        cur_location_block->try_files_args.push_back(rel_path);
        Log::trace(LOG_PRE, "try_files mapped relative path \"" + rel_path + "\" to location block with URI \"" + cur_location_block->uri + "\".");
      }
      // Last try_files parameter is always the fallback
      cur_location_block->try_files_fallback = statement.at(statement.size() - 2);
    }
    else{
      Log::fatal(LOG_PRE, "Unknown location argument: \"" + arg + "\"");
      return false;
    }
  }
  else{ // No valid arguments in http or main context
    Log::fatal(LOG_PRE, "Unexpected argument: \"" + arg +
               "\" in http or main context (expected block)");
    return false;
  }

  statement.clear(); // Reset statement after parsing
  return true;
}


/// Validates the contents of the parsed configs. Returns bool success status.
bool ConfigParser::validate_config(){
  if (context != MAIN_CONTEXT) // The config must end in MAIN_CONTEXT.
    return false;
  // At least one config must define index and root.
  for (Config* config : configs_){
    if (config->index != "" && config->root != ""){
      Log::info(LOG_PRE, "Parsed and validated " +
                std::to_string(configs_.size()) + " config(s)");
      return true;
    }
  }

  Log::fatal(LOG_PRE, "No config defined index and root");
  return false;
}


/** 
 * Resolves relative paths and ensures proper structure of the given path.
 * 
 * @param path A string containing the path to clean.
 * @param is_dir If true, path is a directory (e.g., root).
 *   If false, path is a file (e.g., index).
 * @returns A string containing the cleaned version of path.
 */
std::string ConfigParser::clean(const std::string& path, PathType type){
  std::string out = path;
  boost::replace_all(out, "\\\\", "\\"); // Resolve escaped backslashes
  boost::replace_all(out, "\\\"", "\""); // Resolve escaped quotation marks
  boost::replace_all(out, "\\\'", "\'"); // Resolve escaped quotation marks
  
  if (type != FILE_URI){ // Ensure correct structure for directory path
    if (out[0] != '/') // Resolve relative path
      // Prepend cwd/ to path starting with ./, result cwd/./(rest of path)
      // Prepend cwd/ to path starting with ../, result cwd/../(rest of path)
      // Prepend cwd/ to path starting with neither, result cwd/(rest of path)
      out = cwd_ + "/" + out; // cwd_ always has a leading slash

    if (type == DIR_ONLY)
      out += "/"; // DIR_ONLY should have both leading and trailing slash
    out = std::regex_replace(out, std::regex("/+"), "/"); // Remove duplicates
  }

  /* Remove meaningless "./" in the file path, but avoid matching "../". Regex
     matches "(./)+" with any preceding character other than '.', and replaces
     the sequence with the preceding character (capture group 1). */
  out = std::regex_replace(out, std::regex(R"(([^\.])(\.\/)+)"), R"($1)");
  return out;
}


/** 
 * Parses the next token in the config.
 * 
 * @param[in] cfg_in A file stream pointing to the config being parsed.
 * @param[out] token A string containing the contents of the parsed token.
 * @returns The type of the parsed token.
 */
ConfigParser::TokenType ConfigParser::get_token(fs::ifstream& cfg_in, std::string& token){
  TokenParserState state = INIT_STATE; // DFA state of the token parser
  bool escaped = false; // If true, previous character was escape character

  while (true){ // While not at end of file
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
}