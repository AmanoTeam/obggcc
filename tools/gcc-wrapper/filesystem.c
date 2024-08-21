#include <stdlib.h>
#include <limits.h>

#include <unistd.h>

char* get_app_filename(void) {
	/*
	Returns the filename of the application's executable.
	
	Returns a null pointer on error.
	*/
	
	char* app_filename = malloc(PATH_MAX);
	
	if (app_filename == NULL) {
		return NULL;
	}
	
	if (readlink("/proc/self/exe", app_filename, PATH_MAX) == -1) {
		free(app_filename);
		return NULL;
	}

	return app_filename;
	
}
