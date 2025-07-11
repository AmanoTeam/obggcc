#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <unistd.h>

#include "fs/getexec.h"
#include "fs/ext.h"
#include "fs/dirname.h"
#include "fs/parentpath.h"
#include "fs/sep.h"
#include "fs/cp.h"
#include "fs/exists.h"
#include "fs/basename.h"
#include "biggestint.h"

#if !(defined(OBGGCC) || defined(PINO))
	#error "Please define the cross-compiler flavor for which we will be a wrapper"
#endif

static const char GCC_MAJOR_VERSION[] = "15";

static const char INCLUDE_DIR[] = PATHSEP_M "include";
static const char INCLUDE_MISSING_DIR[] = PATHSEP_M "include-missing";
static const char LIBRARY_DIR[] = PATHSEP_M "lib";
static const char GCC_LIBRARY_DIR[] = PATHSEP_M "lib" PATHSEP_M "gcc";

static const char RT_LIBRARY[] = "rt";
static const char STDCXX_LIBRARY[] = "stdc++";
static const char ATOMIC_LIBRARY[] = "atomic";
static const char GOMP_LIBRARY[] = "gomp";
static const char ITM_LIBRARY[] = "itm";

static const char LIBATOMIC_SHARED[] = "libatomic.so";

#if defined(PINO)
	static const char LIBSTDCXX_SHARED[] = "libestdc++.so";
	static const char LIBGCC_SHARED[] = "libegcc.so";
#else
	static const char LIBSTDCXX_SHARED[] = "libstdc++.so";
	static const char LIBGCC_SHARED[] = "libgcc_s.so";
#endif

static const char LIBGOMP_SHARED[] = "libgomp.so";
static const char LIBITM_SHARED[] = "libitm.so";
static const char LIBSSP_SHARED[] = "libssp.so";

static const char LIBASAN_SHARED[] = "libasan.so";
static const char LIBHWASAN_SHARED[] = "libhwasan.so";
static const char LIBLSAN_SHARED[] = "liblsan.so";
static const char LIBTSAN_SHARED[] = "libtsan.so";
static const char LIBUBSAN_SHARED[] = "libubsan.so";

static const char GCC_OPT_ISYSTEM[] = "-isystem";
static const char GCC_OPT_SYSROOT[] = "--sysroot";
static const char GCC_OPT_NOSTDINC[] = "--no-standard-includes";
static const char GCC_OPT_LIBDIR[] = "-L";
static const char GCC_OPT_STATIC_LIBCXX[] = "-static-libstdc++";
static const char GCC_OPT_STATIC_LIBGCC[] = "-static-libgcc";
static const char GCC_OPT_L[] = "-l";
static const char GCC_OPT_V[] = "-v";
static const char GCC_OPT_D[] = "-D";
static const char GCC_OPT_O[] = "-o";
static const char GCC_OPT_OS[] = "-Os";
static const char GCC_OPT_L_RT[] = "-lrt";
static const char GCC_OPT_FSANITIZE[] = "-fsanitize=";
static const char GCC_OPT_L_STDCXX[] = "-lstdc++";
static const char GCC_OPT_L_ATOMIC[] = "-latomic";
static const char GCC_OPT_L_GOMP[] = "-lgomp";
static const char GCC_OPT_L_ITM[] = "-litm";
static const char GCC_OPT_XLINKER[] = "-Xlinker";
static const char GCC_OPT_WL[] = "-Wl,";
static const char GCC_OPT_F_FAT_LTO_OBJECTS[] = "-ffat-lto-objects";
static const char GCC_OPT_F_NO_FAT_LTO_OBJECTS[] = "-fno-fat-lto-objects";
static const char GCC_OPT_F_LTO[] = "-flto";
static const char GCC_OPT_F_USE_LD[] = "-fuse-ld=";
static const char GCC_OPT_DFORTIFY_SOURCE[] = "-D_FORTIFY_SOURCE=";
static const char GCC_OPT_U_GNUC[] = "-U__GNUC__";

static const char GCC_OPT_D_CLANG[] = "-D__clang__=1";
static const char GCC_OPT_D_CLANG_MAJOR[] = "-D__clang_major__=21";
static const char GCC_OPT_D_CLANG_MINOR[] = "-D__clang_minor__=0";
static const char GCC_OPT_D_CLANG_PATCHLEVEL[] = "-D__clang_patchlevel__=0";

static const char GCC_OPT_F_STACK_PROTECTOR[] = "-fstack-protector";

