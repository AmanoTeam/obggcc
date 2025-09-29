#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <unistd.h>

#if defined(__ANDROID__)
	#include <android/api-level.h>
#endif

#if defined(__GLIBC__)
	#include <gnu/libc-version.h>
#endif

#include "fs/getexec.h"
#include "fs/ext.h"
#include "fs/dirname.h"
#include "fs/parentpath.h"
#include "fs/sep.h"
#include "fs/cp.h"
#include "fs/exists.h"
#include "fs/basename.h"
#include "os/find_exe.h"
#include "biggestint.h"
#include "clang_option.h"
#include "errors.h"
#include "obggcc.h"
#include "query.h"
#include "strsplit.h"

#if !defined(AUTO_PICK_LINKER)
	#define AUTO_PICK_LINKER 1
#endif

static const char GCC_MAJOR_VERSION[] = "15";

static const char INCLUDE_DIR[] = PATHSEP_M "include";
static const char INCLUDE_MISSING_DIR[] = PATHSEP_M "include-missing";
static const char LIBRARY_DIR[] = PATHSEP_M "lib";
static const char STATIC_LIBRARY_DIR[] = PATHSEP_M "static";
static const char LDSCRIPTS_DIR[] = PATHSEP_M "ldscripts";

static const char GCC_LIBRARY_DIR[] = PATHSEP_M "lib" PATHSEP_M "gcc";

static const char RT_LIBRARY[] = "rt";
static const char STDCXX_LIBRARY[] = "stdc++";
static const char ATOMIC_LIBRARY[] = "atomic";
static const char GOMP_LIBRARY[] = "gomp";
static const char ITM_LIBRARY[] = "itm";
static const char QUADMATH_LIBRARY[] = "quadmath";
static const char MATH_LIBRARY[] = "m";

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
static const char LIBQUADMATH_SHARED[] = "libquadmath.so";

static const char LIBASAN_SHARED[] = "libasan.so";
static const char LIBHWASAN_SHARED[] = "libhwasan.so";
static const char LIBLSAN_SHARED[] = "liblsan.so";
static const char LIBTSAN_SHARED[] = "libtsan.so";
static const char LIBUBSAN_SHARED[] = "libubsan.so";

static const char GCC_OPT_ISYSTEM[] = "-isystem";
static const char GCC_OPT_SYSROOT[] = "--sysroot";
static const char GCC_OPT_NOSTDINC[] = "--no-standard-includes";
static const char GCC_OPT_LIBDIR[] = "-L";
static const char GCC_OPT_STATIC[] = "-static";
static const char GCC_OPT_STATIC_LIBCXX[] = "-static-libstdc++";
static const char GCC_OPT_STATIC_LIBGCC[] = "-static-libgcc";
static const char GCC_OPT_STATIC_LIBASAN[] = "-static-libasan";
static const char GCC_OPT_STATIC_LIBTSAN[] = "-static-libtsan";
static const char GCC_OPT_STATIC_LIBLSAN[] = "-static-liblsan";
static const char GCC_OPT_STATIC_LIBUBSAN[] = "-static-libubsan";
static const char GCC_OPT_STATIC_LIBHWASAN[] = "-static-libhwasan";

static const char GCC_OPT_VERSION[] = "--version";
static const char GCC_OPT_L[] = "-l";
static const char GCC_OPT_V[] = "-v";
static const char GCC_OPT_D[] = "-D";
static const char GCC_OPT_O[] = "-o";
static const char GCC_OPT_C[] = "-c";
static const char GCC_OPT_R[] = "-r";
static const char GCC_OPT_S[] = "-S";
static const char GCC_OPT_E[] = "-E";
static const char GCC_OPT_M[] = "-M";
static const char GCC_OPT_MM[] = "-MM";
static const char GCC_OPT_B[] = "-B";
static const char GCC_OPT_SHARED[] = "-shared";
static const char GCC_OPT_F_SYNTAX_ONLY[] = "-fsyntax-only";
static const char GCC_OPT_OS[] = "-Os";
static const char GCC_OPT_L_RT[] = "-lrt";
static const char GCC_OPT_FSANITIZE[] = "-fsanitize=";
static const char GCC_OPT_L_STDCXX[] = "-lstdc++";
static const char GCC_OPT_L_ATOMIC[] = "-latomic";
static const char GCC_OPT_L_GOMP[] = "-lgomp";
static const char GCC_OPT_L_ITM[] = "-litm";
static const char GCC_OPT_L_QUADMATH[] = "-lquadmath";
static const char GCC_OPT_L_MATH[] = "-lm";
static const char GCC_OPT_XLINKER[] = "-Xlinker";
static const char GCC_OPT_WL[] = "-Wl,";
static const char GCC_OPT_F_FAT_LTO_OBJECTS[] = "-ffat-lto-objects";
static const char GCC_OPT_F_NO_FAT_LTO_OBJECTS[] = "-fno-fat-lto-objects";
static const char GCC_OPT_F_LTO_PARTITION_NONE[] = "-flto-partition=none";
static const char GCC_OPT_F_LTO_PARTITION_BALANCED[] = "-flto-partition=balanced";
static const char GCC_OPT_F_DEVIRTUALIZE_AT_LTRANS[] = "-fdevirtualize-at-ltrans";
static const char GCC_OPT_F_LTO_COMPRESSION_LEVEL_ZERO[] = "-flto-compression-level=0";

static const char GCC_OPT_F_LTO[] = "-flto";
static const char GCC_OPT_F_LTO_AUTO[] = "-flto=auto";
static const char GCC_OPT_F_USE_LD[] = "-fuse-ld=";
static const char GCC_OPT_F_GNU_TM[] = "-fgnu-tm";
static const char GCC_OPT_F_OPENMP[] = "-fopenmp";
static const char GCC_OPT_F_OPENACC[] = "-fopenacc";
static const char GCC_OPT_NO_PIE[] = "-no-pie";
static const char GCC_OPT_M_FPU[] = "-mfpu=";
static const char GCC_OPT_M_SSE3[] = "-msse3";
static const char GCC_OPT_M_SSE4_2[] = "-msse4.2";
static const char GCC_OPT_M_ARM[] = "-marm";

static const char GCC_OPT_DFORTIFY_SOURCE[] = "-D_FORTIFY_SOURCE=";
static const char GCC_OPT_U_GNUC[] = "-U__GNUC__";

static const char GCC_OPT_D_CLANG[] = "-D__clang__=1";
static const char GCC_OPT_D_CLANG_MAJOR[] = "-D__clang_major__=21";
static const char GCC_OPT_D_CLANG_MINOR[] = "-D__clang_minor__=0";
static const char GCC_OPT_D_CLANG_PATCHLEVEL[] = "-D__clang_patchlevel__=0";

static const char GCC_OPT_F_STACK_PROTECTOR[] = "-fstack-protector";

static const char GCC_OPT_NODEFAULTLIBS[] = "-nodefaultlibs";
static const char GCC_OPT_NOSTDLIB[] = "-nostdlib";
static const char GCC_OPT_WERROR[] = "-Werror";
static const char GCC_OPT_WNO_ERROR[] = "-Wno-error";
static const char GCC_OPT_F_TREE_VECTORIZE[] = "-ftree-vectorize";

static const char GCC_M_ANDROID_VERSION_MIN[] = "-mandroid-version-min=";

