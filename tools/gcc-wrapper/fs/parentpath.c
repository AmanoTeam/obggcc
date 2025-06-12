#include <stddef.h>
#include <string.h>

#include "fs/sep.h"
#include "fs/absoluteness.h"
#include "fs/parentpath.h"

size_t get_parent_path(const char* const source, char* const destination, const size_t maxdepth) {
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
	size_t size = 0;
	
	const size_t len = strlen(source) - 1;
	unsigned char ch = 0;
	
	if (destination != NULL) {
		destination[0] = '\0';
	}
	
	for (index = len; index-- > 0;) {
		ch = source[index];
		
		if (ch == PATHSEP && depth++ == maxdepth) {
			size = (size_t) ((source + index) - source);
			
			if (destination != NULL) {
				if (size > 0) {
					memcpy(destination, source, size);
					destination[size] = '\0';
				} else {
					strcat(destination, PATHSEP_S);
				}
			}
			
			wsize += (size > 0) ? size : 1;
			
			break;
		}
		
		if (index == 0 && isabsolute(source)) {
			#if defined(_WIN32)
				size = 3;
			#else
				size = 1;
			#endif
			
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
