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

static const char RT_LIBRARY[] = "rt";
static const char STDCXX_LIBRARY[] = "stdc++";

static const char GCC_OPT_ISYSTEM[] = "-isystem";
static const char GCC_OPT_SYSROOT[] = "--sysroot=";
static const char GCC_OPT_NOSTDINC[] = "--no-standard-includes";
static const char GCC_OPT_LIBDIR[] = "-L";
static const char GCC_OPT_STATIC_LIBCXX[] = "-static-libstdc++";
static const char GCC_OPT_L[] = "-l";
static const char GCC_OPT_I[] = "-I";
static const char GCC_OPT_L_RT[] = "-lrt";
static const char GCC_OPT_L_STDCXX[] = "-lstdc++";
static const char GCC_OPT_XLINKER[] = "-Xlinker";
static const char LD_OPT_RPATH_LINK[] = "-rpath-link";
static const char LD_OPT_UNRESOLVED_SYMBOLS[] = "--unresolved-symbols=ignore-in-shared-libs";

static const char* const SYSTEM_LIBRARY_PATH[] = {
	"/usr/local/lib64",
	"/usr/local/lib",
	"/lib64",
	"/lib",
	"/usr/lib64",
	"/usr/lib"
};

static const char SYSTEM_INCLUDE_PATH[] = "/usr/include";

#define ERR_SUCCESS 0
#define ERR_MEMORY_ALLOCATE_FAILURE -1
#define ERR_UNKNOWN_COMPILER -2
#define ERR_GET_APP_FILENAME_FAILURE -3
#define ERR_EXECVE_FAILURE -4

#define CPP "c++"
#define GCC "gcc"
#define GPLUSPLUS "g++"

