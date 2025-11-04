#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "obggcc.h"
#include "fs/cwd.h"

void obggcc_print_args(char* args[]) {
	
	size_t index = 0;
	const char* arg = NULL;
	
	char* cwd = NULL;
	
	cwd = get_current_directory();
	
	if (cwd != NULL) {
		fprintf(stderr, "+ cd %s\n", cwd);
	}
	
	fprintf(stderr, "%s", "+ ");
	
	for (index = 0; 1; index++) {
		arg = args[index];
		
		if (arg == NULL) {
			break;
		}
		
		if (index != 0) {
			fprintf(stderr, "%s", " ");
		}
		
		fprintf(stderr, "%s", arg);
	}
	
	fprintf(stderr, "%s", "\n");
	
	free(cwd);
	
}