static const char CLANG_OPT_OZ[] = "-Oz";
static const char CLANG_OPT_ICF[] = "--icf=";
static const char CLANG_OPT_TARGET[] = "--target";
static const char CLANG_OPT_Q_UNUSED_ARGUMENTS[] = "-Qunused-arguments";
static const char CLANG_OPT_W_NO_UNUSED_COMMAND_LINE_ARGUMENT[] = "-Wno-unused-command-line-argument";
static const char CLANG_OPT_W_NO_INVALID_COMMAND_LINE_ARGUMENT[] = "-Wno-invalid-command-line-argument";
static const char CLANG_OPT_PRINT_RESOURCE_DIR[] = "-print-resource-dir";
static const char CLANG_OPT_F_NO_LIMIT_DEBUG_INFO[] = "-fno-limit-debug-info";

static const char LD_OPT_DYNAMIC_LINKER[] = "-dynamic-linker";
static const char LD_OPT_RPATH_LINK[] = "-rpath-link";
static const char LD_OPT_RPATH[] = "-rpath";
static const char LD_OPT_UNRESOLVED_SYMBOLS[] = "--unresolved-symbols=ignore-in-shared-libs";

static const char M_ANDROID_API[] = "__ANDROID_API__=";

static const char CMAKE_C_COMPILER_ID[] = "CMakeCCompilerId.c";
static const char CMAKE_CXX_COMPILER_ID[] = "CMakeCXXCompilerId.cpp";

static const char CMAKE_C_COMPILER_TEST[] = "testCCompiler.c";
static const char CMAKE_CXX_COMPILER_TEST[] = "testCXXCompiler.cxx";

static const char CMAKE_FILES_DIRECTORY[] = "CMakeFiles";

static const char DEFAULT_TARGET[] = 
#if defined(OBGGCC)
	 "x86_64-unknown-linux-gnu2.3";
#elif defined(PINO)
	"x86_64-unknown-linux-android21";
#else
	#error "I don't know how to handle this"
#endif

#if defined(OBGGCC)
	#define WRAPPER_FLAVOR_NAME "OBGGCC"
#elif defined(PINO)
	#define WRAPPER_FLAVOR_NAME "PINO"
#else
	#error "I don't know how to handle this"
#endif

static const char HYPHEN[] = "-";

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

static const char CPP[] = "c++";

static const char GCC[] = "gcc";
static const char GPP[] = "g++";

static const char CLANG[] = "clang";
static const char CLANGPP[] = "clang++";

static const char GM2[] = "gm2";

static const char GFORTRAN[] = "gfortran";

static const char VENDOR_NONE[] = "-none-";
static const char VENDOR_UNKNOWN[] = "-unknown-";

static const char LD_PREFIX[] = "ld.";

static const char EQUAL = '=';
static const char EQUAL_S[] = "=";

static const char* const FASTER_LINKERS[] = {
	"mold",
	"lld",
	"gold"
};

#define ERR_SUCCESS 0
#define ERR_MEM_ALLOC_FAILURE -1
#define ERR_UNKNOWN_COMPILER -2
#define ERR_GET_APP_FILENAME_FAILURE -3
#define ERR_EXECVE_FAILURE -4
#define ERR_BAD_TRIPLET -5
#define ERR_GETEXT_FAILURE -6
#define ERR_DIRNAME_FAILURE -7
#define ERR_COPY_FILE_FAILURE -8
#define ERR_UNKNOWN_SYSTEM_VERSION -9

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

static int known_clang(const char* const cc) {
	
	const int status = (
		strcmp(cc, CLANG) == 0 ||
		strcmp(cc, CLANGPP) == 0
	);
	
	return status;
	
}

static int known_gcc(const char* const cc) {
	
	const int status = (
		strcmp(cc, GCC) == 0 ||
		strcmp(cc, GPP) == 0 ||
		strcmp(cc, CPP) == 0 ||
		strcmp(cc, GM2) == 0 ||
		strcmp(cc, GFORTRAN) == 0
	);
	
	return status;
	
}

static int known_compiler(const char* const cc) {
	
	return (known_clang(cc) || known_gcc(cc));
	
}

static int clang_specific_remove(const char* const prev, const char* const cur) {
	/*
	Remove Clang-specific options that have no GCC equivalents.
	*/
	
	int status = 1;
	
	const char* previous = prev;
	const char* current = cur;
	
	if (strncmp(current, GCC_OPT_WL, strlen(GCC_OPT_WL)) == 0) {
		current += strlen(GCC_OPT_WL);
	}
	
	if (strcmp(current, CLANG_OPT_Q_UNUSED_ARGUMENTS) == 0) {
		goto end;
	}
	
	if (strcmp(current, CLANG_OPT_W_NO_UNUSED_COMMAND_LINE_ARGUMENT) == 0) {
		goto end;
	}
	
	if (strcmp(current, CLANG_OPT_W_NO_INVALID_COMMAND_LINE_ARGUMENT) == 0) {
		goto end;
	}
	
	if (strncmp(current, CLANG_OPT_ICF, strlen(CLANG_OPT_ICF)) == 0) {
		goto end;
	}
	
	if (strcmp(current, CLANG_OPT_F_NO_LIMIT_DEBUG_INFO) == 0) {
		goto end;
	}
	
	status = 0;
	
	end:;
	
	return status;
	
}

