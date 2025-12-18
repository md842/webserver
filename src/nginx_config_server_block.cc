#include "nginx_config_server_block.h"


/// Validates the individual server block stored by the Config object.
bool Config::validate(){
	if (ret / 100 == 3){ // If 3xx return directive specified,
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

	// Ensure each location block within this server block defines root and index
	for (int i = 0; i < 4; i++){ // For all 4 location block types
		for (LocationBlock* location : locations[i]){
			if (location->root == "") // No root directive in location block
				location->root = root; // Use server block root value
			if (location->index == "") // No index directive in location block
				location->index = index; // Use server block index value
		}
	}

  return true; // Validation succeeded
}