#include "nginx_config.h"


/// Validates the individual server block stored by the Config object.
bool Config::validate(){
	if (port == 0) // If server block doesn't define a port, default to 80.
		port = 80;
	// Server block must define index AND root, or a return directive.
	if ((index == "" || root == "") && (ret == 0))
		return false;
	if (ret / 100 == 3){ // If the return directive is a 3xx status,
		// Valid 3xx statuses are 301, 302, 303, 307, and 308
		if (ret != 301 && ret != 302 && ret != 303 && ret != 307 && ret != 308)
			return false;
		if (ret_val == "") // ret_val is required for 3xx status
			return false;
		else{ // ret_val is defined for 3xx status
			// If ret_val for a 3xx return uses $host, host must be defined.
			if ((ret_val.find("$host") != std::string::npos) && (host == ""))
				return false;
		}
	}
	if (type == HTTP_SERVER){ // If non SSL, must not define SSL parameters
		if (certificate != "" || private_key != "")
			return false;
	}
	else{ // Conversely, if SSL, must define SSL parameters
		if (certificate == "" || private_key == "")
			return false;
	}
  return true; // Validation succeeded
}