static int clang_specific_replace(
	const char* const cur,
	size_t* const kargc,
	char** kargv
) {
	/*
	Replace Clang-specific options with GCC equivalents, if any.
	*/
	
	int status = 0;
	
	size_t index = *kargc;
	const char* current = cur;
	
	const biguint_t gcc_version = strtobui(GCC_MAJOR_VERSION, NULL, 10);
	
	if (strncmp(current, GCC_OPT_F_LTO, strlen(GCC_OPT_F_LTO)) == 0) {
		current += strlen(GCC_OPT_F_LTO);
		
		if (*current != EQUAL) {
			status = 0;
			goto end;
		}
		
		current += 1;
		
		if (*current == '\0') {
			status = 0;
			goto end;
		}
		
		kargv[index++] = (char*) GCC_OPT_F_LTO;
		
		if (strcmp(current, "auto") == 0 || strcmp(current, "full") == 0) {
			/* Replace -flto={full,auto} with -flto -ffat-lto-objects. */
			kargv[index++] = (char*) GCC_OPT_F_FAT_LTO_OBJECTS;
		} else {
			/* Replace -flto=thin with -flto -fno-fat-lto-objects. */
			kargv[index++] = (char*) GCC_OPT_F_NO_FAT_LTO_OBJECTS;
		}
		
		status = 1;
		goto end;
	} else if (strcmp(current, CLANG_OPT_OZ) == 0 && gcc_version < 12) {
		/* Replace -Oz with -Os. */
		kargv[index++] = (char*) GCC_OPT_OS;
		
		status = 1;
		goto end;
	}
	
	end:;
	
	*kargc = index;
	
	return status;
	
}

static const char* get_fast_linker(
	const char* const directory,
	const char* const triplet
) {
	/*
	Pick a faster linker if available in the current bin directory.
	*/
	
	size_t index = 0;
	
	char* executable = NULL;
	char* end = NULL;
	
	const char* linker = NULL;
	
	for (index = 0; index < sizeof(FASTER_LINKERS) / sizeof(*FASTER_LINKERS); index++) {
		linker = FASTER_LINKERS[index];
		
		executable = malloc(
			strlen(directory) +
			strlen(PATHSEP_S) +
			strlen(triplet) + 
			strlen(HYPHEN) +
			strlen(LD_PREFIX) +
			strlen(linker) +
			1
		);
		
		if (executable == NULL) {
			goto end;
		}
		
		strcpy(executable, directory);
		strcat(executable, PATHSEP_S);
		
		end = strchr(executable, '\0');
		
		/* Try the triplet-prefixed executable (<triplet>-<ld.<linker>) */
		strcat(executable, triplet);
		strcat(executable, HYPHEN);
		strcat(executable, LD_PREFIX);
		strcat(executable, linker);
		
		if (file_exists(executable) == 1) {
			goto end;
		}
		
		/* Try the non-triplet-prefixed executable (<ld.<linker>) */
		strcpy(end, LD_PREFIX);
		strcat(end, linker);
		
		if (file_exists(executable) == 1) {
			goto end;
		}
		
		free(executable);
		executable = NULL;
		
		linker = NULL;
	}
	
	end:;
	
	free(executable);
	
	return linker;
	
}

