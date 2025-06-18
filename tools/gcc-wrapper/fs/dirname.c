#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "fs/stripsep.h"
#include "fs/dirname.h"
#include "fs/basename.h"

char* dirname(const char* const path) {
	/*
	Returns the directory name of a path.
	*/
	
	char* directory = NULL;
	
	const char* const name = basename(path);
	const size_t size = (size_t) (name - path);
	
	directory = malloc(size + 1);
	
	if (directory == NULL) {
		return NULL;
	}
	
	if (size > 0) {
		memcpy(directory, path, size);
	}
	
	directory[size] = '\0';
	
	strip_path_sep(directory);
	
	return directory;
	
}
