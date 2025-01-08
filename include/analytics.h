#pragma once

#include <ctime>
#include <string>

class Analytics final{ // Singleton class (only one instance)
public:
  // Deleting the copy and assignment operators due to being a singleton class
  Analytics(const Analytics&) = delete;
  Analytics& operator=(const Analytics&) = delete;

  /// Returns the analytics report for the current session as a string.
  std::string report();

  /// Returns a static reference to the singleton instance of Registry.
  static Analytics& inst();

  int gets = 0;
  int posts = 0;
  int invalid = 0;
  int malicious = 0;
  int health = 0;

private:
  Analytics(){}; // Making constructor private due to being a singleton class
  std::time_t start_time = std::time(0);
};