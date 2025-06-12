#if defined(_WIN32)
	#include <ctype.h>
#endif

#include "fs/sep.h"
#include "fs/absoluteness.h"

int isabsolute(const char* const path) {
	
	const unsigned char ch = path[0];
	int status = (ch == PATHSEP);
	
	#if defined(_WIN32)
		status = status || ((isupper(ch) && path[1] == ':' && path[2] == PATHSEP));
	#endif
	
	return status;
	
}

int isrelative(const char* const path) {
	return !isabsolute(path);
}