static const char CLANG_OPT_OZ[] = "-Oz";
static const char CLANG_OPT_ICF[] = "--icf";
static const char CLANG_OPT_TARGET[] = "--target";
static const char CLANG_OPT_Q_UNUSED_ARGUMENTS[] = "-Qunused-arguments";
static const char CLANG_OPT_W_UNUSED_COMMAND_LINE_ARGUMENT[] = "-Wunused-command-line-argument";
static const char CLANG_OPT_W_INVALID_COMMAND_LINE_ARGUMENT[] = "-Winvalid-command-line-argument";
static const char CLANG_OPT_W_NEWLINE_EOF[] = "-Wnewline-eof";
static const char CLANG_OPT_W_UNGUARDED_AVAILABILITY[] = "-Wunguarded-availability";
static const char CLANG_OPT_W_UNGUARDED_AVAILABILITY_NEW[] = "-Wunguarded-availability-new";
static const char CLANG_OPT_PRINT_RESOURCE_DIR[] = "-print-resource-dir";
static const char CLANG_OPT_F_NO_LIMIT_DEBUG_INFO[] = "-fno-limit-debug-info";
static const char CLANG_OPT_GCC_TOOLCHAIN[] = "--gcc-toolchain";
static const char CLANG_OPT_F_COLOR_DIAGNOSTICS[] = "-fcolor-diagnostics";
static const char CLANG_OPT_F_NO_INTEGRATED_AS[] = "-fno-integrated-as";
static const char CLANG_OPT_F_INTEGRATED_AS[] = "-fintegrated-as";
static const char CLANG_OPT_F_SLP_VECTORIZE_AGGRESSIVE[] = "-fslp-vectorize-aggressive";

#define LTO_NONE 0x00
#define LTO_FULL 0x01
#define LTO_THIN 0x02

#define ARCH_ABI_32 0x20
#define ARCH_ABI_64 0x40

#define ARCH_SPEC_NONE 0x00
#define ARCH_SPEC_ARM 0x01
#define ARCH_SPEC_X86 0x02
#define ARCH_SPEC_MIPS 0x03
#define ARCH_SPEC_RISCV 0x04

static const char LD_OPT_DYNAMIC_LINKER[] = "-dynamic-linker";
static const char LD_OPT_RPATH_LINK[] = "-rpath-link";
static const char LD_OPT_RPATH[] = "-rpath";
static const char LD_OPT_UNRESOLVED_SYMBOLS[] = "--unresolved-symbols=ignore-in-shared-libs";
static const char LD_OPT_NO_ROSEGMENT[] = "--no-rosegment";
static const char LD_OPT_Z[] = "-z";
static const char LD_OPT_PACK_RELATIVE_RELOCS[] = "pack-relative-relocs";

static const char LLD_OPT_USE_ANDROID_RELR_TAGS[] = "--use-android-relr-tags";
static const char LLD_OPT_PACK_DYN_RELOCS[] = "--pack-dyn-relocs=relr";

static const char M_ANDROID_API[] = "__ANDROID_API__=";
static const char M_ANDROID_MIN_SDK_VERSION[] = "__ANDROID_MIN_SDK_VERSION__=";
static const char M_ANDROID_UNAVAILABLE_SYMBOLS_ARE_WEAK[] = "__ANDROID_UNAVAILABLE_SYMBOLS_ARE_WEAK__";

static const char GCC_FPU_NEON[] = "neon-vfpv3";

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

static const char NDK_CXX_STL_DIRECTORY[] = PATHSEP_M "sources" PATHSEP_M "cxx-stl";
static const char NDK_SYSROOT_INCLUDE_DIRECTORY[] = PATHSEP_M "sysroot" PATHSEP_M "usr" PATHSEP_M "include";
static const char NDK_SYSROOT_LIBRARY_DIRECTORY[] = PATHSEP_M "prebuilt" PATHSEP_M "linux-x86_64" PATHSEP_M "lib" PATHSEP_M "gcc";

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

static const char DASH = '-';
static const char EQUAL = '=';
static const char ZERO = '\0';
static const char EQUAL_S[] = "=";

extern char** environ;

#if AUTO_PICK_LINKER
static const char* const FASTER_LINKERS[] = {
	"mold",
	"lld",
	"gold"
};
#endif

static const char CLANG_VERSION_TEMPLATE[] = 
	"clang version 21.0.0\n"
	"Target: %s\n"
	"Thread model: posix\n"
	"InstalledDir: %s\n";

static const char CLANG_WARNING_REMOVE_NONE[] = "none";

static clang_option_t CLANG_SPECIFIC_REMOVE[] = {
	{
		.name = CLANG_OPT_Q_UNUSED_ARGUMENTS,
		.value = 0
	},
	{
		.name = CLANG_OPT_W_UNUSED_COMMAND_LINE_ARGUMENT,
		.value = 0
	},
	{
		.name = CLANG_OPT_W_INVALID_COMMAND_LINE_ARGUMENT,
		.value = 0
	},
	{
		.name = CLANG_OPT_W_UNGUARDED_AVAILABILITY,
		.value = 0
	},
	{
		.name = CLANG_OPT_W_UNGUARDED_AVAILABILITY_NEW,
		.value = 0
	},
	{
		.name = CLANG_OPT_ICF,
		.value = 1
	},
	{
		.name = CLANG_OPT_F_NO_LIMIT_DEBUG_INFO,
		.value = 0
	},
	{
		.name = CLANG_OPT_GCC_TOOLCHAIN,
		.value = 1
	},
	{
		.name = CLANG_OPT_W_NEWLINE_EOF,
		.value = 0
	},
	{
		.name = CLANG_OPT_F_COLOR_DIAGNOSTICS,
		.value = 0
	},
	{
		.name = CLANG_OPT_F_NO_INTEGRATED_AS,
		.value = 0
	},
	{
		.name = CLANG_OPT_F_INTEGRATED_AS,
		.value = 0
	},
#if defined(PINO)
	{
		.name = GCC_OPT_STATIC,
		.value = 0
	}
#endif
};

#define CLANG_SPECIFIC_REMOVE_NON 0
#define CLANG_SPECIFIC_REMOVE_CUR 1
#define CLANG_SPECIFIC_REMOVE_NXT 2

#define LIBC_VERSION(major, minor) ((major << 16) + minor)

static int libcv_matches(const char a, const char b) {
	
	#if defined(OBGGCC) /* Linux glibc (e.g., 2.17) */
		if (a == '2' && (b == '.' || b == '-' || b == ZERO)) {
			return 1;
		}
	#elif defined(PINO) /* Android API level (e.g., 14) */
		if ((a >= '1' && a <= '3') && (b >= '0' && b <= '9')) {
			return 1;
		}
	#else
		#error "I don't know how to handle this"
	#endif
	
	return 0;
	
}

static long int* get_libc_version_int(const char* const string, long int value[2]) {
	
	char* ptr = NULL;
	
	long int libc_major = 0;
	long int libc_minor = 0;
	
	libc_major = strtol(string, &ptr, 10);
	
	if (!(*ptr == '-' || *ptr == ZERO)) {
		libc_minor = strtol(ptr + 1, NULL, 10);
	}
	
	value[0] = libc_major;
	value[1] = libc_minor;
	
	return value;
	
}

static int get_bitness(const char* const triplet) {
	
	int status = 0;
	
	#if defined(PINO)
		status = (
			strcmp(triplet, "riscv64-unknown-linux-android") == 0 ||
			strcmp(triplet, "aarch64-unknown-linux-android") == 0 ||
			strcmp(triplet, "x86_64-unknown-linux-android") == 0 ||
			strcmp(triplet, "mips64el-unknown-linux-android") == 0
		);
		
		if (status) {
			return ARCH_ABI_64;
		}
		
		status = (
			strcmp(triplet, "mipsel-unknown-linux-android") == 0 ||
			strcmp(triplet, "i686-unknown-linux-android") == 0 ||
			strcmp(triplet, "armv7-unknown-linux-androideabi") == 0 ||
			strcmp(triplet, "armv5-unknown-linux-androideabi") == 0
		);
		
		if (status) {
			return ARCH_ABI_32;
		}
	#elif defined(OBGGCC)
		status = (
			strcmp(triplet, "x86_64-unknown-linux-gnu") == 0 ||
			strcmp(triplet, "aarch64-unknown-linux-gnu") == 0
		);
		
		if (status) {
			return ARCH_ABI_64;
		}
		
		status = (
			strcmp(triplet, "arm-unknown-linux-gnueabi") == 0 ||
			strcmp(triplet, "arm-unknown-linux-gnueabihf") == 0 ||
			strcmp(triplet, "i386-unknown-linux-gnu") == 0
		);
		
		if (status) {
			return ARCH_ABI_32;
		}
	#else
		#error "I don't know how to handle this"
	#endif
	
	return 0;
	
}

