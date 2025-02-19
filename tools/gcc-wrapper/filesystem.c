#include <stdlib.h>
#include <limits.h>

#include <unistd.h>

char* get_app_filename(void) {
	/*
	Returns the filename of the application's executable.
	
	Returns a null pointer on error.
	*/
	
	ssize_t wsize = 0;
	char* app_filename = malloc(PATH_MAX);
	
	if (app_filename == NULL) {
		return NULL;
	}
	
	wsize = readlink("/proc/self/exe", app_filename, PATH_MAX);
	
	if (wsize == -1 || wsize == PATH_MAX) {
		free(app_filename);
		return NULL;
	}
	
	app_filename[wsize] = '\0';
	
	return app_filename;
	
}
