#include <stddef.h>
#include <string.h>

#include "fs/sep.h"
#include "fs/basename.h"

char* basename(const char* const path) {
	/*
	Returns the final component of a path.
	*/
	
	char* last_comp = (char*) path;
	
	while (1) {
		char* slash_at = strchr(last_comp, PATHSEP);
		
		if (slash_at == NULL) {
			break;
		}
		
		last_comp = slash_at + 1;
	}
	
	return last_comp;
	
}