static int get_arch(const char* const triplet) {
	
	int status = 0;
	
	#if defined(PINO)
		status = (strcmp(triplet, "riscv64-unknown-linux-android") == 0);
		
		if (status) {
			return ARCH_SPEC_RISCV;
		}
		
		status = (
			strcmp(triplet, "x86_64-unknown-linux-android") == 0 ||
			strcmp(triplet, "i686-unknown-linux-android") == 0
		);
		
		if (status) {
			return ARCH_SPEC_X86;
		}
		
		status = (
			strcmp(triplet, "aarch64-unknown-linux-android") == 0 ||
			strcmp(triplet, "armv7-unknown-linux-androideabi") == 0 ||
			strcmp(triplet, "armv5-unknown-linux-androideabi") == 0
		);
		
		if (status) {
			return ARCH_SPEC_ARM;
		}
		
		status = (
			strcmp(triplet, "mipsel-unknown-linux-android") == 0 ||
			strcmp(triplet, "mips64el-unknown-linux-android") == 0
		);
		
		if (status) {
			return ARCH_SPEC_MIPS;
		}
	#elif defined(OBGGCC)
		status = (
			strcmp(triplet, "x86_64-unknown-linux-gnu") == 0 ||
			strcmp(triplet, "i386-unknown-linux-gnu") == 0
		);
		
		if (status) {
			return ARCH_SPEC_X86;
		}
		
		status = (
			strcmp(triplet, "aarch64-unknown-linux-gnu") == 0 ||
			strcmp(triplet, "arm-unknown-linux-gnueabi") == 0 ||
			strcmp(triplet, "arm-unknown-linux-gnueabihf") == 0
		);
		
		if (status) {
			return ARCH_SPEC_ARM;
		}
		
		status = (
			strcmp(triplet, "mips-unknown-linux-gnu") == 0 ||
			strcmp(triplet, "mips64el-unknown-linux-gnuabi64") == 0 ||
			strcmp(triplet, "mipsel-unknown-linux-gnu") == 0
		);
		
		if (status) {
			return ARCH_SPEC_MIPS;
		}
	#else
		#error "I don't know how to handle this"
	#endif
	
	return ARCH_SPEC_NONE;
	
}

static int target_supports_neon(const char* const name) {
	
	#if defined(PINO)
		if (strcmp(name, "armv7-unknown-linux-androideabi") == 0) {
			return 1;
		}
	#elif defined(OBGGCC)
		if (strcmp(name, "arm-unknown-linux-gnueabihf") == 0) {
			return 1;
		}
	#else
		#error "I don't know how to handle this"
	#endif
	
	return 0;
	
}

static int check_linker_lld(const char* const name) {
	return name != NULL && strcmp(name, "lld") == 0;
}

static int check_linker_mold(const char* const name) {
	return name != NULL && strcmp(name, "mold") == 0;
}


static int target_supports_relr(const char* const name, const char* const linker, const int version) {
	
	#if defined(OBGGCC)
		/* The GNU C Library has supported DT_RELR since the 2.36 release. */
		if (version < LIBC_VERSION(2, 36)) {
			return 0;
		}
		
		if (strcmp(name, "x86_64-unknown-linux-gnu") == 0) {
			return 1;
		}
		
		if (strcmp(name, "i386-unknown-linux-gnu") == 0) {
			return 1;
		}
		
		if (strcmp(name, "aarch64-unknown-linux-gnu") == 0) {
			return 1;
		}
		
		if (check_linker_lld(linker) || check_linker_mold(linker)) {
			if (strcmp(name, "arm-unknown-linux-gnueabihf") == 0) {
				return 1;
			}
		}
	#elif defined(PINO)
		/* Not handled */
	#else
		#error "I don't know how to handle this"
	#endif
	
	return 0;
	
}

static const char* get_simd(const char* const name) {
	
	if (strcmp(name, "i686-unknown-linux-android") == 0 || strcmp(name, "i386-unknown-linux-gnu") == 0) {
		return GCC_OPT_M_SSE3;
	}
	
	if (strcmp(name, "x86_64-unknown-linux-android") == 0 || strcmp(name, "x86_64-unknown-linux-gnu") == 0) {
		return GCC_OPT_M_SSE4_2;
	}
	
	return NULL;
	
}

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

static int get_host_version(void) {
	/*
	Get the system/libc version of the host system.
	*/
	
	long int libc_major = 0;
	long int libc_minor = 0;
	
	#if defined(__ANDROID__)
		libc_major = android_get_device_api_level();
		
		if (libc_major == -1) {
			libc_major = 0;
		}
	#elif defined(__GLIBC__)
		const char* string = gnu_get_libc_version();
		long int version[2];
		
		get_libc_version_int(string, version);
		
		libc_major = version[0];
		libc_minor = version[1];
	#endif
	
	return LIBC_VERSION(libc_major, libc_minor);
	
}

static const char* get_host_triplet(void) {
	/*
	Get the triplet of the host system.
	*/
	
	#if defined(__ANDROID__)
		#if defined(__x86_64__)
			return "x86_64-unknown-linux-android";
		#elif defined(__i386__)
			return "i686-unknown-linux-android";
		#elif defined(__ARM_ARCH_5TE__)
			return "armv5-unknown-linux-androideabi";
		#elif defined(__ARM_ARCH_7A__)
			return "armv7-unknown-linux-androideabi";
		#elif defined(__aarch64__)
			return "aarch64-unknown-linux-android";
		#elif defined(__riscv)
			return "riscv64-unknown-linux-android";
		#elif defined(__mips64)
			return "mips64el-unknown-linux-android";
		#elif defined(__mips__)
			return "mipsel-unknown-linux-android";
		#endif
	#elif defined(__GLIBC__)
		#if defined(__x86_64__)
			return "x86_64-unknown-linux-gnu";
		#elif defined(__i386__)
			return "i386-unknown-linux-gnu";
		#elif defined(__ARM_ARCH_4T__)
			return "arm-unknown-linux-gnueabi";
		#elif defined(__ARM_ARCH_7A__)
			return "armv7-unknown-linux-androideabi";
		#elif defined(__aarch64__)
			return "aarch64-unknown-linux-gnu";
		#endif
	#endif
	
	return NULL;
	
}

