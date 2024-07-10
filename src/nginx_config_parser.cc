#include "log.h"
#include "nginx_config_parser.h"
#include "registry.h"

namespace fs = boost::filesystem;

bool Parser::parse(fs::ifstream& cfg_in){
  // Parses the given config file stream.
  // Returns true if parsing was successful, false otherwise.
  // May return false due to invalid token OR invalid transition.
  TokenType prev_type = INIT;
  TokenType token_type = INIT;

  while (true){
    std::string token;
    token_type = get_token(cfg_in, token); // Populates token string

    if (token_type == EOF_) // Reached end of file without errors
      return true;

    Log::trace("ConfigParser: " + type_str(token_type) + " \"" + token + "\"");

    // TODO: Check config file validity, extract necessary information

    if (token_type == INVALID){ // Encountered error while parsing token
      Log::fatal("ConfigParser: Invalid token \"" + token + "\", aborting.");
      return false;
    }

    prev_type = token_type;
  }

  Log::fatal("ConfigParser: Invalid transition from " + type_str(prev_type) +
             " to " + type_str(token_type) + ", aborting.");
  return false;
}

bool Parser::parse(const std::string& file_path){
  // Sets up file stream for the given file path, then calls the config parser.
  // Returns true if parsing was successful, false otherwise.
  // May return false due to a file error OR a parse error. 
  fs::path file_obj(file_path);

  if (!exists(file_obj) || is_directory(file_obj)){ // Nonexistent or directory
    Log::fatal("ConfigParser: " + file_path + " not found, aborting.");
    return false;
  }
  else{ // Non-directory file found
    fs::ifstream fstream(file_obj); // Attempt to open the file
    if (!fstream){ // File exists, but failed to open it for some reason.
      Log::fatal("ConfigParser: " + file_path + " not found, aborting.");
      return false;
    }
    else{ // File opened successfully
      Log::info("ConfigParser: Parsing " + file_path);
      return parse(fstream);
    }
  }
}

Parser::TokenType Parser::get_token(fs::ifstream& cfg_in, std::string& token){
  // Returns next token type, puts contents in token (passed by reference).
  ParserState state = INIT_STATE; // DFA state of the token parser
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

std::string Parser::type_str(TokenType type){
  switch (type){
    case INVALID:     return "INVALID";
    case INIT:        return "INIT";
    case BLOCK_START: return "BLOCK_START";
    case BLOCK_END:   return "BLOCK_END";
    case SEMICOLON:   return "SEMICOLON";
    case COMMENT:     return "COMMENT";
    case WORD:        return "WORD";
    case QUOTE_WORD:  return "QUOTE_WORD";
    case EOF_:        return "EOF";
    default:          return "UNKNOWN";
  }
}