#include "analytics.h"


/// Returns the analytics report for the current session as a string.
std::string Analytics::report(){
  // Get uptime in seconds, convert to d/h/m/s string
  int uptime = std::time(0) - start_time;
  int days = uptime / 86400;
  uptime -= days * 86400;
  int hours = uptime / 3600;
  uptime -= hours * 3600;
  int minutes = uptime / 60;
  uptime -= minutes * 60;
  // Suppresses favicon request, more efficient than sending plain-text
  std::string out = "<!doctype html>"\
    "<html>"\
      "<head>"\
        "<link rel=\"icon\" href=\"data:,\">"\
        "<title>Analytics</title>"\
      "</head>"\
      "<body>"\
        "<pre>";

  out += "Uptime: " + std::to_string(days) + "d " +
         std::to_string(hours) + "h " +
         std::to_string(minutes) + "m " +
         std::to_string(uptime) + "s\n\n";

  out += "Requests served: " + std::to_string(gets + posts + invalid + malicious + health) + "\n" +
         "- " + std::to_string(gets) + " valid (GET)\n" +
         "- " + std::to_string(posts) + " valid (POST)\n" +
         "- " + std::to_string(invalid) + " invalid\n" +
         "- " + std::to_string(malicious) + " malicious\n" +
         "- " + std::to_string(health) + " health checks\n";

  out += "</pre></body></html>";
  return out;
}


/// Returns a static reference to the singleton instance of Analytics.
Analytics& Analytics::inst(){
  static Analytics instRef;
  return instRef;
}