#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fs/sep.h"
#include "fs/basename.h"
#include "fs/getexec.h"
#include "fs/parentpath.h"
#include "os/execve.h"
#include "errors.h"

static const char BIN_DIR[] = PATHSEP_M "bin";
static const char HYPHEN[] = "-";

#if defined(_WIN32)
	#define EXECUTABLE_SUFFIX ".exe"
#else
	#define EXECUTABLE_SUFFIX ""
#endif

static const char* const BINUTILS[] = {
	"ar" EXECUTABLE_SUFFIX,
	"as" EXECUTABLE_SUFFIX,
	"ld" EXECUTABLE_SUFFIX,
	"ld.bfd" EXECUTABLE_SUFFIX,
	"ld.gold" EXECUTABLE_SUFFIX,
	"dwp" EXECUTABLE_SUFFIX,
	"nm" EXECUTABLE_SUFFIX,
	"objcopy" EXECUTABLE_SUFFIX,
	"objdump" EXECUTABLE_SUFFIX,
	"ranlib" EXECUTABLE_SUFFIX,
	"readelf" EXECUTABLE_SUFFIX,
	"strip" EXECUTABLE_SUFFIX
};

static int binutils_matches(const char* const name) {
	
	size_t index = 0;
	const char* value = NULL;
	
	for (index = 0; index < sizeof(BINUTILS) / sizeof(*BINUTILS); index++) {
		value = BINUTILS[index];
		
		if (strcmp(name, value) == 0) {
			return 1;
		}
	}
	
	return 0;
	
}

int main(int argc, char* argv[]) {
	
	int err = ERR_SUCCESS;
	
	char* app_filename = NULL;
	char* parent_directory = NULL;
	const char* file_name = NULL;
	
	const char* prefix = NULL;
	
	char* triplet = NULL;
	
	char* name = NULL;
	char* executable = NULL;
	
	app_filename = get_app_filename();
	
	if (app_filename == NULL) {
		err = ERR_GET_APP_FILENAME_FAILURE;
		goto end;
	}
	
	file_name = basename(app_filename);
	
	parent_directory = malloc(strlen(app_filename) + 1);
	
	if (parent_directory == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	if (!binutils_matches(file_name)) {
		err = ERR_UNKNOWN_BINUTILS_WRAPPER;
		goto end;
	}
	
	name = file_name;
	
	get_parent_path(app_filename, parent_directory, 2);
	file_name = basename(parent_directory);
	
	triplet = malloc(strlen(file_name) + 1);
	
	if (triplet == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(triplet, file_name);
	
	get_parent_path(app_filename, parent_directory, 3);
	
	executable = malloc(strlen(parent_directory) + strlen(BIN_DIR) + strlen(PATHSEP_S) + strlen(triplet) + strlen(HYPHEN) + strlen(name) + 1);
	
	if (executable == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(executable, parent_directory);
	strcat(executable, BIN_DIR);
	strcat(executable, PATHSEP_S);
	strcat(executable, triplet);
	strcat(executable, HYPHEN);
	strcat(executable, name);
	
	if (execute_command(executable, argv) == -1) {
		err = ERR_EXECVE_FAILURE;
		goto end;
	}
	
	end:;
	
	free(triplet);
	free(executable);
	free(parent_directory);
	free(app_filename);
	
	if (err != ERR_SUCCESS) {
		fprintf(stderr, "fatal error: %s\n", obggcc_strerror(err));
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
	
}