static const char* get_max_libc_version(const char* const triplet) {
	
	int status = 0;
	
	#if defined(PINO)
		status = (
			strcmp(triplet, "riscv64-unknown-linux-android") == 0 ||
			strcmp(triplet, "i686-unknown-linux-android") == 0 ||
			strcmp(triplet, "armv7-unknown-linux-androideabi") == 0 ||
			strcmp(triplet, "aarch64-unknown-linux-android") == 0 ||
			strcmp(triplet, "x86_64-unknown-linux-android") == 0
		);
		
		if (status) {
			return "35";
		}
		
		status = (
			strcmp(triplet, "mipsel-unknown-linux-android") == 0 ||
			strcmp(triplet, "mips64el-unknown-linux-android") == 0 ||
			strcmp(triplet, "armv5-unknown-linux-androideabi") == 0
		);
		
		if (status) {
			return "27";
		}
	#endif
	
	return NULL;
	
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

static char* clang_warning_remove(const char* const cur) {
	/*
	Remove Clang-specific warning options that have no GCC equivalents.
	*/
	
	int remove = 0;
	
	size_t index = 0;
	
	size_t* items = NULL;
	size_t items_offset = 0;
	
	size_t warnc = 0;
	
	strsplit_t split = {0};
	strsplit_part_t part = {0};
	
	char* wopt = NULL;
	
	const char* werror = GCC_OPT_WERROR + 1;
	size_t size = 0;
	
	const char* current = cur;
	const char* pos = NULL;
	
	const char separator[] = {EQUAL, '\0'};
	
	const char* name = cur;
	int value = 0;
	const clang_option_t* option = NULL;
	
	while (*(++current) == DASH) {};
	
	size = strlen(werror);
	
	if (strncmp(current, werror, size) != 0) {
		werror = GCC_OPT_WNO_ERROR + 1;
	}
	
	size = strlen(werror);
	
	if (strncmp(current, werror, size) != 0) {
		return (char*) CLANG_WARNING_REMOVE_NONE;
	}
	
	current += size;
	
	if (*current != EQUAL) {
		return (char*) CLANG_WARNING_REMOVE_NONE;
	}
	
	current++;
	
	strsplit_init(&split, &part, current, ",");
	
	warnc = strsplit_size(&split, &part);
	
	items = malloc(warnc * sizeof(*items));
	
	for (index = 0; index < sizeof(CLANG_SPECIFIC_REMOVE) / sizeof(*CLANG_SPECIFIC_REMOVE); index++) {
		option = &CLANG_SPECIFIC_REMOVE[index];
		
		name = option->name;
		value = option->value;
		
		while (*(++name) == DASH) {};
		
		if (name[0] != 'W') {
			continue;
		}
		
		name++;
		
		size = strlen(name);
		
		strsplit_init(&split, &part, current, ",");
		
		while (strsplit_next(&split, &part) != NULL) {
			if (!(size == part.size && strncmp(part.begin, name, part.size) == 0)) {
				continue;
			}
			
			items[items_offset++] = part.index;
		}
	}
	
	if (items_offset == 0) {
		return (char*) CLANG_WARNING_REMOVE_NONE;
	}
	
	if (items_offset == warnc) {
		return NULL;
	}
	
	wopt = malloc(strlen(cur) + 1);
	
	if (wopt == NULL) {
		return (char*) CLANG_WARNING_REMOVE_NONE;
	}
	
	wopt[0] = DASH;
	
	strcpy(wopt + 1, werror);
	strcat(wopt, separator);

	strsplit_init(&split, &part, current, ",");
	
	while (strsplit_next(&split, &part) != NULL) {
		remove = 0;
		
		if (part.size == 0) {
			continue;
		}
		
		for (index = 0; index < items_offset; index++) {
			remove = (items[index] == part.index);
			
			if (remove) {
				break;
			}
		}
		
		if (remove) {
			continue;
		}
		
		pos = strchr(wopt, '\0') - 1;
		
		if (strcmp(pos, separator) != 0) {
			strcat(wopt, ",");
		}
		
		strncat(wopt, part.begin, part.size);
	}
	
	pos = strchr(wopt, '\0') - 1;
	
	if (strcmp(pos, separator) == 0) {
		free(wopt);
		wopt = NULL;
	}

	return wopt;
	
}

static int clang_specific_remove(const char* const prev, const char* const cur) {
	/*
	Remove Clang-specific options that have no GCC equivalents.
	*/
	
	int status = CLANG_SPECIFIC_REMOVE_CUR;
	size_t index = 0;
	
	size_t size = 0;
	
	const char* previous = prev;
	const char* current = cur;
	
	const char* name = cur;
	int value = 0;
	const clang_option_t* option = NULL;
	
	unsigned char ch = 0;
	
	if (strncmp(current, GCC_OPT_WL, strlen(GCC_OPT_WL)) == 0) {
		current += strlen(GCC_OPT_WL);
	}
	
	if (*current != DASH || *current == ZERO) {
		status = CLANG_SPECIFIC_REMOVE_NON;
		goto end;
	}
	
	while (*(++current) == DASH) {};
	
	for (index = 0; index < sizeof(CLANG_SPECIFIC_REMOVE) / sizeof(*CLANG_SPECIFIC_REMOVE); index++) {
		option = &CLANG_SPECIFIC_REMOVE[index];
		
		name = option->name;
		value = option->value;
		
		while (*(++name) == DASH) {};
		
		size = strlen(name);
		
		if (name[0] == 'W' && strncmp(current, "Wno-", 4) == 0 && strcmp(name + 1, current + 4) == 0) {
			goto end;
		}
		
		if (strncmp(current, name, size) != 0) {
			continue;
		}
		
		ch = current[size];
		
		if (!(ch == EQUAL || ch == ZERO)) {
			continue;
		}
		
		if (value && ch == ZERO) {
			status = CLANG_SPECIFIC_REMOVE_NXT;
		}
		
		goto end;
	}
	
	status = CLANG_SPECIFIC_REMOVE_NON;
	
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
		
		if (*current == ZERO) {
			status = 0;
			goto end;
		}
		
		kargv[index++] = (char*) GCC_OPT_F_LTO_AUTO;
		
		if (strcmp(current, "auto") == 0 || strcmp(current, "full") == 0) {
			/* Replace -flto={full,auto} with -flto=auto -flto-partition=none. */
			kargv[index++] = (char*) GCC_OPT_F_LTO_PARTITION_NONE;
		} else {
			/* Replace -flto=thin with -flto=auto -flto-partition=balanced. */
			kargv[index++] = (char*) GCC_OPT_F_LTO_PARTITION_BALANCED;
		}
		
		status = 1;
		goto end;
	} else if (strcmp(current, CLANG_OPT_OZ) == 0 && gcc_version < 12) {
		/* Replace -Oz with -Os. */
		kargv[index++] = (char*) GCC_OPT_OS;
		
		status = 1;
		goto end;
	} else if (strcmp(current, CLANG_OPT_F_SLP_VECTORIZE_AGGRESSIVE) == 0) {
		/* Replace -fslp-vectorize-aggressive with -ftree-vectorize. */
		kargv[index++] = (char*) GCC_OPT_F_TREE_VECTORIZE;
		
		status = 1;
		goto end;
	}
	
	end:;
	
	*kargc = index;
	
	return status;
	
}

#if AUTO_PICK_LINKER
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
	
	if (get_arch(triplet) == ARCH_SPEC_MIPS) {
		return linker;
	}
	
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
#endif

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