int main(int argc, char* argv[], char* envp[]) {
	
	int err = ERR_SUCCESS;
	
	size_t size = 0;
	size_t offset = 0;
	size_t index = 0;
	
	long int glibc_version_major = 0;
	long int glibc_version_minor = 0;
	
	int wants_system_libraries = 0;
	
	int wants_libcxx = 0;
	int wants_rt_library = 0;
	
	int have_rt_library = 0;
	
	char** args = NULL;
	char* arg = NULL;
	
	const char* cc = NULL;
	const char* start = NULL;
	char* ptr = NULL;
	const char* opt = NULL;
	
	const char* prev = NULL;
	const char* cur = NULL;
	
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
	
	wants_system_libraries = getenv("OBGGCC_WANTS_SYSTEM_LIBRARIES") != NULL;
	
	for (index = 0; index < argc; index++) {
		cur = argv[index];
		
		if (strcmp(cur, GCC_OPT_STATIC_LIBCXX) == 0 || strcmp(cur, GCC_OPT_L_STDCXX) == 0) {
			wants_libcxx = 1;
		} else if (strcmp(cur, GCC_OPT_L_RT) == 0) {
			have_rt_library = 1;
		} else if (prev != NULL && strcmp(prev, GCC_OPT_L) == 0) {
			if (strcmp(cur, STDCXX_LIBRARY) == 0) {
				wants_libcxx = 1;
			} else if (strcmp(cur, RT_LIBRARY) == 0) {
				have_rt_library = 1;
			}
		}
		
		prev = cur;
	}
	
	app_filename = get_app_filename();
	
	if (app_filename == NULL) {
		err = ERR_GET_APP_FILENAME_FAILURE;
		goto end;
	}
	
	fname = basename(app_filename);
	
	parent_directory = malloc(strlen(app_filename) + 1);
	
	if (parent_directory == NULL) {
		err = ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	get_parent_directory(app_filename, parent_directory, 1);
	
	ptr = strchr(fname, '\0');
	cc = ptr - 3;
	
	if (!(strcmp(cc, GCC) == 0 || strcmp(cc, GPLUSPLUS) == 0)) {
		err = ERR_UNKNOWN_COMPILER;
		goto end;
	}
	
	wants_libcxx += (strcmp(cc, GPLUSPLUS) == 0);
	
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
		err = ERR_MEMORY_ALLOCATE_FAILURE;
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
		err = ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	memcpy(glibc_version, start, size);
	glibc_version[size] = '\0';
	
	glibc_version_major = strtol(glibc_version, &ptr, 16);
	
	ptr++;
	
	glibc_version_minor = strtol(ptr, NULL, 16);
	
	/* Determine whether we need to implicit link with -lrt */
	wants_rt_library = wants_libcxx && !have_rt_library && (glibc_version_major == 2 && glibc_version_minor < 17);
	
	executable = malloc(strlen(parent_directory) + strlen(PATHSEP) + strlen(triplet) + 1 + strlen(cc) + 1);
	
	if (executable == NULL) {
		err = ERR_MEMORY_ALLOCATE_FAILURE;
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
		err = ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	strcpy(sysroot_directory, parent_directory);
	strcat(sysroot_directory, PATHSEP);
	strcat(sysroot_directory, triplet);
	strcat(sysroot_directory, glibc_version);
	
	size = argc + 13 + wants_rt_library;
	
	if (wants_system_libraries) {
		size += (sizeof(SYSTEM_LIBRARY_PATH) / sizeof(*SYSTEM_LIBRARY_PATH)) * 6;
		size += 4;
	}
	
	args = malloc(size * sizeof(char*));
	
	if (args == NULL) {
		err = ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	sysroot_include_directory = malloc(strlen(sysroot_directory) + strlen(INCLUDE_DIR) + 1);
	
	if (sysroot_include_directory == NULL) {
		err = ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	strcpy(sysroot_include_directory, sysroot_directory);
	strcat(sysroot_include_directory, INCLUDE_DIR);
	
	sysroot_library_directory = malloc(strlen(sysroot_directory) + strlen(LIBRARY_DIR) + 1);
	
	if (sysroot_library_directory == NULL) {
		err = ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	strcpy(sysroot_library_directory, sysroot_directory);
	strcat(sysroot_library_directory, LIBRARY_DIR);
	
	gcc_include_directory = malloc(strlen(parent_directory) + strlen(GCC_LIBRARY_DIR) + strlen(PATHSEP) + strlen(triplet) + strlen(PATHSEP) + strlen(GCC_MAJOR_VERSION) + strlen(INCLUDE_DIR) + 1);
	
	if (gcc_include_directory == NULL) {
		err = ERR_MEMORY_ALLOCATE_FAILURE;
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
		err = ERR_MEMORY_ALLOCATE_FAILURE;
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
		err = ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	strcpy(gpp_builtins_include_directory, gpp_include_directory);
	strcat(gpp_builtins_include_directory, PATHSEP);
	strcat(gpp_builtins_include_directory, triplet);
	
	arg = malloc(strlen(GCC_OPT_SYSROOT) + strlen(sysroot_directory) + 1);
	
	if (arg == NULL) {
		err = ERR_MEMORY_ALLOCATE_FAILURE;
		goto end;
	}
	
	strcpy(arg, GCC_OPT_SYSROOT);
	strcat(arg, sysroot_directory);
	
	args[offset++] = executable;
	args[offset++] = arg;
	args[offset++] = (char*) GCC_OPT_NOSTDINC;
	
	if (strcmp(cc, GPLUSPLUS) == 0) {
		args[offset++] = (char*) GCC_OPT_ISYSTEM;
		args[offset++] = gpp_include_directory;
		
		args[offset++] = (char*) GCC_OPT_ISYSTEM;
		args[offset++] = gpp_builtins_include_directory;
	}
	
	args[offset++] = (char*) GCC_OPT_ISYSTEM;
	args[offset++] = gcc_include_directory;
	args[offset++] = (char*) GCC_OPT_ISYSTEM;
	args[offset++] = sysroot_include_directory;
	args[offset++] = (char*) GCC_OPT_LIBDIR;
	args[offset++] = sysroot_library_directory;
	
	if (wants_rt_library) {
		fprintf(stderr, "warning: implicit linking with %s due to GLIBC < 2.17 requirement\n", GCC_OPT_L_RT);
		args[offset++] = (char*) GCC_OPT_L_RT;
	}
	
	if (wants_system_libraries) {
		fprintf(stderr, "warning: linking with system libraries is untested and may result in broken binaries\n");
		
		args[offset++] = (char*) GCC_OPT_I;
		args[offset++] = (char*) SYSTEM_INCLUDE_PATH;
		
		args[offset++] = (char*) GCC_OPT_XLINKER;
		args[offset++] = (char*) LD_OPT_UNRESOLVED_SYMBOLS;
		
		for (index = 0; index < sizeof(SYSTEM_LIBRARY_PATH) / sizeof(*SYSTEM_LIBRARY_PATH); index++) {
			cur = SYSTEM_LIBRARY_PATH[index];
			
			args[offset++] = (char*) GCC_OPT_LIBDIR;
			args[offset++] = (char*) cur;
			
			args[offset++] = (char*) GCC_OPT_XLINKER;
			args[offset++] = (char*) LD_OPT_RPATH_LINK;
			
			args[offset++] = (char*) GCC_OPT_XLINKER;
			args[offset++] = (char*) cur;
		}
	}
	
	memcpy(&args[offset], &argv[1], argc * sizeof(*argv));
	
	if (execve(executable, args, envp) == -1) {
		err = ERR_EXECVE_FAILURE;
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
	
	switch (err) {
		case ERR_SUCCESS:
			opt = "Success";
		case ERR_MEMORY_ALLOCATE_FAILURE:
			opt = "Could not allocate memory";
		case ERR_UNKNOWN_COMPILER:
			opt = "Unknown GCC compiler";
		case ERR_GET_APP_FILENAME_FAILURE:
			opt = "Could not get app filename";
		case ERR_EXECVE_FAILURE:
			opt = "Call to execve failed";
	}
	
	if (err != ERR_SUCCESS) {
		fprintf(stderr, "fatal error: %s\n", opt);
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
	
}
