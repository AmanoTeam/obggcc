#include <string.h>

#if defined(_WIN32)
	#include <ctype.h>
#endif

#include "fs/sep.h"
#include "fs/stripsep.h"

char* strip_path_sep(char* const s) {
	/*
	Strip the trailing path separator from a string.
	*/
	
	const char* const start = s;
	char* end = strchr(s, '\0');
	
	const unsigned char separator = PATHSEP;
	
	if (start == end) {
		return s;
	}
	
	end--;
	
	while (end != start) {
		const char ch = *end;
		
		if (ch != separator) {
			break;
		}
		
		#if defined(_WIN32)
			if ((size_t) (end - start) == 2 && isalpha(start[0]) && start[1] == ':') {
				break;
			}
		#endif
		
		end--;
	}
	
	end++;
	
	*end = '\0';
	
	return s;
	
}
