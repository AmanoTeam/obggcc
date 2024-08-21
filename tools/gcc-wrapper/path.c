#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "path.h"
#include "pathsep.h"

static int isabsolute(const char* const path) {
	
	const int status = (*path == *PATHSEP);
	return status;
	
}

char* basename(const char* const path) {
	/*
	Returns the final component of a path.
	*/
	
	char* last_comp = (char*) path;
	
	while (1) {
		char* slash_at = strchr(last_comp, PATHSEP[0]);
		
		if (slash_at == NULL) {
			break;
		}
		
		last_comp = slash_at + 1;
	}
	
	return last_comp;
	
}

size_t get_parent_directory(const char* const source, char* const destination, const size_t maxdepth) {
	/*
	Get the parent directory up to the specified "maxdepth" depth of a path.
	
	Returns:
	
	- If the "destination" parameter is NULL, this will return the required size
	  for the buffer (not including the null-terminator).

	- If the "destination" parameter is not NULL, this will return the number of
	  characters written into the buffer (not including the null-terminator).
	*/
	
	size_t index = 0;
	size_t depth = 1;
	size_t wsize = 0;
	
	if (destination != NULL) {
		destination[0] = '\0';
	}
	
	for (index = strlen(source) - 1; index-- > 0;) {
		const char ch = source[index];
		
		if (ch == PATHSEP[0] && depth++ == maxdepth) {
			const size_t size = (size_t) ((source + index) - source);
			
			if (destination != NULL) {
				if (size > 0) {
					memcpy(destination, source, size);
					destination[size] = '\0';
				} else {
					strcat(destination, PATHSEP);
				}
			}
			
			wsize += (size > 0) ? size : 1;
			
			break;
		}
		
		if (index == 0 && isabsolute(source)) {
			const size_t size = 1;
			
			wsize += size;
			
			if (destination != NULL) {
				memcpy(destination, source, size);
				destination[size] = '\0';
			}
			
			break;
		}
	}
	
	return wsize;
	
}
