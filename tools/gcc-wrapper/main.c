#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>

#include "fs/getexec.h"
#include "fs/parentpath.h"
#include "fs/sep.h"
#include "fs/basename.h"

static const char GCC_MAJOR_VERSION[] = "15";

static const char INCLUDE_DIR[] = PATHSEP_M "include";
static const char INCLUDE_MISSING_DIR[] = PATHSEP_M "include-missing";
static const char LIBRARY_DIR[] = PATHSEP_M "lib";
static const char GCC_LIBRARY_DIR[] = PATHSEP_M "lib" PATHSEP_M "gcc";

static const char RT_LIBRARY[] = "rt";
static const char STDCXX_LIBRARY[] = "stdc++";

static const char GCC_OPT_ISYSTEM[] = "-isystem";
static const char GCC_OPT_SYSROOT[] = "--sysroot=";
static const char GCC_OPT_NOSTDINC[] = "--no-standard-includes";
static const char GCC_OPT_LIBDIR[] = "-L";
static const char GCC_OPT_STATIC_LIBCXX[] = "-static-libstdc++";
static const char GCC_OPT_L[] = "-l";
static const char GCC_OPT_V[] = "-v";
static const char GCC_OPT_D[] = "-D";
static const char GCC_OPT_L_RT[] = "-lrt";
static const char GCC_OPT_FSANITIZE[] = "-fsanitize=";
static const char GCC_OPT_L_STDCXX[] = "-lstdc++";
static const char GCC_OPT_L_ATOMIC[] = "-latomic";
static const char GCC_OPT_XLINKER[] = "-Xlinker";
static const char LD_OPT_DYNAMIC_LINKER[] = "-dynamic-linker";
static const char LD_OPT_RPATH_LINK[] = "-rpath-link";
static const char LD_OPT_RPATH[] = "-rpath";
static const char LD_OPT_UNRESOLVED_SYMBOLS[] = "--unresolved-symbols=ignore-in-shared-libs";

static const char M_ANDROID_API[] = "__ANDROID_API__=";

static char* SYSTEM_LIBRARY_PATH[] = {
	PATHSEP_M "usr" PATHSEP_M "local" PATHSEP_M "lib64",
	PATHSEP_M "usr" PATHSEP_M "local" PATHSEP_M "lib",
	PATHSEP_M "lib64",
	PATHSEP_M "lib",
	PATHSEP_M "usr" PATHSEP_M "lib64",
	PATHSEP_M "usr" PATHSEP_M "lib",
	NULL,
	NULL
};

static const char USR_DIRECTORY[] = PATHSEP_M "usr";
static const char LIB_DIRECTORY[] = PATHSEP_M "lib";

static const char NZ_SYSROOT[] = PATHSEP_M "lib" PATHSEP_M "nouzen" PATHSEP_M "sysroot";

static const char SYSTEM_INCLUDE_PATH[] = PATHSEP_M "usr" PATHSEP_M "include";

static const char CPLUSPLUS[] = "c++";
static const char GCC[] = "gcc";
static const char GPLUSPLUS[] = "g++";
static const char GM2[] = "gm2";
static const char GFORTRAN[] = "gfortran";

#define ERR_SUCCESS 0
#define ERR_MEM_ALLOC_FAILURE -1
#define ERR_UNKNOWN_COMPILER -2
#define ERR_GET_APP_FILENAME_FAILURE -3
#define ERR_EXECVE_FAILURE -4
#define ERR_BAD_TRIPLET -5

#define LIBC_VERSION(major, minor) ((major << 16) + minor)

