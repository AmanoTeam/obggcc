#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>

#include "filesystem.h"
#include "path.h"
#include "pathsep.h"

static const char GCC_MAJOR_VERSION[] = "15";

static const char INCLUDE_DIR[] = "/include";
static const char LIBRARY_DIR[] = "/lib";
static const char GCC_LIBRARY_DIR[] = "/lib/gcc";

#define CPP "c++"
#define GCC "gcc"
#define GPLUSPLUS "g++"

int main(int argc, char* argv[], char* envp[]) {
	
	int status = EXIT_SUCCESS;
	size_t size = 0;
	size_t offset = 0;
	
	char** args = NULL;
	char* arg = NULL;
	
	const char* cc = NULL;
	const char* start = NULL;
	const char* ptr = NULL;
	
	char* executable = NULL;
	
	char* gpp_builtins_include_directory = NULL;
	
	char* gpp_include_directory = NULL;
	char* gcc_include_directory = NULL;
	
	char* sysroot_include_directory = NULL;
	char* sysroot_library_directory = NULL;
	
	char* sysroot_directory = NULL;
	char* parent_directory = NULL;
	
	char* fname = NULL;
	char* app_filename = NULL;
	
	char* triplet = NULL;
	char* glibc_version = NULL;
	
	app_filename = get_app_filename();
	
	if (app_filename == NULL) {
		fprintf(stderr, "fatal error: could not get app filename\n");
		
		status = EXIT_FAILURE;
		goto end;
	}
	
	fname = basename(app_filename);
	
	parent_directory = malloc(strlen(app_filename) + 1);
	
	if (parent_directory == NULL) {
		fprintf(stderr, "fatal error: could not allocate memory\n");
		
		status = EXIT_FAILURE;
		goto end;
	}
	
	get_parent_directory(app_filename, parent_directory, 1);
	
	ptr = strchr(fname, '\0');
	cc = ptr - 3;
	
	if (!(strcmp(cc, GCC) == 0 || strcmp(cc, GPLUSPLUS) == 0)) {
		fprintf(stderr, "fatal error: unknown GCC compiler: %s\n", cc);
		
		status = EXIT_FAILURE;
		goto end;
	}
	
	ptr = fname;
	
	while (1) {
		const unsigned char a = *ptr;
		const unsigned char b = *(ptr + 1);
		
		if (isdigit(a) && b == '.') {
			break;
		}
		
		ptr++;
	}
	
	size = (size_t) (ptr - fname);
	
	triplet = malloc(size + 1);
	
	if (triplet == NULL) {
		fprintf(stderr, "fatal error: could not allocate memory\n");
		
		status = EXIT_FAILURE;
		goto end;
	}
	
	memcpy(triplet, fname, size);
	triplet[size] = '\0';
	
	start = ptr;
	
	while (1) {
		const unsigned char a = *ptr;
		
		if (a == '-') {
			break;
		}
		
		ptr++;
	}
	
	size = (size_t) (ptr - start);
	
	glibc_version = malloc(size + 1);
	
	if (glibc_version == NULL) {
		fprintf(stderr, "fatal error: could not allocate memory\n");
		
		status = EXIT_FAILURE;
		goto end;
	}
	
	memcpy(glibc_version, start, size);
	glibc_version[size] = '\0';
	
	executable = malloc(strlen(parent_directory) + strlen(PATHSEP) + strlen(triplet) + 1 + strlen(cc) + 1);
	
	if (executable == NULL) {
		fprintf(stderr, "fatal error: could not allocate memory\n");
		
		status = EXIT_FAILURE;
		goto end;
	}
	
	strcpy(executable, parent_directory);
	strcat(executable, PATHSEP);
	strcat(executable, triplet);
	strcat(executable, "-");
	strcat(executable, cc);
	
	get_parent_directory(app_filename, parent_directory, 2);
	
	sysroot_directory = malloc(strlen(parent_directory) + strlen(PATHSEP) + strlen(triplet) + strlen(glibc_version) + 1);
	
	if (sysroot_directory == NULL) {
		fprintf(stderr, "fatal error: could not allocate memory\n");
		
		status = EXIT_FAILURE;
		goto end;
	}
	
	strcpy(sysroot_directory, parent_directory);
	strcat(sysroot_directory, PATHSEP);
	strcat(sysroot_directory, triplet);
	strcat(sysroot_directory, glibc_version);
	
	args = malloc(sizeof(char*) * (argc + 13));
	
	if (args == NULL) {
		fprintf(stderr, "fatal error: could not allocate memory\n");
		
		status = EXIT_FAILURE;
		goto end;
	}
	
	sysroot_include_directory = malloc(strlen(sysroot_directory) + strlen(INCLUDE_DIR) + 1);
	
	if (sysroot_include_directory == NULL) {
		fprintf(stderr, "fatal error: could not allocate memory\n");
		
		status = EXIT_FAILURE;
		goto end;
	}
	
	strcpy(sysroot_include_directory, sysroot_directory);
	strcat(sysroot_include_directory, INCLUDE_DIR);
	
	sysroot_library_directory = malloc(strlen(sysroot_directory) + strlen(LIBRARY_DIR) + 1);
	
	if (sysroot_library_directory == NULL) {
		fprintf(stderr, "fatal error: could not allocate memory\n");
		
		status = EXIT_FAILURE;
		goto end;
	}
	
	strcpy(sysroot_library_directory, sysroot_directory);
	strcat(sysroot_library_directory, LIBRARY_DIR);
	
	gcc_include_directory = malloc(strlen(parent_directory) + strlen(GCC_LIBRARY_DIR) + strlen(PATHSEP) + strlen(triplet) + strlen(PATHSEP) + strlen(GCC_MAJOR_VERSION) + strlen(INCLUDE_DIR) + 1);
	
	if (gcc_include_directory == NULL) {
		fprintf(stderr, "fatal error: could not allocate memory\n");
		
		status = EXIT_FAILURE;
		goto end;
	}
	
	strcpy(gcc_include_directory, parent_directory);
	strcat(gcc_include_directory, GCC_LIBRARY_DIR);
	strcat(gcc_include_directory, PATHSEP);
	strcat(gcc_include_directory, triplet);
	strcat(gcc_include_directory, PATHSEP);
	strcat(gcc_include_directory, GCC_MAJOR_VERSION);
	strcat(gcc_include_directory, INCLUDE_DIR);
	
	gpp_include_directory = malloc(strlen(parent_directory) + strlen(PATHSEP) + strlen(triplet) + strlen(INCLUDE_DIR) + strlen(PATHSEP) + strlen(CPP) + strlen(PATHSEP) + strlen(GCC_MAJOR_VERSION) + 1);
	
	if (gpp_include_directory == NULL) {
		fprintf(stderr, "fatal error: could not allocate memory\n");
		
		status = EXIT_FAILURE;
		goto end;
	}
	
	strcpy(gpp_include_directory, parent_directory);
	strcat(gpp_include_directory, PATHSEP);
	strcat(gpp_include_directory, triplet);
	strcat(gpp_include_directory, INCLUDE_DIR);
	strcat(gpp_include_directory, PATHSEP);
	strcat(gpp_include_directory, CPP);
	strcat(gpp_include_directory, PATHSEP);
	strcat(gpp_include_directory, GCC_MAJOR_VERSION);
	
	gpp_builtins_include_directory = malloc(strlen(gpp_include_directory) + strlen(PATHSEP) + strlen(triplet) + 1);
	
	if (gpp_builtins_include_directory == NULL) {
		fprintf(stderr, "fatal error: could not allocate memory\n");
		
		status = EXIT_FAILURE;
		goto end;
	}
	
	strcpy(gpp_builtins_include_directory, gpp_include_directory);
	strcat(gpp_builtins_include_directory, PATHSEP);
	strcat(gpp_builtins_include_directory, triplet);
	
	arg = malloc(10 + strlen(sysroot_directory) + 1);
	
	if (arg == NULL) {
		fprintf(stderr, "fatal error: could not allocate memory\n");
		
		status = EXIT_FAILURE;
		goto end;
	}
	
	strcpy(arg, "--sysroot=");
	strcat(arg, sysroot_directory);
	
	args[offset++] = executable;
	args[offset++] = arg;
	args[offset++] = "--no-standard-includes";
	
	if (strcmp(cc, GPLUSPLUS) == 0) {
		args[offset++] = "-isystem";
		args[offset++] = gpp_include_directory;
		
		args[offset++] = "-isystem";
		args[offset++] = gpp_builtins_include_directory;
	}
	
	args[offset++] = "-isystem";
	args[offset++] = gcc_include_directory;
	args[offset++] = "-isystem";
	args[offset++] = sysroot_include_directory;
	args[offset++] = "-L";
	args[offset++] = sysroot_library_directory;
	
	memcpy(&args[offset], &argv[1], argc * sizeof(*argv));
	
	if (execve(executable, args, envp) == -1) {
		status = EXIT_FAILURE;
		goto end;
	}
	
	end:;
	
	free(triplet);
	free(glibc_version);
	free(executable);
	free(sysroot_directory);
	free(app_filename);
	free(args);
	free(arg);
	free(sysroot_include_directory);
	free(sysroot_library_directory);
	free(gcc_include_directory);
	free(gpp_include_directory);
	free(gpp_builtins_include_directory);
	
	return status;
	
}