#if defined(PINO)
int copy_shared_library(
	const char* const source_directory,
	const char* const destination_directory,
	const char* const source_file,
	const char* const destination_file
) {
	
	int err = ERR_SUCCESS;
	
	char* source = NULL;
	char* destination = NULL;
	
	source = malloc(strlen(source_directory) + strlen(PATHSEP_S) + strlen(source_file) + 1);
	
	if (source == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(source, source_directory);
	strcat(source, PATHSEP_S);
	strcat(source, source_file);
	
	destination = malloc(strlen(destination_directory) + strlen(PATHSEP_S) + strlen(destination_file) + 1);
	
	if (destination == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	destination[0] = '\0';
	
	/* If empty, assume the path is relative to the current directory */
	if (*destination_directory != '\0') {
		strcat(destination, destination_directory);
		strcat(destination, PATHSEP_S);
	}
	
	strcat(destination, destination_file);
	
	if (file_exists(destination) != 1) {
		/* fprintf(stderr, "warning: copying shared library from '%s' to '%s'\n", source, destination); */
		
		 if (copy_file(source, destination) != 0) {
			err = ERR_COPY_FILE_FAILURE;
			goto end;
		}
	}
	
	end:;
	
	free(source);
	free(destination);
	
	return err;
	
}
#endif

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
	
	#if defined(PINO)
		int wants_disable_shlib = 0;
	#endif
	
	int address_sanitizer = 0;
	int stack_protector = 0;
	int verbose = 0;
	int wants_libcxx = 0;
	int wants_static_libcxx = 0;
	int wants_static_libgcc = 0;
	int wants_libatomic = 0;
	int wants_libgomp = 0;
	int wants_libgcc = 0;
	int wants_libitm = 0;
	int wants_librt = 0;
	int wants_libssp = 0;
	
	#if defined(OBGGCC)
		int require_atomic_library = 0;
	#endif
	
	int override_linker = 0;
	
	int cmake_init = 0;
	
	int have_rt_library = 0;
	
	char** args = NULL;
	char* arg = NULL;
	
	const char* cc = NULL;
	const char* override_cc = NULL;
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
	
	char* app_filename = NULL;
	
	char* non_prefixed_triplet = NULL;
	char* triplet = NULL;
	char* override_triplet = NULL;
	
	char* libc_version = NULL;
	
	char* file_name = NULL;
	char* file_extension = NULL;
	char* output_directory = NULL;
	
	char* source = NULL;
	char* destination = NULL;
	
	#if defined(PINO)
		char* android_api = NULL;
	#endif
	
	size_t kargc = 0;
	char** kargv = NULL;
	
	char* linker = NULL;
	
	unsigned char ch = 0;
	
	#if defined(OBGGCC)
		wants_system_libraries = get_env(WRAPPER_FLAVOR_NAME "_SYSTEM_LIBRARIES");
		wants_builtin_loader = get_env(WRAPPER_FLAVOR_NAME "_BUILTIN_LOADER");
	#endif
	
	#if defined(PINO)
		wants_disable_shlib = get_env(WRAPPER_FLAVOR_NAME "_DISABLE_SHLIB");
	#endif
	
	wants_nz = get_env(WRAPPER_FLAVOR_NAME "_NZ");
	wants_runtime_rpath = get_env(WRAPPER_FLAVOR_NAME "_RUNTIME_RPATH");
	verbose = get_env(WRAPPER_FLAVOR_NAME "_VERBOSE");
	
	kargv = malloc(
		sizeof(*argv) * (
			argc + 
			1 + /* -U__GNUC__ */
			1 + /* -D__clang__ */
			1 + /* -D__clang_major__ */
			1 + /* -D__clang_minor__ */
			1 + /* -D__clang_patchlevel__ */
			1 + /* -ffat-lto-objects / -fno-fat-lto-objects */
			1 + /* -fuse-ld=<linker> */
			1 /* NULL */
		)
	);
	
	if (kargv == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	for (index = 0; index < (size_t) argc; index++) {
		cur = argv[index];
		
		if (strncmp(cur, GCC_OPT_FSANITIZE, strlen(GCC_OPT_FSANITIZE)) == 0) {
			address_sanitizer = 1;
		} else if (strncmp(cur, GCC_OPT_F_USE_LD, strlen(GCC_OPT_F_USE_LD)) == 0) {
			override_linker = 1;
		} else if (strncmp(cur, GCC_OPT_F_STACK_PROTECTOR, strlen(GCC_OPT_F_STACK_PROTECTOR)) == 0) {
			stack_protector = 1;
		} else if (strcmp(cur, GCC_OPT_V) == 0) {
			verbose = 1;
		} else if (strcmp(cur, GCC_OPT_L_STDCXX) == 0) {
			wants_libcxx = 1;
		} else if (strcmp(cur, GCC_OPT_STATIC_LIBCXX) == 0) {
			wants_libcxx = 1;
			wants_static_libcxx = 1;
			
			#if defined(PINO)
				if (!wants_disable_shlib) {
					continue;
				}
			#endif
		} else if (strcmp(cur, GCC_OPT_STATIC_LIBGCC) == 0) {
			wants_libgcc = 1;
			wants_static_libgcc = 1;
			
			#if defined(PINO)
				if (!wants_disable_shlib) {
					continue;
				}
			#endif
		} else if (strcmp(cur, GCC_OPT_L_ATOMIC) == 0) {
			wants_libatomic = 1;
		} else if (strcmp(cur, GCC_OPT_L_GOMP) == 0) {
			wants_libgomp = 1;
		} else if (strcmp(cur, GCC_OPT_L_ITM) == 0) {
			wants_libitm = 1;
		} else if (strcmp(cur, GCC_OPT_L_RT) == 0) {
			have_rt_library = 1;
		} else if (prev != NULL && strcmp(prev, GCC_OPT_L) == 0) {
			if (strcmp(cur, STDCXX_LIBRARY) == 0) {
				wants_libcxx = 1;
			} else if (strcmp(cur, RT_LIBRARY) == 0) {
				have_rt_library = 1;
			} else if (strcmp(cur, ATOMIC_LIBRARY) == 0) {
				wants_libatomic = 1;
			} else if (strcmp(cur, GOMP_LIBRARY) == 0) {
				wants_libgomp = 1;
			} else if (strcmp(cur, ITM_LIBRARY) == 0) {
				wants_libitm = 1;
			}
		} else if (strcmp(cur, CMAKE_C_COMPILER_ID) == 0 || strcmp(cur, CMAKE_CXX_COMPILER_ID) == 0) {
			cmake_init = 1;
		} else if (strstr(cur, CMAKE_FILES_DIRECTORY) != NULL) {
			file_name = basename(cur);
			
			if (file_name != NULL) {
				cmake_init += (strncmp(file_name, CMAKE_C_COMPILER_TEST, strlen(CMAKE_C_COMPILER_TEST)) == 0 || strncmp(file_name, CMAKE_CXX_COMPILER_TEST, strlen(CMAKE_CXX_COMPILER_TEST)) == 0);
			}
		} else if (prev != NULL && strcmp(prev, GCC_OPT_O) == 0 && output_directory == NULL) {
			file_extension = getext(cur);
			
			if (file_extension != NULL && strcmp(file_extension, "so") == 0) {
				output_directory = dirname(cur);
				
				if (output_directory == NULL) {
					err = ERR_DIRNAME_FAILURE;
					goto end;
				}
			}
		} else if (strncmp(cur, CLANG_OPT_TARGET, strlen(CLANG_OPT_TARGET)) == 0 || strncmp(cur, CLANG_OPT_TARGET + 1, strlen(CLANG_OPT_TARGET + 1)) == 0) {
			cur += strlen(CLANG_OPT_TARGET);
			
			/* Clang has two variants of this flag: one with double hyphens, and one with a single hyphen. */
			if (strncmp(cur, CLANG_OPT_TARGET + 1, strlen(CLANG_OPT_TARGET + 1)) == 0) {
				cur -= 1;
			}
			
			ch = *cur;
			
			if (!(ch == EQUAL || ch == '\0')) {
				goto next;
			}
			
			if (ch == EQUAL) {
				cur++;
			} else {
				index++;
				
				if (index > (size_t) argc) {
					continue;
				}
				
				cur = argv[index];
			}
			
			size = strlen(cur);
			
			if (size == 0) {
				continue;
			}
			
			override_triplet = malloc(size + strlen(VENDOR_UNKNOWN) + 1);
			
			if (override_triplet == NULL) {
				err = ERR_MEM_ALLOC_FAILURE;
				goto end;
			}
			
			/* The Android NDK defaults to "none" as the vendor, but we use "unknown" instead */
			ptr = strstr(cur, VENDOR_NONE);
			
			if (ptr != NULL) {
				/* Architecture */
				size = (size_t) (ptr - cur);
				strncpy(override_triplet, cur, size);
				override_triplet[size] = '\0';
				
				if (strcmp(override_triplet, "armv7") == 0) {
					strcpy(override_triplet, "arm");
				}
				
				/* Vendor */
				strcat(override_triplet, VENDOR_UNKNOWN);
				
				/* System */
				ptr += strlen(VENDOR_NONE);
				strcat(override_triplet, ptr);
			} else {
				strcpy(override_triplet, cur);
			}
			
			continue;
		} else if (strncmp(cur, GCC_OPT_SYSROOT, strlen(GCC_OPT_SYSROOT)) == 0) {
			/* Don't override the system root, Clang. That's our job. */
			cur += strlen(GCC_OPT_SYSROOT);
			ch = *cur;
			
			if (!(ch == EQUAL || ch == '\0')) {
				goto next;
			}
			
			if (ch == '\0') {
				index++;
			}
			
			continue;
		} else if (strcmp(cur, CLANG_OPT_PRINT_RESOURCE_DIR) == 0) {
			/*
			The ndk-build's Makefile calls Clang with this flag every time a cross-compilation for a specific architecture is about to begin.
			It's unclear what the intended purpose of this is, because even after looking in the ndk-build's Makefile, it seems that the return
			value of this command is stored in a variable and then never reused anywhere in the code.
			
			+ https://android.googlesource.com/platform/ndk/+/3cb267487b9a7c1b5956f4239ddd15095630041a/build/core/setup-toolchain.mk#107
			
			Since that seems to be a completely useless call and has nothing to do with the compilation step, we will just ignore it.
			*/
			goto end;
		}
		
		next:;
		
		 if (clang_specific_remove(prev, cur)) {
			if (prev != NULL && strcmp(prev, GCC_OPT_XLINKER) == 0) {
				prev = NULL;
				kargv[--kargc] = (char*) prev;
			}
			
			continue;
		}
		
		prev = cur;
		
		if (clang_specific_replace(cur, &kargc, kargv)) {
			continue;
		}
		
		kargv[kargc++] = (char*) cur;
	}
	
	if (address_sanitizer) {
		wants_libcxx = 1;
		wants_libgcc = 1;
	}
	
	if (wants_libcxx || wants_libitm || wants_libgomp) {
		wants_libgcc = 1;
	}
	
	if (stack_protector) {
		wants_libssp = 1;
	}
	
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
	
	/* Get the directory where our executable is (i.e., <prefix>/bin) */
	get_parent_path(app_filename, parent_directory, 1);
	
	/*
	Check if we are dealing with a non-triplet-prefixed executable name.
	*/
	if (known_compiler(file_name)) {
		override_cc = file_name;
		
		/*
		If we are acting as a wrapper for Clang and are inside CMake, trick CMake into thinking we are Clang.
		
		This is required because otherwise CMake won't pass the --target flag to the
		C/C++ compiler invocation during cross-compilation since it's a Clang-specific thing.
		This flag is the only way for us to know which cross-compilation platform we are targeting.
		
		This spoofing is only applied during the initial C/C++ compiler detection.
		*/
		if (known_clang(override_cc) && cmake_init) {
			kargv[kargc++] = (char*) GCC_OPT_U_GNUC;
			kargv[kargc++] = (char*) GCC_OPT_D_CLANG;
			kargv[kargc++] = (char*) GCC_OPT_D_CLANG_MAJOR;
			kargv[kargc++] = (char*) GCC_OPT_D_CLANG_MINOR;
			kargv[kargc++] = (char*) GCC_OPT_D_CLANG_PATCHLEVEL;
		}
	}
	
	cc = override_cc;
	
	/*
	Check if we are dealing with a triplet-prefixed executable name (<triplet><libc_version>-<cc>),
	and if so, try to extract the compiler name from it.
	*/
	if (cc == NULL) {
		ptr = strchr(file_name, '\0');
		
		while (1) {
			const unsigned char ch = *ptr;
			
			/* We have traversed the entire string but could not find the compiler name. */
			if (ptr == file_name) {
				err = ERR_UNKNOWN_COMPILER;
				goto end;
			}
			
			if (ch == '-') {
				break;
			}
			
			ptr--;
		}
		
		cc = (ptr + 1);
		
		if (!known_compiler(cc)) {
			err = ERR_UNKNOWN_COMPILER;
			goto end;
		}
	}
	
	wants_libcxx += (strcmp(cc, GPP) == 0 || strcmp(cc, CPP) == 0 || strcmp(cc, GM2) == 0 || strcmp(cc, CLANGPP) == 0);
	
	if (wants_libcxx) {
		wants_libgcc = 1;
	}
	
	/*
	Check if the user is calling the wrapper without a properly triplet-prefixed executable name.
	If so, it's expected that the user provide the triplet through the --target=<value> flag instead.
	*/
	if (known_compiler(file_name) && override_triplet == NULL) {
		if (!(override_cc != NULL && known_clang(override_cc) && cmake_init)) {
			err = ERR_BAD_TRIPLET;
			goto end;
		}
		
		/*
		CMake won't pass the --target flag to the C/C++ compiler invocation during the initial compiler
		testing/detection, so for now we must pick a random supported target.
		*/
		override_triplet = (char*) DEFAULT_TARGET;
	}
	
	if (override_triplet != NULL) {
		file_name = override_triplet;
	}
	
	ptr = file_name;
	
	/* Attempt to extract the system/libc version from the target triplet. */
	while (1) {
		const unsigned char a = *ptr;
		const unsigned char b = *(ptr + 1);
		
		/* We reached the end of the string without finding it. */
		if (a == '\0') {
			err = ERR_UNKNOWN_SYSTEM_VERSION;
			goto end;
		}
		
		#if defined(OBGGCC) /* Linux glibc (e.g., 2.17) */
			if (a == '2' && (b == '.' || b == '-' || b == '\0')) {
				break;
			}
		#elif defined(PINO) /* Android API level (e.g., 21) */
			if ((a >= '2' && a <= '3') && (b >= '0' && b <= '9')) {
				break;
			}
		#else
			#error "I don't know how to handle this"
		#endif
		
		ptr++;
	}
	
	size = (size_t) (ptr - file_name);
	
	triplet = malloc(size + 1);
	
	if (triplet == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	memcpy(triplet, file_name, size);
	triplet[size] = '\0';
	
	/* Pick a fast linker if available. */
	if (!override_linker && (cur = get_fast_linker(parent_directory, triplet)) != NULL) {
		linker = malloc(strlen(GCC_OPT_F_USE_LD) + strlen(cur) + 1);
		
		if (linker == NULL) {
			err = ERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(linker, GCC_OPT_F_USE_LD);
		strcat(linker, cur);
		
		kargv[kargc++] = linker;
	}
	
	#if defined(OBGGCC)
		/* Atomics are not natively supported on SPARC, so we need to rely on -latomic. */
		require_atomic_library = strcmp(triplet, "sparc-unknown-linux-gnu") == 0;
	#endif
	
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
		
		if (a == '-' || a == '\0') {
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
	
	if (!(*ptr == '-' || *ptr == '\0')) {
		libc_minor = strtol(ptr + 1, NULL, 16);
	}
	
	#if defined(OBGGCC)
		/* Determine whether we need to implicit link with -lrt */
		wants_librt = wants_libcxx && !have_rt_library && LIBC_VERSION(libc_major, libc_minor) < LIBC_VERSION(2, 17);
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
	
	if (strcmp(cc, CLANG) == 0) {
		strcat(executable, GCC);
	} else if (strcmp(cc, CLANGPP) == 0) {
		strcat(executable, GPP);
	} else {
		strcat(executable, cc);
	}
	
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
	
	kargv[kargc++] = NULL;
	
	size = kargc + 13 + (size_t) wants_librt;
	
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
	
	#if defined(OBGGCC)
		if (require_atomic_library) {
			size += 1;
		}
	#endif
	
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
	
	gpp_include_directory = malloc(strlen(parent_directory) + strlen(PATHSEP_S) + strlen(triplet) + strlen(INCLUDE_DIR) + strlen(PATHSEP_S) + strlen(CPP) + strlen(PATHSEP_S) + strlen(GCC_MAJOR_VERSION) + 1);
	
	if (gpp_include_directory == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(gpp_include_directory, parent_directory);
	strcat(gpp_include_directory, PATHSEP_S);
	strcat(gpp_include_directory, triplet);
	strcat(gpp_include_directory, INCLUDE_DIR);
	strcat(gpp_include_directory, PATHSEP_S);
	strcat(gpp_include_directory, CPP);
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
	
	arg = malloc(strlen(GCC_OPT_SYSROOT) + strlen(EQUAL_S) + strlen(sysroot_directory) + 1);
	
	if (arg == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(arg, GCC_OPT_SYSROOT);
	strcat(arg, EQUAL_S);
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
	
	if (strcmp(cc, GPP) == 0 || strcmp(cc, CPP) == 0 || strcmp(cc, CLANGPP) == 0) {
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
	
	if (wants_librt) {
		args[offset++] = (char*) GCC_OPT_L_RT;
	}
	
	#if defined(OBGGCC)
		if (require_atomic_library) {
			args[offset++] = (char*) GCC_OPT_L_ATOMIC;
		}
	#endif
	
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
	
	memcpy(&args[offset], &kargv[1], kargc * sizeof(*kargv));
	
	if (verbose) {
		fprintf(stderr, "%s", "+ ");
		
		for (index = 0; 1; index++) {
			cur = args[index];
			
			if (cur == NULL) {
				break;
			}
			
			if (index != 0) {
				fprintf(stderr, "%s", " ");
			}
			
			fprintf(stderr, "%s", cur);
		}
		
		fprintf(stderr, "%s", "\n");
	}
	
	#if defined(PINO)
		/*
		In the context of CMake and ndk-build in Android, the NDK provides only a single static library for most components.
		Anything other than the Bionic and OpenGL libraries (i.e., Clang runtime libraries) is statically linked by default, with no option
		to use the shared variant. The only time you're allowed to choose between static and shared is when linking with the libc++ library.
		In that case, the Android Gradle plugin is the one in charge of copying the libc++ library to the APK when you choose the shared library
		over the static one.
		
		In the context of Pino, we provide shared versions of all libraries, and we prefer to link to them over using the static variants when possible.
		Since the Gradle plugin is unaware of our preferences, we cannot rely on it to copy the libraries to the APK, so we do it ourselves.
		
		FIXME: Figure out a way to detect when we are not being invoked by Gradle and avoid copying the libraries when we are not building APKs.
		*/
		if (!wants_disable_shlib && known_clang(cc) && output_directory != NULL) {
			/* libatomic */
			err = copy_shared_library(sysroot_library_directory, output_directory, LIBATOMIC_SHARED, LIBATOMIC_SHARED);
			
			if (err != ERR_SUCCESS) {
				goto end;
			}
			
			/* libstdc++ */
			if (wants_libcxx) {
				err = copy_shared_library(sysroot_library_directory, output_directory, LIBSTDCXX_SHARED, LIBSTDCXX_SHARED);
				
				if (err != ERR_SUCCESS) {
					goto end;
				}
			}
			
			/* libgcc */
			if (wants_libgcc) {
				err = copy_shared_library(sysroot_library_directory, output_directory, LIBGCC_SHARED, LIBGCC_SHARED);
				
				if (err != ERR_SUCCESS) {
					goto end;
				}
			}
			
			/* asan/hwasan */
			if (address_sanitizer) {
				err = copy_shared_library(sysroot_library_directory, output_directory, LIBASAN_SHARED, LIBASAN_SHARED);
				
				if (err != ERR_SUCCESS) {
					goto end;
				}
				
				err = copy_shared_library(sysroot_library_directory, output_directory, LIBHWASAN_SHARED, LIBHWASAN_SHARED);
				
				if (err != ERR_SUCCESS) {
					goto end;
				}
				
				err = copy_shared_library(sysroot_library_directory, output_directory, LIBLSAN_SHARED, LIBLSAN_SHARED);
				
				if (err != ERR_SUCCESS) {
					goto end;
				}
				
				err = copy_shared_library(sysroot_library_directory, output_directory, LIBTSAN_SHARED, LIBTSAN_SHARED);
				
				if (err != ERR_SUCCESS) {
					goto end;
				}
				
				err = copy_shared_library(sysroot_library_directory, output_directory, LIBUBSAN_SHARED, LIBUBSAN_SHARED);
				
				if (err != ERR_SUCCESS) {
					goto end;
				}
			}
			
			/* libgomp */
			if (wants_libgomp) {
				err = copy_shared_library(sysroot_library_directory, output_directory, LIBGOMP_SHARED, LIBGOMP_SHARED);
				
				if (err != ERR_SUCCESS) {
					goto end;
				}
			}
			
			/* libitm */
			if (wants_libitm) {
				err = copy_shared_library(sysroot_library_directory, output_directory, LIBITM_SHARED, LIBITM_SHARED);
				
				if (err != ERR_SUCCESS) {
					goto end;
				}
			}
			
			/* libssp */
			if (wants_libssp) {
				err = copy_shared_library(sysroot_library_directory, output_directory, LIBSSP_SHARED, LIBSSP_SHARED);
				
				if (err != ERR_SUCCESS) {
					goto end;
				}
			}
		}
	#endif
	
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
	free(output_directory);
	free(args);
	free(kargv);
	free(arg);
	free(linker);
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
			opt = "Unknown C/C++ compiler";
			break;
		case ERR_GET_APP_FILENAME_FAILURE:
			opt = "Could not get app filename";
			break;
		case ERR_EXECVE_FAILURE:
			opt = "Call to execve failed";
			break;
		case ERR_BAD_TRIPLET:
			opt = "The target triplet is invalid or was not recognized";
			break;
		case ERR_GETEXT_FAILURE:
			opt = "Could not get file extension of object file";
			break;
		case ERR_COPY_FILE_FAILURE:
			opt = "Could not copy file";
			break;
		case ERR_UNKNOWN_SYSTEM_VERSION:
			opt = "The system or libc version provided through the target triplet is invalid or was not recognized";
			break;
		default:
			opt = "Unknown error";
			break;
	}
	
	if (err != ERR_SUCCESS) {
		cur = strerror(errno);
		
		fprintf(stderr, "fatal error: %s", opt);
		
		switch (err) {
			case ERR_EXECVE_FAILURE:
			case ERR_COPY_FILE_FAILURE:
			case ERR_GET_APP_FILENAME_FAILURE:
			case ERR_MEM_ALLOC_FAILURE:
				fprintf(stderr, ": %s", cur);
				break;
		}
		
		fprintf(stderr, "\n");
		
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
	
}
