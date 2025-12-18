#pragma once

#include <string>
#include <vector>

struct LocationBlock{
  enum ModifierType{
    EXACT_MATCH = 0,
    PREFIX_MATCH_STOP = 1,
    REGEX_MATCH = 2,
    NONE = 3
  };

  // Modifier after "location" directive (=, ^~, ~, ~*, none)
  ModifierType modifier = NONE;
  // Stores case sensitivity of regex modifier (~, ~*) if specified
  bool regex_case_sensitive = false;
  // URI of location block, used for prefix match or as regex search term
  std::string uri = "";

  // Can override index and root of containing server block
  std::string index = "";
  std::string root = "";

  /* If this location block specified (optional) try_files directive:
   * - try_files_args stores the relative paths to try.
   * - try_files_fallback stores the last parameter, which is either an
   *   internal redirect URI or a return code (most likely 404).
   **/
  std::vector<std::string> try_files_args;
  std::string try_files_fallback = "";
};