int main(int argc, char* argv[]) {
	
	int status = 0;
	int err = ERR_SUCCESS;
	
	size_t size = 0;
	size_t offset = 0;
	size_t index = 0;
	
	long int libc_major = 0;
	long int libc_minor = 0;
	long int version[2];
	
	int arch = 0;
	int bitness = 0;
	
	unsigned char a = 0;
	unsigned char b = 0;
	
	int intercept = 0;
	
	const char* system_prefix = NULL;
	int wants_system_libraries = 0;
	int wants_builtin_loader = 0;
	int wants_runtime_rpath = 0;
	int wants_nz = 0;
	int wants_neon = 0;
	int wants_arm_mode = 0;
	int wants_simd = 0;
	int wants_lto = LTO_NONE;
	
	int nodefaultlibs = 0;
	int address_sanitizer = 0;
	int stack_protector = 0;
	int print_version = 0;
	int verbose = 0;
	int wants_libcxx = 0;
	int wants_static_libcxx = 0;
	int wants_static_libgcc = 0;
	int wants_libatomic = 0;
	int wants_libgomp = 0;
	int wants_libgcc = 0;
	int wants_libitm = 0;
	int wants_libquadmath = 0;
	int wants_librt = 0;
	int wants_libssp = 0;
	int wants_libm = 0;
	int wants_force_static = 0;
	
	hquery_t query = {0};
	
	#if defined(OBGGCC)
		int require_atomic_library = 0;
	#endif
	
	const char* override_linker = NULL;
	
	int cmake_init = 0;
	
	int have_rt_library = 0;
	
	int linking = 1;
	int linking_shared = 0;
	
	char** args = NULL;
	char* arg = NULL;
	
	const char* werror = NULL;
	const char* cc = NULL;
	const char* override_cc = NULL;
	const char* override_libcv = NULL;
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
	char* sysroot_ldscripts_directory = NULL;
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
	
	const char* host = NULL;
	long int host_version = 0;
	long int target_version = 0;
	
	#if defined(PINO)
		int android_weak_api_defs = 0;
		
		char* android_version_min = NULL;
		char* android_current_sdk_version = NULL;
	#endif
	
	int hardcode_system_rpath = 0;
	
	size_t kargc = 0;
	char** kargv = NULL;
	
	#if AUTO_PICK_LINKER
		char* linker = NULL;
	#endif
	
	char* floating_point_unit = NULL;
	
	unsigned char ch = 0;
	
	query_load_environ(&query);
	
	#if defined(OBGGCC)
		wants_builtin_loader = query_get_bool(&query, ENV_BUILTIN_LOADER) == 1;
	#endif
	
	wants_force_static = query_get_bool(&query, ENV_STATIC);
	
	wants_system_libraries = query_get_bool(&query, ENV_SYSTEM_LIBRARIES) == 1;
	wants_nz = query_get_bool(&query, ENV_NZ) == 1;
	wants_runtime_rpath = query_get_bool(&query, ENV_RUNTIME_RPATH) == 1;
	verbose = query_get_bool(&query, ENV_VERBOSE) == 1;
	wants_neon = query_get_bool(&query, ENV_NEON) == 1;
	wants_arm_mode = query_get_bool(&query, ENV_ARM_MODE) == 1;
	wants_simd = query_get_bool(&query, ENV_SIMD) == 1;
	
	cur = query_get_string(&query, ENV_LTO);
	
	if (cur != NULL) {
		if (strcmp(cur, "full") == 0) {
			wants_lto = LTO_FULL;
		} else if (strcmp(cur, "thin") == 0) {
			wants_lto = LTO_THIN;
		}
	}
	
	system_prefix = query_get_string(&query, ENV_SYSTEM_PREFIX);
	
	host = get_host_triplet();
	host_version = get_host_version();
	
	#if defined(__ANDROID__)
		file_name = find_exe("termux-open");
		
		if (file_name != NULL && host_version == 0) {
			/* We are probably running inside termux-docker; assume API level 24 */
			host_version = LIBC_VERSION(24, 0);
		}
		
		/*
		* On Android 7.0 (API level 24) and higher, Termux's C/C++ toolchains hardcode the system library path
		* into every executable built.
		* 
		* On Android 6.0 (API level 23) and below, they default to using LD_LIBRARY_PATH instead.
		*/
		hardcode_system_rpath = (file_name != NULL) && host_version >= LIBC_VERSION(24, 0);
	#endif
	
	free(file_name);
	file_name = NULL;
	
	kargv = malloc(
		sizeof(*argv) * (
			argc + 
			1 + /* -U__GNUC__ */
			1 + /* -D__clang__ */
			1 + /* -D__clang_major__ */
			1 + /* -D__clang_minor__ */
			1 + /* -D__clang_patchlevel__ */
			1 + /* -flto-partition=<...> */
			1 + /* -fuse-ld=<linker> */
			1 + /* -static-libgcc */
			1 + /* -static-libasan */
			1 + /* -static-libtsan */
			1 + /* -static-liblsan */
			1 + /* -static-libubsan */
			1 + /* -static-libhwasan */
			1 + /* -mfpu=<value> */
			1 + /* -msse<version> */
			1 + /* -marm */
			4 + /* -Xlinker -z -Xlinker pack-relative-relocs */
			5 + /* -flto=auto -flto-compression-level=0 -flto-partition={none,balanced} -fno-fat-lto-objects -fdevirtualize-at-ltrans */
			1 /* NULL */
		)
	);
	
	if (kargv == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	for (index = 0; index < (size_t) argc; index++) {
		cur = argv[index];
		
		if (strncmp(cur, GCC_OPT_ISYSTEM, strlen(GCC_OPT_ISYSTEM)) == 0 || strncmp(cur, GCC_OPT_LIBDIR, strlen(GCC_OPT_LIBDIR)) == 0) {
			pattern = GCC_OPT_ISYSTEM;
			
			if (strncmp(cur, GCC_OPT_LIBDIR, strlen(GCC_OPT_LIBDIR)) == 0) {
				pattern = GCC_OPT_LIBDIR;
			}
			
			size = strlen(pattern);
			offset = 0;
			
			cur += size;
			ch = *cur;
			
			if (ch == ZERO) {
				if ((index + ++offset) > (size_t) argc) {
					goto next;
				}
				
				cur = argv[index + offset];
			}
			
			/*
			In older versions of the NDK, the CMake/ndk-build scripts used to manually pass 
			the library/include directories of the C/C++ standard libraries to the compiler command.
			
			Pino does not use this approach, and instead relies on its own logic for locating 
			the C/C++ standard library directories.
			
			Since this is irrelevant to us—and may even cause conflicts with our own implementation—
			strip these arguments out and avoid passing them down to the compiler.
			*/
			if (strstr(cur, NDK_CXX_STL_DIRECTORY) == NULL && strstr(cur, NDK_SYSROOT_INCLUDE_DIRECTORY) == NULL && strstr(cur, NDK_SYSROOT_LIBRARY_DIRECTORY) == NULL) {
				goto next;
			}
			
			index += offset;
			
			continue;
		} else if (strncmp(cur, GCC_OPT_D, strlen(GCC_OPT_D)) == 0) {
			size = strlen(GCC_OPT_D);
			offset = 0;
			
			cur += size;
			ch = *cur;
			
			if (ch == ZERO) {
				if ((index + ++offset) > (size_t) argc) {
					goto next;
				}
				
				cur = argv[index + offset];
			}
			
			#if defined(PINO)
				if (strcmp(cur, M_ANDROID_UNAVAILABLE_SYMBOLS_ARE_WEAK) == 0) {
					android_weak_api_defs = 1;
					goto next;
				}
			#endif
			
			if (strncmp(cur, M_ANDROID_API, strlen(M_ANDROID_API)) == 0) {
				cur += strlen(M_ANDROID_API);
			} else if (strncmp(cur, M_ANDROID_MIN_SDK_VERSION, strlen(M_ANDROID_MIN_SDK_VERSION)) == 0) {
				cur += strlen(M_ANDROID_MIN_SDK_VERSION);
			} else {
				goto next;
			}
			
			ch = *cur;
			
			if (ch == ZERO || strlen(cur) != 2) {
				goto next;
			}
			
			a = cur[0];
			b = cur[1];
			
			if (!libcv_matches(a, b)) {
				goto next;
			}
			
			override_libcv = cur;
			
			index += offset;
			
			continue;
		} else if (strcmp(cur, GCC_OPT_C) == 0 || strcmp(cur, GCC_OPT_R) == 0 || strcmp(cur, GCC_OPT_S) == 0 || strcmp(cur, GCC_OPT_E) == 0 || strcmp(cur, GCC_OPT_M) == 0 || strcmp(cur, GCC_OPT_MM) == 0 || strcmp(cur, GCC_OPT_F_SYNTAX_ONLY) == 0) {
			linking = 0;
		} else if (strcmp(cur, GCC_OPT_SHARED) == 0) {
			linking_shared = 1;
		} else if (strncmp(cur, GCC_OPT_FSANITIZE, strlen(GCC_OPT_FSANITIZE)) == 0) {
			address_sanitizer = 1;
		} else if (strncmp(cur, GCC_OPT_F_USE_LD, strlen(GCC_OPT_F_USE_LD)) == 0) {
			override_linker = cur + strlen(GCC_OPT_F_USE_LD);
		} else if (strncmp(cur, GCC_OPT_F_STACK_PROTECTOR, strlen(GCC_OPT_F_STACK_PROTECTOR)) == 0) {
			stack_protector = 1;
		} else if (strcmp(cur, GCC_OPT_VERSION) == 0) {
			print_version = 1;
		} else if (strcmp(cur, GCC_OPT_NODEFAULTLIBS) == 0 || strcmp(cur, GCC_OPT_NOSTDLIB) == 0) {
			nodefaultlibs = 1;
		} else if (strcmp(cur, GCC_OPT_L_STDCXX) == 0) {
			wants_libcxx = 1;
		} else if (strcmp(cur, GCC_OPT_STATIC_LIBCXX) == 0) {
			wants_libcxx = 1;
			wants_static_libcxx = 1;
		} else if (strcmp(cur, GCC_OPT_STATIC_LIBGCC) == 0) {
			wants_libgcc = 1;
			wants_static_libgcc = 1;
		} else if (strcmp(cur, GCC_OPT_L_ATOMIC) == 0) {
			wants_libatomic = 1;
		} else if (strcmp(cur, GCC_OPT_L_GOMP) == 0 || strcmp(cur, GCC_OPT_F_OPENMP) == 0 || strcmp(cur, GCC_OPT_F_OPENACC) == 0) {
			wants_libgomp = 1;
		} else if (strcmp(cur, GCC_OPT_L_ITM) == 0 || strcmp(cur, GCC_OPT_F_GNU_TM) == 0) {
			wants_libitm = 1;
		} else if (strcmp(cur, GCC_OPT_L_QUADMATH) == 0) {
			wants_libquadmath = 1;
		} else if (strcmp(cur, GCC_OPT_L_RT) == 0) {
			have_rt_library = 1;
		} else if (strcmp(cur, GCC_OPT_L_MATH) == 0) {
			wants_libm = 1;
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
			} else if (strcmp(cur, QUADMATH_LIBRARY) == 0) {
				wants_libquadmath = 1;
			} else if (strcmp(cur, MATH_LIBRARY) == 0) {
				wants_libm = 1;
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
			size = strlen(CLANG_OPT_TARGET);
			
			/* Clang has two variants of this flag: one with double hyphens, and one with a single hyphen. */
			if (strncmp(cur, CLANG_OPT_TARGET + 1, strlen(CLANG_OPT_TARGET + 1)) == 0) {
				size--;
			}
			
			cur += size;
			
			ch = *cur;
			
			if (!(ch == EQUAL || ch == ZERO)) {
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
				
				if (strcmp(override_triplet, "armv5te") == 0) {
					strcpy(override_triplet, "armv5");
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
			/* Don't let Clang override the system root. */
			cur += strlen(GCC_OPT_SYSROOT);
			ch = *cur;
			
			if (!(ch == EQUAL || ch == ZERO)) {
				goto next;
			}
			
			if (ch == ZERO) {
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
		
		cur = argv[index];
		
		status = clang_specific_remove(prev, cur);
		
		 if (status != CLANG_SPECIFIC_REMOVE_NON) {
			if (status == CLANG_SPECIFIC_REMOVE_NXT) {
				index++;
			}
			
			if (prev != NULL && strcmp(prev, GCC_OPT_XLINKER) == 0) {
				prev = NULL;
				kargv[--kargc] = (char*) prev;
				
				if (status == CLANG_SPECIFIC_REMOVE_NXT) {
					index++;
				}
			}
			
			continue;
		}
		
		prev = cur;
		
		if (clang_specific_replace(cur, &kargc, kargv)) {
			continue;
		}
		
		werror = clang_warning_remove(cur);
		
		if (werror != CLANG_WARNING_REMOVE_NONE) {
			if (werror == NULL) {
				continue;
			}
			
			if (werror != cur) {
				cur = werror;
			}
		}
		
		kargv[kargc++] = (char*) cur;
	}
	
	if (wants_force_static == -1) {
		wants_force_static = (wants_static_libcxx || wants_static_libgcc);
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
	
	if (print_version && known_clang(cc)) {
		printf(CLANG_VERSION_TEMPLATE, DEFAULT_TARGET, parent_directory);
		goto end;
	}
	
	wants_libcxx += (strcmp(cc, GPP) == 0 || strcmp(cc, CPP) == 0 || strcmp(cc, GM2) == 0 || strcmp(cc, CLANGPP) == 0);
	
	if (wants_libcxx) {
		wants_libgcc = 1;
	}
	
	if (linking && wants_force_static) {
		/*
		These libraries rely on libgcc. If we are going to statically link with them,
		we should statically link with libgcc as well.
		*/
		if ((wants_libcxx || wants_libitm || wants_libgomp) && !wants_static_libgcc) {
			kargv[kargc++] = (char*) GCC_OPT_STATIC_LIBGCC;
		}
		
		/*
		GCC has some issues with statically linking these libraries implicitly
		(it leads to undefined references to libm functions), so we need to pass those
		"-static-<library>" flags explicitly to it:
		
		$ aarch64-unknown-linux-android21-gcc ... -fsanitize=address
		/lib/static/libasan.a(asan_interceptors.o):asan_interceptors.cpp:function ___interceptor_lgamma.part.0:(.text+0xfcfc): error: undefined reference to 'signgam'
		...
		*/
		if (address_sanitizer) {
			kargv[kargc++] = (char*) GCC_OPT_STATIC_LIBASAN;
			kargv[kargc++] = (char*) GCC_OPT_STATIC_LIBTSAN;
			kargv[kargc++] = (char*) GCC_OPT_STATIC_LIBLSAN;
			kargv[kargc++] = (char*) GCC_OPT_STATIC_LIBUBSAN;
			kargv[kargc++] = (char*) GCC_OPT_STATIC_LIBHWASAN;
		}
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
		a = *ptr;
		
		/* We reached the end of the string without finding it. */
		if (a == ZERO) {
			if (override_libcv != NULL) {
				/*
				 It's okay if we could not extract the libc version from the target triplet;
				 the caller provided it through other means.
				 */
				break;
			}
			
			err = ERR_UNKNOWN_SYSTEM_VERSION;
			goto end;
		}
		
		b = *(ptr + 1);
		
		if (libcv_matches(a, b)) {
			break;
		}
		
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
	
	if (wants_system_libraries) {
		/*
		* Using system libraries during compilation makes no sense when the target
		* and host don't match; linking would fail due to mismatching ABIs.
		*/
		wants_system_libraries -= (host != NULL && strcmp(host, triplet) != 0);
	}
	
	arch = get_arch(triplet);
	bitness = get_bitness(triplet);
	
	if (wants_neon && target_supports_neon(triplet)) {
		floating_point_unit = malloc(strlen(GCC_OPT_M_FPU) + strlen(GCC_FPU_NEON) + 1);
		
		if (floating_point_unit == NULL) {
			err = ERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(floating_point_unit, GCC_OPT_M_FPU);
		strcat(floating_point_unit, GCC_FPU_NEON);
		
		kargv[kargc++] = floating_point_unit;
	}
	
	if (wants_simd && (cur = get_simd(triplet)) != NULL) {
		kargv[kargc++] = (char*) cur;
	}
	
	if (wants_arm_mode && (arch == ARCH_SPEC_ARM && bitness == ARCH_ABI_32)) {
		kargv[kargc++] = (char*) GCC_OPT_M_ARM;
	}
	
	#if AUTO_PICK_LINKER
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
	#endif
	
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
		primary_library_directory = malloc((system_prefix == NULL ? 0 : strlen(system_prefix)) + strlen(USR_DIRECTORY) + strlen(LIB_DIRECTORY) + strlen(PATHSEP_S) + strlen(non_prefixed_triplet) + 1);
		
		if (primary_library_directory == NULL) {
			err = ERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		primary_library_directory[0] = '\0';
		
		if (system_prefix != NULL) {
			strcat(primary_library_directory, system_prefix);
		}
		
		strcat(primary_library_directory, USR_DIRECTORY);
		strcat(primary_library_directory, LIB_DIRECTORY);
		strcat(primary_library_directory, PATHSEP_S);
		strcat(primary_library_directory, non_prefixed_triplet);
		
		secondary_library_directory = malloc((system_prefix == NULL ? 0 : strlen(system_prefix)) + strlen(LIB_DIRECTORY) + strlen(PATHSEP_S) + strlen(non_prefixed_triplet) + 1);
		
		if (secondary_library_directory == NULL) {
			err = ERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		secondary_library_directory[0] = '\0';
		
		if (system_prefix != NULL) {
			strcat(secondary_library_directory, system_prefix);
		}
		
		strcat(secondary_library_directory, LIB_DIRECTORY);
		strcat(secondary_library_directory, PATHSEP_S);
		strcat(secondary_library_directory, non_prefixed_triplet);
		
		SYSTEM_LIBRARY_PATH[6] = primary_library_directory;
		SYSTEM_LIBRARY_PATH[7] = secondary_library_directory;
	}
	
	ch = *ptr;
	
	if (ch == ZERO && override_libcv != NULL) {
		ptr = (char*) override_libcv;
	}
	
	start = ptr;
	
	while (1) {
		a = *ptr;
		
		if (a == '-' || a == ZERO) {
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
	
	#if defined(PINO)
		android_current_sdk_version = libc_version;
	#endif
	
	get_libc_version_int(libc_version, version);
	
	libc_major = version[0];
	libc_minor = version[1];
	
	target_version = LIBC_VERSION(libc_major, libc_minor);
	
	#if defined(PINO)
		/*
		Handle support for weak API references.
		
		- https://developer.android.com/ndk/guides/using-newer-apis
		*/
		if (android_weak_api_defs) {
			cur = get_max_libc_version(triplet);
			
			libc_version = malloc(strlen(cur) + 1);
			
			if (libc_version == NULL) {
				err = ERR_MEM_ALLOC_FAILURE;
				goto end;
			}
			
			strcpy(libc_version, cur);
		}
	#endif
	
	/*
	Enable DT_RELR relocations on supported targets.
	
	- bfd
	As of binutils 2.45, this is supported for the following architectures: AArch64, x86, and x86_64.
	
	- gold
	Not supported at all.
	
	- LLD
	LLD always accepts the flag regardless of the target architecture and never outputs an error message.
	It's unclear whether it truly supports all architectures or if LLD is simply ignoring the flag when it's not supported.
	
	- mold
	Assumed to follow the same behavior as LLD.
	*/
	if (linking && target_supports_relr(triplet, override_linker, target_version)) {
		if (override_linker == NULL || strcmp(override_linker, "bfd") == 0) {
			kargv[kargc++] = (char*) GCC_OPT_XLINKER;
			kargv[kargc++] = (char*) LD_OPT_Z;
			kargv[kargc++] = (char*) GCC_OPT_XLINKER;
			kargv[kargc++] = (char*) LD_OPT_PACK_RELATIVE_RELOCS;
		} else if (strcmp(override_linker, "mold") == 0 || strcmp(override_linker, "lld") == 0) {
			#if defined(OBGGCC)
				/*
				DT_RELR on glibc requires a dependency on GLIBC_ABI_DT_RELR to be present,
				but --pack-dyn-relocs does not add that dependency by default, which leads to this error at runtime:
				
				$ gcc -fuse-ld=lld -Xlinker --pack-dyn-relocs <...>
				$ ./main
				./main: error while loading shared libraries: ./main: DT_RELR without GLIBC_ABI_DT_RELR dependency
				
				To work around this, we use -z pack-relative-relocs instead, but this comes with the downside of not being
				supported on older versions of lld and mold:
				
				$ gcc -fuse-ld=lld -Xlinker -z -Xlinker pack-relative-relocs <...>
				ld.lld: error: unknown -z value: pack-relative-relocs
				collect2: error: ld returned 1 exit status
				*/
				kargv[kargc++] = (char*) GCC_OPT_XLINKER;
				kargv[kargc++] = (char*) LD_OPT_Z;
				kargv[kargc++] = (char*) GCC_OPT_XLINKER;
				kargv[kargc++] = (char*) LD_OPT_PACK_RELATIVE_RELOCS;
			#else
				/*
				Everything else should work with --pack-dyn-relocs.
				*/
				kargv[kargc++] = (char*) GCC_OPT_XLINKER;
				kargv[kargc++] = (char*) LLD_OPT_PACK_DYN_RELOCS;
			#endif
		}
	}
	
	if (wants_lto != LTO_NONE) {
		kargv[kargc++] = (char*) GCC_OPT_F_LTO_AUTO;
		kargv[kargc++] = (char*) GCC_OPT_F_NO_FAT_LTO_OBJECTS;
		kargv[kargc++] = (char*) GCC_OPT_F_LTO_COMPRESSION_LEVEL_ZERO;
		
		kargv[kargc++] = (char*) ((wants_lto == LTO_FULL) ? GCC_OPT_F_LTO_PARTITION_NONE : GCC_OPT_F_LTO_PARTITION_BALANCED);
		
		if (wants_lto == LTO_FULL) {
			kargv[kargc++] = (char*) GCC_OPT_F_DEVIRTUALIZE_AT_LTRANS;
		}
	}
	
	#if defined(OBGGCC)
		/* Determine whether we need to implicit link with -lrt */
		wants_librt = wants_libcxx && !have_rt_library && target_version < LIBC_VERSION(2, 17);
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
	
	size += 2; /* -B <directory> */
	size += 2; /* -L <sysroot/lib/ldscripts> */
	
	if (wants_system_libraries) {
		size += (sizeof(SYSTEM_LIBRARY_PATH) / sizeof(*SYSTEM_LIBRARY_PATH)) * 6;
		size += 8;
		
		if (hardcode_system_rpath) {
			size += (sizeof(SYSTEM_LIBRARY_PATH) / sizeof(*SYSTEM_LIBRARY_PATH)) * 4;
		}
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
		size += 1; /* -mandroid-version-min=<version> */
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
	
	sysroot_library_directory = malloc(strlen(sysroot_directory) + strlen(LIBRARY_DIR) + strlen(STATIC_LIBRARY_DIR) + 1);
	
	if (sysroot_library_directory == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(sysroot_library_directory, sysroot_directory);
	strcat(sysroot_library_directory, LIBRARY_DIR);
	
	sysroot_ldscripts_directory = malloc(strlen(sysroot_library_directory) + strlen(LDSCRIPTS_DIR) + 1);
	
	if (sysroot_ldscripts_directory == NULL) {
		err = ERR_MEM_ALLOC_FAILURE;
		goto end;
	}
	
	strcpy(sysroot_ldscripts_directory, sysroot_library_directory);
	strcat(sysroot_ldscripts_directory, LDSCRIPTS_DIR);
	
	if (wants_force_static) {
		strcat(sysroot_library_directory, STATIC_LIBRARY_DIR);
	}
	
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
	
	#if !defined(PINO)
		gpp_builtins_include_directory = malloc(strlen(gpp_include_directory) + strlen(PATHSEP_S) + strlen(triplet) + 1);
		
		if (gpp_builtins_include_directory == NULL) {
			err = ERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(gpp_builtins_include_directory, gpp_include_directory);
		strcat(gpp_builtins_include_directory, PATHSEP_S);
		strcat(gpp_builtins_include_directory, triplet);
	#endif
	
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
	
	offset = 0;
	
	args[offset++] = executable;
	args[offset++] = arg;
	args[offset++] = (char*) GCC_OPT_NOSTDINC;
	
	if (strcmp(cc, GPP) == 0 || strcmp(cc, CPP) == 0 || strcmp(cc, CLANGPP) == 0) {
		args[offset++] = (char*) GCC_OPT_ISYSTEM;
		args[offset++] = gpp_include_directory;
		
		#if !defined(PINO)
			args[offset++] = (char*) GCC_OPT_ISYSTEM;
			args[offset++] = gpp_builtins_include_directory;
		#endif
	}
	
	args[offset++] = (char*) GCC_OPT_ISYSTEM;
	args[offset++] = gcc_include_directory;
	args[offset++] = (char*) GCC_OPT_ISYSTEM;
	args[offset++] = sysroot_include_directory;
	
	if (linking) {
		args[offset++] = (char*) GCC_OPT_LIBDIR;
		args[offset++] = sysroot_ldscripts_directory;
		
		args[offset++] = (char*) GCC_OPT_LIBDIR;
		args[offset++] = sysroot_library_directory;
		
		args[offset++] = (char*) GCC_OPT_B;
		args[offset++] = sysroot_library_directory;
		
		#if defined(OBGGCC)
			if (wants_librt && !nodefaultlibs) {
				args[offset++] = (char*) GCC_OPT_L_RT;
			}
			
			/* TODO: apply this patch instead: https://gitlab.alpinelinux.org/alpine/aports/-/blob/4d8f399a49555dae872efd56251c066d21f428a0/main/gcc/0029-configure-Add-enable-autolink-libatomic-use-in-LINK_.patch */
			if (require_atomic_library && !nodefaultlibs) {
				args[offset++] = (char*) GCC_OPT_L_ATOMIC;
			}
		#endif
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
		
		if (linking) {
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
	}
	
	if (wants_system_libraries) {
		if (host_version < target_version && directory_exists(sysroot_include_missing_directory) == 1) {
			args[offset++] = (char*) GCC_OPT_ISYSTEM;
			args[offset++] = sysroot_include_missing_directory;
		}
		
		/* <prefix>/usr/include */
		directory = malloc((system_prefix == NULL ? 0 : strlen(system_prefix)) + strlen(SYSTEM_INCLUDE_PATH) + 1);
		
		if (directory == NULL) {
			err = ERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		directory[0] = '\0';
		
		if (system_prefix != NULL) {
			strcat(directory, system_prefix);
		}
		
		strcat(directory, SYSTEM_INCLUDE_PATH);
		
		if (directory_exists(directory) == 1) {
			args[offset++] = (char*) GCC_OPT_ISYSTEM;
			args[offset++] = (char*) directory;
		}
		
		/* <prefix>/usr/include/<triplet> */
		directory = malloc((system_prefix == NULL ? 0 : strlen(system_prefix)) + strlen(SYSTEM_INCLUDE_PATH) + strlen(PATHSEP_S) + strlen(non_prefixed_triplet) + 1);
		
		if (directory == NULL) {
			err = ERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		directory[0] = '\0';
		
		if (system_prefix != NULL) {
			strcat(directory, system_prefix);
		}
		
		strcat(directory, SYSTEM_INCLUDE_PATH);
		strcat(directory, PATHSEP_S);
		strcat(directory, non_prefixed_triplet);
		
		if (directory_exists(directory) == 1) {
			args[offset++] = (char*) GCC_OPT_ISYSTEM;
			args[offset++] = directory;
		}
		
		if (linking) {
			if (host_version < target_version) {
				/*
				* Setting this flag is only useful when the host has a system/libc version lower than the target system.
				*/
				args[offset++] = (char*) GCC_OPT_XLINKER;
				args[offset++] = (char*) LD_OPT_UNRESOLVED_SYMBOLS;
			}
			
			for (index = 0; index < sizeof(SYSTEM_LIBRARY_PATH) / sizeof(*SYSTEM_LIBRARY_PATH); index++) {
				cur = SYSTEM_LIBRARY_PATH[index];
				
				directory = malloc((system_prefix == NULL ? 0 : strlen(system_prefix)) + strlen(cur) + 1);
				
				if (directory == NULL) {
					err = ERR_MEM_ALLOC_FAILURE;
					goto end;
				}
				
				directory[0] = '\0';
				
				if (system_prefix != NULL) {
					strcat(directory, system_prefix);
				}
				
				strcat(directory, cur);
				
				if (directory_exists(directory) != 1) {
					continue;
				}
				
				args[offset++] = (char*) GCC_OPT_LIBDIR;
				args[offset++] = (char*) directory;
				
				args[offset++] = (char*) GCC_OPT_XLINKER;
				args[offset++] = (char*) LD_OPT_RPATH_LINK;
				
				args[offset++] = (char*) GCC_OPT_XLINKER;
				args[offset++] = (char*) directory;
				
				/*
				* Hardcode the system library directories during linkage.
				* Currently, we only do this on systems where this is the
				* default required behavior:
				*
				* - Termux running on Android 7.0 (API level 24) or higher.
				*/
				if (hardcode_system_rpath) {
					args[offset++] = (char*) GCC_OPT_XLINKER;
					args[offset++] = (char*) LD_OPT_RPATH;
					
					args[offset++] = (char*) GCC_OPT_XLINKER;
					args[offset++] = (char*) directory;
				}
			}
		}
	}
	
	if (linking && wants_builtin_loader) {
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
	
	if (linking && wants_runtime_rpath) {
		args[offset++] = (char*) GCC_OPT_XLINKER;
		args[offset++] = (char*) LD_OPT_RPATH;
		
		args[offset++] = (char*) GCC_OPT_XLINKER;
		args[offset++] = sysroot_runtime_directory;
	}
	
	#if defined(OBGGCC)
		if (linking && address_sanitizer) {
			args[offset++] = (char*) GCC_OPT_XLINKER;
			args[offset++] = (char*) LD_OPT_UNRESOLVED_SYMBOLS;
		}
	#endif
	
	#if defined(PINO)
		/* -mandroid-version-min */
		android_version_min = malloc(strlen(GCC_M_ANDROID_VERSION_MIN) + strlen(android_current_sdk_version) + 1);
		
		if (android_version_min == NULL) {
			err = ERR_MEM_ALLOC_FAILURE;
			goto end;
		}
		
		strcpy(android_version_min, GCC_M_ANDROID_VERSION_MIN);
		strcat(android_version_min, android_current_sdk_version);
		
		args[offset++] = android_version_min;
	#endif
	
	memcpy(&args[offset], &kargv[1], kargc * sizeof(*kargv));
	
	if (verbose) {
		obggcc_print_args(args);
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
		if (linking && !wants_force_static && known_clang(cc) && output_directory != NULL) {
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
			
			/* libquadmath */
			if (wants_libquadmath) {
				err = copy_shared_library(sysroot_library_directory, output_directory, LIBQUADMATH_SHARED, LIBQUADMATH_SHARED);
				
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
	
	if (execve(executable, args, environ) == -1) {
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
	
	#if AUTO_PICK_LINKER
		free(linker);
	#endif
	
	free(floating_point_unit);
	free(sysroot_include_directory);
	free(sysroot_include_missing_directory);
	free(sysroot_library_directory);
	free(sysroot_ldscripts_directory);
	free(gcc_include_directory);
	free(gpp_include_directory);
	free(gpp_builtins_include_directory);
	free(primary_library_directory);
	free(secondary_library_directory);
	free(nz_sysroot_directory);
	free(parent_directory);
	free(non_prefixed_triplet);
	free(override_triplet);
	
	#if defined(PINO)
		free(android_version_min);
		
		if (android_weak_api_defs) {
			free(android_current_sdk_version);
		}
	#endif
	
	query_free(&query);
	
	if (err != ERR_SUCCESS) {
		cur = strerror(errno);
		
		fprintf(stderr, "fatal error: %s", obggcc_strerror(err));
		
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
