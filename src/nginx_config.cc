#include "nginx_config.h"


/// Validates the individual server block stored by the Config object.
bool Config::validate(){
	if (port == 0) // Server block must define a port.
		return false;
	// Server block must define a redirect (ret and ret_uri), or index and root.
	if ((index == "" || root == "") && (ret == 0 || ret_uri == ""))
		return false;
  return true; // Validation succeeded
}