static const char* get_loader(const char* const triplet) {
	
	if (strcmp(triplet, "aarch64-unknown-linux-gnu") == 0) {
		return "ld-linux-aarch64.so.1";
	}
	
	if (strcmp(triplet, "alpha-unknown-linux-gnu") == 0) {
		return "ld-linux.so.2";
	}
	
	if (strcmp(triplet, "arm-unknown-linux-gnueabi") == 0) {
		return "ld-linux.so.3";
	}
	
	if (strcmp(triplet, "arm-unknown-linux-gnueabi") == 0) {
		return "ld-linux.so.3";
	}
	
	if (strcmp(triplet, "arm-unknown-linux-gnueabihf") == 0) {
		return "ld-linux-armhf.so.3";
	}
	
	if (strcmp(triplet, "hppa-unknown-linux-gnu") == 0) {
		return "ld.so.1";
	}
	
	if (strcmp(triplet, "i386-unknown-linux-gnu") == 0) {
		return "ld-linux.so.2";
	}
	
	if (strcmp(triplet, "ia64-unknown-linux-gnu") == 0) {
		return "ld-linux-ia64.so.2";
	}
	
	if (strcmp(triplet, "mips64el-unknown-linux-gnuabi64") == 0) {
		return "ld.so.1";
	}
	
	if (strcmp(triplet, "mipsel-unknown-linux-gnu") == 0) {
		return "ld.so.1";
	}
	
	if (strcmp(triplet, "mips-unknown-linux-gnu") == 0) {
		return "ld.so.1";
	}
	
	if (strcmp(triplet, "powerpc64le-unknown-linux-gnu") == 0) {
		return "ld64.so.2";
	}
	
	if (strcmp(triplet, "powerpc-unknown-linux-gnu") == 0) {
		return "ld.so.1";
	}
	
	if (strcmp(triplet, "s390-unknown-linux-gnu") == 0) {
		return "ld.so.1";
	}
	
	if (strcmp(triplet, "s390x-unknown-linux-gnu") == 0) {
		return "ld64.so.1";
	}
	
	if (strcmp(triplet, "sparc-unknown-linux-gnu") == 0) {
		return "ld-linux.so.2";
	}
	
	if (strcmp(triplet, "x86_64-unknown-linux-gnu") == 0) {
		return "ld-linux-x86-64.so.2";
	}
	
	return NULL;
	
}

static int get_env(const char* const key) {
	
	const char* const value = getenv(key);
	
	if (value == NULL) {
		return 0;
	}
	
	return (strcmp(value, "1") == 0);
	
}

