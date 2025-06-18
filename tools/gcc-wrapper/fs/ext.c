#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include "fs/basename.h"

char* getext(const char* const filename) {
	/*
	Returns the extension of a filename.
	*/
	
	size_t index = 0;
	size_t size = 0;
	
	unsigned char ch = 0;
	
	char* start = NULL;
	char* tmp = NULL;
	
	const char* last_part = NULL;
	
	last_part = basename(filename);
	start = strstr(last_part, ".");;
	
	if (start == NULL) {
		return NULL;
	}
	
	while (1) {
		tmp = strstr(start + 1, ".");
		
		if (tmp == NULL) {
			break;
		}
		
		start = tmp;
	}
	
	if (start == filename) {
		return NULL;
	}
	
	start++;
	
	if (*start == '\0') {
		return NULL;
	}
	
	size = strlen(start);
	
	for (index = 0; index < size; index++) {
		ch = start[index];
		
		if (!isalnum(ch)) {
			return NULL;
		}
	}
	
	return start;
	
}

char* delext(char* const filename) {
	/*
	Removes extension of the filename.
	*/
	
	char* file_extension = NULL;
	
	while (1) {
		file_extension = getext(filename);
		
		if (file_extension == NULL) {
			break;
		}
		
		*(file_extension - 1) = '\0';
	}
	
	return filename;
	
}
