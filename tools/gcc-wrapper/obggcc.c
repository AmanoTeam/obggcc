#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "obggcc.h"
#include "biggestint.h"
#include "os/envdir.h"
#include "os/time.h"
#include "fs/mkdir.h"
#include "fs/cwd.h"
#include "fs/sep.h"
#include "fstream.h"

void obggcc_print_args(char* args[]) {
	
	size_t index = 0;
	const char* arg = NULL;
	
	char* cwd = NULL;
	
	const unsigned long long int time = get_time();
	
	size_t size = uintlen(time);
	
	char* temp_dir = get_temp_dir();
	char* temp_file = malloc(strlen(temp_dir) + strlen(PATHSEP_S) + 6 + strlen(PATHSEP_S) + size + 1);
	
	strcpy(temp_file, temp_dir);
	strcat(temp_file, PATHSEP_S);
	strcat(temp_file, "obggcc");
	
	create_directory(temp_file);
	
	strcat(temp_file, PATHSEP_S);
	
	snprintf(strchr(temp_file, '\0'), size + 1, "%llu", time);
	
	fstream_t* stream = fstream_open(temp_file, FSTREAM_WRITE);
	
	cwd = get_current_directory();
	
	if (cwd != NULL) {
		fprintf(stderr, "+ cd %s\n", cwd);
		
		size = strlen(cwd);
		
		fstream_write(stream, "+ cd ", 5);
		fstream_write(stream, cwd, size);
		fstream_write(stream, "\n", 1);
	}
	
	fprintf(stderr, "%s", "+ ");
	fstream_write(stream, "+ ", 2);
	
	for (index = 0; 1; index++) {
		arg = args[index];
		
		if (arg == NULL) {
			break;
		}
		
		if (index != 0) {
			fprintf(stderr, "%s", " ");
			fstream_write(stream, " ", 1);
		}
		
		fprintf(stderr, "%s", arg);
		
		size = strlen(arg);
		fstream_write(stream, arg, size);
	}
	
	fprintf(stderr, "%s", "\n");
	fstream_write(stream, "\n", 1);
	
	free(cwd);
	free(temp_file);
	free(temp_dir);
	
	fstream_close(stream);
	
}