int main(int argc, char* argv[], char* envp[]) {
	
	int err = ERR_SUCCESS;
	
	size_t size = 0;
	size_t offset = 0;
	size_t index = 0;
	
	long int libc_major = 0;
	long int libc_minor = 0;
	
	int intercept = 0;
	
	int wants_system_libraries = 0;
	int wants_builtin_loader = 0;
	int wants_runtime_rpath = 0;
	int wants_nz = 0;
	
	int address_sanitizer = 0;
	int verbose = 0;
	int wants_libcxx = 0;
	int wants_rt_library = 0;
	int require_atomic_library = 0;
	
	int have_rt_library = 0;
	
	char** args = NULL;
	char* arg = NULL;
	
	const char* cc = NULL;
	char* start = NULL;
	char* ptr = NULL;
	char* dst = NULL;
	const char* opt = NULL;
	const char* pattern = NULL;
	
	const char* prev = NULL;
	const char* cur = NULL;
	
	char* executable = NULL;
	
	char* gpp_builtins_include_directory = NULL;
	
	char* gpp_include_directory = NULL;
	char* gcc_include_directory = NULL;
	
	char* primary_library_directory = NULL;
	char* secondary_library_directory = NULL;
	
	static char* directory = NULL;
	
	char* nz_sysroot_directory = NULL;
	
	char* sysroot_include_directory = NULL;
	char* sysroot_include_missing_directory = NULL;
	char* sysroot_library_directory = NULL;
	char* sysroot_runtime_directory = NULL;
	
	char* sysroot_dynamic_linker = NULL;
	
	char* sysroot_directory = NULL;
	char* parent_directory = NULL;
	
	char* fname = NULL;
	char* app_filename = NULL;
	
	char* non_prefixed_triplet = NULL;
	char* triplet = NULL;
	char* libc_version = NULL;
	
	#if defined(PINO)
		char* android_api = NULL;
	#endif
	
	#if defined(OBGGCC)
		wants_system_libraries = get_env("OBGGCC_SYSTEM_LIBRARIES");
		wants_builtin_loader = get_env("OBGGCC_BUILTIN_LOADER");
		wants_runtime_rpath = get_env("OBGGCC_RUNTIME_RPATH");
		wants_nz = get_env("OBGGCC_NZ");
	#endif
	
	for (index = 0; index < (size_t) argc; index++) {
		cur = argv[index];
		
		if (strncmp(cur, GCC_OPT_FSANITIZE, strlen(GCC_OPT_FSANITIZE)) == 0) {
			address_sanitizer = 1;
		} else if (strcmp(cur, GCC_OPT_V) == 0) {
			verbose = 1;
		} else if (strcmp(cur, GCC_OPT_STATIC_LIBCXX) == 0 || strcmp(cur, GCC_OPT_L_STDCXX) == 0) {
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
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	get_parent_path(app_filename, parent_directory, 1);
	
	ptr = strchr(fname, '\0');
	
	while (ptr != fname) {
		const unsigned char ch = *ptr;
		
		if (ch == '-') {
			break;
		}
		
		ptr--;
	}
	
	cc = (ptr + 1);
	
	if (!(strcmp(cc, GCC) == 0 || strcmp(cc, GPLUSPLUS) == 0 || strcmp(cc, CPLUSPLUS) == 0 || strcmp(cc, GM2) == 0 || strcmp(cc, GFORTRAN) == 0)) {
		err = ERR_UNKNOWN_COMPILER;
		goto end;
	}
	
	wants_libcxx += (strcmp(cc, GPLUSPLUS) == 0 || strcmp(cc, CPLUSPLUS) == 0 || strcmp(cc, GM2) == 0);
	
	ptr = fname;
	
	while (1) {
		const unsigned char a = *ptr;
		const unsigned char b = *(ptr + 1);
		
		#if defined(OBGGCC)
			if (a == '2' && (b == '.' || b == '-')) {
				break;
			}
		#elif defined(PINO)
			if ((a >= '2' && a <= '3') && (b >= '0' && b <= '9')) {
				break;
			}
		#else
			#error "I don't know how to handle that"
		#endif
		
		ptr++;
	}
	
	size = (size_t) (ptr - fname);
	
	triplet = malloc(size + 1);
	
	if (triplet == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	memcpy(triplet, fname, size);
	triplet[size] = '\0';
	
	/* Atomics are not natively supported on SPARC, so we need to rely on -latomic. */
	require_atomic_library = strcmp(triplet, "sparc-unknown-linux-gnu") == 0;
	
	non_prefixed_triplet = malloc(strlen(triplet) + 1);
	
	if (non_prefixed_triplet == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(non_prefixed_triplet, triplet);
	
	pattern = "-unknown";
	start = strstr(non_prefixed_triplet, pattern);
	
	if (start == NULL) {
		err = ERR_BAD_TRIPLET;
		goto end;
	}
	
	dst = start;
	start += strlen(pattern);
	size = strlen(start) + 1;
	
	memmove(dst, start, size);
	
	if (wants_system_libraries || wants_nz) {
		primary_library_directory = malloc(strlen(USR_DIRECTORY) + strlen(LIB_DIRECTORY) + strlen(PATHSEP_S) + strlen(non_prefixed_triplet) + 1);
		
		if (primary_library_directory == NULL) {
			err = ERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(primary_library_directory, USR_DIRECTORY);
		strcat(primary_library_directory, LIB_DIRECTORY);
		strcat(primary_library_directory, PATHSEP_S);
		strcat(primary_library_directory, non_prefixed_triplet);
		
		secondary_library_directory = malloc(strlen(LIB_DIRECTORY) + strlen(PATHSEP_S) + strlen(non_prefixed_triplet) + 1);
		
		if (secondary_library_directory == NULL) {
			err = ERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(secondary_library_directory, LIB_DIRECTORY);
		strcat(secondary_library_directory, PATHSEP_S);
		strcat(secondary_library_directory, non_prefixed_triplet);
		
		SYSTEM_LIBRARY_PATH[6] = primary_library_directory;
		SYSTEM_LIBRARY_PATH[7] = secondary_library_directory;
	}
	
	start = ptr;
	
	while (1) {
		const unsigned char a = *ptr;
		
		if (a == '-') {
			break;
		}
		
		ptr++;
	}
	
	size = (size_t) (ptr - start);
	
	libc_version = malloc(size + 1);
	
	if (libc_version == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	memcpy(libc_version, start, size);
	libc_version[size] = '\0';
	
	libc_major = strtol(libc_version, &ptr, 16);
	
	if (*ptr != '-') {
		libc_minor = strtol(ptr + 1, NULL, 16);
	}
	
	#if defined(OBGGCC)
		/* Determine whether we need to implicit link with -lrt */
		wants_rt_library = wants_libcxx && !have_rt_library && LIBC_VERSION(libc_major, libc_minor) < LIBC_VERSION(2, 17);
	#endif
	
	executable = malloc(strlen(parent_directory) + strlen(PATHSEP_S) + strlen(triplet) + 1 + strlen(cc) + 1);
	
	if (executable == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(executable, parent_directory);
	strcat(executable, PATHSEP_S);
	strcat(executable, triplet);
	strcat(executable, "-");
	strcat(executable, cc);
	
	get_parent_path(app_filename, parent_directory, 2);
	
	sysroot_directory = malloc(strlen(parent_directory) + strlen(PATHSEP_S) + strlen(triplet) + strlen(libc_version) + 1);
	
	if (sysroot_directory == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(sysroot_directory, parent_directory);
	strcat(sysroot_directory, PATHSEP_S);
	strcat(sysroot_directory, triplet);
	strcat(sysroot_directory, libc_version);
	
	size = (size_t) argc + 13 + (size_t) wants_rt_library;
	
	if (wants_system_libraries) {
		size += (sizeof(SYSTEM_LIBRARY_PATH) / sizeof(*SYSTEM_LIBRARY_PATH)) * 6;
		size += 8;
	}
	
	if (wants_nz) {
		size += (sizeof(SYSTEM_LIBRARY_PATH) / sizeof(*SYSTEM_LIBRARY_PATH)) * 6;
		size += 8;
	}
	
	if (wants_builtin_loader) {
		size += 8;
		
		if (wants_nz) {
			size += (sizeof(SYSTEM_LIBRARY_PATH) / sizeof(*SYSTEM_LIBRARY_PATH)) * 4;
		}
	}
	
	if (wants_runtime_rpath) {
		size += 4;
	}
	
	if (address_sanitizer) {
		size += 2;
	}
	
	if (require_atomic_library) {
		size += 1;
	}
	
	#if defined(PINO)
		size += 2; /* -D __ANDROID_API__=<LEVEL> */
	#endif
	
	args = malloc(size * sizeof(char*));
	
	if (args == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	sysroot_include_directory = malloc(strlen(sysroot_directory) + strlen(INCLUDE_DIR) + 1);
	
	if (sysroot_include_directory == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(sysroot_include_directory, sysroot_directory);
	strcat(sysroot_include_directory, INCLUDE_DIR);
	
	if (wants_system_libraries || wants_nz) {
		sysroot_include_missing_directory = malloc(strlen(sysroot_directory) + strlen(INCLUDE_MISSING_DIR) + 1);
		
		if (sysroot_include_missing_directory == NULL) {
			err = ERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(sysroot_include_missing_directory, sysroot_directory);
		strcat(sysroot_include_missing_directory, INCLUDE_MISSING_DIR);
	}
	
	sysroot_library_directory = malloc(strlen(sysroot_directory) + strlen(LIBRARY_DIR) + 1);
	
	if (sysroot_library_directory == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(sysroot_library_directory, sysroot_directory);
	strcat(sysroot_library_directory, LIBRARY_DIR);
	
	if (wants_runtime_rpath) {
		sysroot_runtime_directory = malloc(strlen(sysroot_library_directory) + strlen(PATHSEP_S) + strlen(GCC) + 1);
		
		if (sysroot_runtime_directory == NULL) {
			err = ERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(sysroot_runtime_directory, sysroot_library_directory);
		strcat(sysroot_runtime_directory, PATHSEP_S);
		strcat(sysroot_runtime_directory, GCC);
	}
	
	if (wants_builtin_loader) {
		opt = get_loader(triplet);
		
		sysroot_dynamic_linker = malloc(strlen(sysroot_library_directory) + strlen(PATHSEP_S) + strlen(opt) + 1);
		
		if (sysroot_dynamic_linker == NULL) {
			err = ERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(sysroot_dynamic_linker, sysroot_library_directory);
		strcat(sysroot_dynamic_linker, PATHSEP_S);
		strcat(sysroot_dynamic_linker, opt);
	}
	
	gcc_include_directory = malloc(strlen(parent_directory) + strlen(GCC_LIBRARY_DIR) + strlen(PATHSEP_S) + strlen(triplet) + strlen(PATHSEP_S) + strlen(GCC_MAJOR_VERSION) + strlen(INCLUDE_DIR) + 1);
	
	if (gcc_include_directory == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(gcc_include_directory, parent_directory);
	strcat(gcc_include_directory, GCC_LIBRARY_DIR);
	strcat(gcc_include_directory, PATHSEP_S);
	strcat(gcc_include_directory, triplet);
	strcat(gcc_include_directory, PATHSEP_S);
	strcat(gcc_include_directory, GCC_MAJOR_VERSION);
	strcat(gcc_include_directory, INCLUDE_DIR);
	
	gpp_include_directory = malloc(strlen(parent_directory) + strlen(PATHSEP_S) + strlen(triplet) + strlen(INCLUDE_DIR) + strlen(PATHSEP_S) + strlen(CPLUSPLUS) + strlen(PATHSEP_S) + strlen(GCC_MAJOR_VERSION) + 1);
	
	if (gpp_include_directory == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(gpp_include_directory, parent_directory);
	strcat(gpp_include_directory, PATHSEP_S);
	strcat(gpp_include_directory, triplet);
	strcat(gpp_include_directory, INCLUDE_DIR);
	strcat(gpp_include_directory, PATHSEP_S);
	strcat(gpp_include_directory, CPLUSPLUS);
	strcat(gpp_include_directory, PATHSEP_S);
	strcat(gpp_include_directory, GCC_MAJOR_VERSION);
	
	gpp_builtins_include_directory = malloc(strlen(gpp_include_directory) + strlen(PATHSEP_S) + strlen(triplet) + 1);
	
	if (gpp_builtins_include_directory == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(gpp_builtins_include_directory, gpp_include_directory);
	strcat(gpp_builtins_include_directory, PATHSEP_S);
	strcat(gpp_builtins_include_directory, triplet);
	
	arg = malloc(strlen(GCC_OPT_SYSROOT) + strlen(sysroot_directory) + 1);
	
	if (arg == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(arg, GCC_OPT_SYSROOT);
	strcat(arg, sysroot_directory);
	
	nz_sysroot_directory = malloc(strlen(sysroot_directory) + strlen(NZ_SYSROOT) + 1);
	
	if (nz_sysroot_directory == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(nz_sysroot_directory, sysroot_directory);
	strcat(nz_sysroot_directory, NZ_SYSROOT);
		
	args[offset++] = executable;
	args[offset++] = arg;
	args[offset++] = (char*) GCC_OPT_NOSTDINC;
	
	if (strcmp(cc, GPLUSPLUS) == 0 || strcmp(cc, CPLUSPLUS) == 0) {
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
		args[offset++] = (char*) GCC_OPT_L_RT;
	}
	
	if (require_atomic_library) {
		args[offset++] = (char*) GCC_OPT_L_ATOMIC;
	}
	
	if (wants_nz) {
		args[offset++] = (char*) GCC_OPT_ISYSTEM;
		args[offset++] = sysroot_include_missing_directory;
		
		directory = malloc(strlen(nz_sysroot_directory) + strlen(SYSTEM_INCLUDE_PATH) + 1);
		
		if (directory == NULL) {
			err = ERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(directory, nz_sysroot_directory);
		strcat(directory, SYSTEM_INCLUDE_PATH);
		
		args[offset++] = (char*) GCC_OPT_ISYSTEM;
		args[offset++] = directory;
		
		directory = malloc(strlen(nz_sysroot_directory) + strlen(SYSTEM_INCLUDE_PATH) + strlen(PATHSEP_S) + strlen(non_prefixed_triplet) + 1);
		
		if (directory == NULL) {
			err = ERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(directory, nz_sysroot_directory);
		strcat(directory, SYSTEM_INCLUDE_PATH);
		strcat(directory, PATHSEP_S);
		strcat(directory, non_prefixed_triplet);
		
		args[offset++] = (char*) GCC_OPT_ISYSTEM;
		args[offset++] = directory;
		
		args[offset++] = (char*) GCC_OPT_XLINKER;
		args[offset++] = (char*) LD_OPT_UNRESOLVED_SYMBOLS;
		
		for (index = 0; index < sizeof(SYSTEM_LIBRARY_PATH) / sizeof(*SYSTEM_LIBRARY_PATH); index++) {
			cur = SYSTEM_LIBRARY_PATH[index];
			
			directory = malloc(strlen(nz_sysroot_directory) + strlen(cur) + 1);
			
			if (directory == NULL) {
				err = ERR_MEM_ALLOC_FAILURE;
				goto end;
			}
			
			strcpy(directory, nz_sysroot_directory);
			strcat(directory, cur);
			
			args[offset++] = (char*) GCC_OPT_LIBDIR;
			args[offset++] = (char*) directory;
			
			args[offset++] = (char*) GCC_OPT_XLINKER;
			args[offset++] = (char*) LD_OPT_RPATH_LINK;
			
			args[offset++] = (char*) GCC_OPT_XLINKER;
			args[offset++] = (char*) directory;
		}
	}
	
	if (wants_system_libraries) {
		args[offset++] = (char*) GCC_OPT_ISYSTEM;
		args[offset++] = sysroot_include_missing_directory;
		
		args[offset++] = (char*) GCC_OPT_ISYSTEM;
		args[offset++] = (char*) SYSTEM_INCLUDE_PATH;
		
		directory = malloc(strlen(SYSTEM_INCLUDE_PATH) + strlen(PATHSEP_S) + strlen(non_prefixed_triplet) + 1);
		
		if (directory == NULL) {
			err = ERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(directory, SYSTEM_INCLUDE_PATH);
		strcat(directory, PATHSEP_S);
		strcat(directory, non_prefixed_triplet);
		
		args[offset++] = (char*) GCC_OPT_ISYSTEM;
		args[offset++] = directory;
		
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
	
	if (wants_builtin_loader) {
		args[offset++] = (char*) GCC_OPT_XLINKER;
		args[offset++] = (char*) LD_OPT_DYNAMIC_LINKER;
		
		args[offset++] = (char*) GCC_OPT_XLINKER;
		args[offset++] = sysroot_dynamic_linker;
		
		args[offset++] = (char*) GCC_OPT_XLINKER;
		args[offset++] = (char*) LD_OPT_RPATH;
		
		args[offset++] = (char*) GCC_OPT_XLINKER;
		args[offset++] = sysroot_library_directory;
		
		for (index = 0; wants_nz && index < sizeof(SYSTEM_LIBRARY_PATH) / sizeof(*SYSTEM_LIBRARY_PATH); index++) {
			cur = SYSTEM_LIBRARY_PATH[index];
			
			directory = malloc(strlen(nz_sysroot_directory) + strlen(cur) + 1);
			
			if (directory == NULL) {
				err = ERR_MEM_ALLOC_FAILURE;
				goto end;
			}
			
			strcpy(directory, nz_sysroot_directory);
			strcat(directory, cur);
			
			args[offset++] = (char*) GCC_OPT_XLINKER;
			args[offset++] = (char*) LD_OPT_RPATH;
			
			args[offset++] = (char*) GCC_OPT_XLINKER;
			args[offset++] = (char*) directory;
		}
	}
	
	if (wants_runtime_rpath) {
		args[offset++] = (char*) GCC_OPT_XLINKER;
		args[offset++] = (char*) LD_OPT_RPATH;
		
		args[offset++] = (char*) GCC_OPT_XLINKER;
		args[offset++] = sysroot_runtime_directory;
	}
	
	if (address_sanitizer) {
		args[offset++] = (char*) GCC_OPT_XLINKER;
		args[offset++] = (char*) LD_OPT_UNRESOLVED_SYMBOLS;
	}
	
	#if defined(PINO)
		android_api = malloc(strlen(M_ANDROID_API) + strlen(libc_version) + 1);
		
		if (android_api == NULL) {
			err = ERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(android_api, M_ANDROID_API);
		strcat(android_api, libc_version);
		
		args[offset++] = (char*) GCC_OPT_D;
		args[offset++] = android_api;
	#endif
	
	memcpy(&args[offset], &argv[1], (size_t) argc * sizeof(*argv));
	
	if (verbose) {
		printf("%s", "+ ");
		
		for (index = 0; 1; index++) {
			cur = args[index];
			
			if (cur == NULL) {
				break;
			}
			
			if (index != 0) {
				printf("%s", " ");
			}
			
			printf("%s", cur);
		}
		
		printf("%s", "\n");
	}
	
	if (execve(executable, args, envp) == -1) {
		err = ERR_EXECVE_FAILURE;
		goto end;
	}
	
	end:;
	
	free(triplet);
	free(libc_version);
	free(executable);
	free(sysroot_directory);
	free(app_filename);
	free(args);
	free(arg);
	free(sysroot_include_directory);
	free(sysroot_include_missing_directory);
	free(sysroot_library_directory);
	free(gcc_include_directory);
	free(gpp_include_directory);
	free(gpp_builtins_include_directory);
	free(primary_library_directory);
	free(secondary_library_directory);
	free(nz_sysroot_directory);
	free(non_prefixed_triplet);
	
	#if defined(PINO)
		free(android_api);
	#endif
	
	switch (err) {
		case ERR_SUCCESS:
			opt = "Success";
			break;
		case ERR_MEM_ALLOC_FAILURE:
			opt = "Could not allocate memory";
			break;
		case ERR_UNKNOWN_COMPILER:
			opt = "Unknown GCC compiler";
			break;
		case ERR_GET_APP_FILENAME_FAILURE:
			opt = "Could not get app filename";
			break;
		case ERR_EXECVE_FAILURE:
			opt = "Call to execve failed";
			break;
		case ERR_BAD_TRIPLET:
			opt = "Unknown triplet";
			break;
		default:
			opt = "Unknown error";
			break;
	}
	
	if (err != ERR_SUCCESS) {
		fprintf(stderr, "fatal error: %s\n", opt);
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
	
}
