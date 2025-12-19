#if !defined(OBGGCC_H)
#define OBGGCC_H

#if !(defined(OBGGCC) || defined(PINO) || defined(ATAR))
	#error "Please define the cross-compiler flavor for which we will be a wrapper"
#endif

#if defined(OBGGCC)
	#define WRAPPER_FLAVOR_NAME "OBGGCC"
#elif defined(PINO)
	#define WRAPPER_FLAVOR_NAME "PINO"
#elif defined(ATAR)
	#define WRAPPER_FLAVOR_NAME "ATAR"
#else
	#error "I don't know how to handle this"
#endif

#if defined(WCLANG) && defined(PINO)
	#error "-DWCLANG and -DPINO are not supported together"
#endif

#if defined(ATAR)
	#define UNVERSIONED_CROSS_COMPILER
#endif

static const char DEFAULT_TARGET[] = 
#if defined(OBGGCC)
	 "x86_64-unknown-linux-gnu2.3";
#elif defined(PINO)
	"x86_64-unknown-linux-android21";
#elif defined(ATAR)
	"x86_64-unknown-openbsd";
#else
	#error "I don't know how to handle this"
#endif

static const char ENV_SYSTEM_PREFIX[] = WRAPPER_FLAVOR_NAME "_SYSTEM_PREFIX";
static const char ENV_SYSTEM_LIBRARIES[] = WRAPPER_FLAVOR_NAME "_SYSTEM_LIBRARIES";
static const char ENV_BUILTIN_LOADER[] = WRAPPER_FLAVOR_NAME "_BUILTIN_LOADER";
static const char ENV_STATIC[] = WRAPPER_FLAVOR_NAME "_STATIC";
static const char ENV_STATIC_RUNTIME[] = WRAPPER_FLAVOR_NAME "_STATIC_RUNTIME";
static const char ENV_NZ[] = WRAPPER_FLAVOR_NAME "_NZ";
static const char ENV_RUNTIME_RPATH[] = WRAPPER_FLAVOR_NAME "_RUNTIME_RPATH";
static const char ENV_VERBOSE[] = WRAPPER_FLAVOR_NAME "_VERBOSE";
static const char ENV_NEON[] = WRAPPER_FLAVOR_NAME "_NEON";
static const char ENV_ARM_MODE[] = WRAPPER_FLAVOR_NAME "_ARM_MODE";
static const char ENV_SIMD[] = WRAPPER_FLAVOR_NAME "_SIMD";
static const char ENV_LTO[] = WRAPPER_FLAVOR_NAME "_LTO";

static const char OBGGCC_OPT_HELP[] = "--obggcc-help";

static const char OBGGCC_OPT_F_BUILTIN_LOADER[] = "-fbuiltin-loader";
static const char OBGGCC_OPT_F_NO_BUILTIN_LOADER[] = "-fno-builtin-loader";

static const char OBGGCC_OPT_F_RUNTIME_RPATH[] = "-fruntime-rpath";
static const char OBGGCC_OPT_F_NO_RUNTIME_RPATH[] = "-fno-runtime-rpath";

static const char OBGGCC_OPT_F_SYSTEM_LIBRARIES[] = "-fsystem-libraries";
static const char OBGGCC_OPT_F_NO_SYSTEM_LIBRARIES[] = "-fno-system-libraries";

static const char OBGGCC_OPT_F_NZ[] = "-fnz";
static const char OBGGCC_OPT_F_NO_NZ[] = "-fno-nz";

static const char OBGGCC_OPT_F_STATIC_RUNTIME[] = "-fstatic-runtime";
static const char OBGGCC_OPT_F_NO_STATIC_RUNTIME[] = "-fno-static-runtime";

static const char OBGGCC_OPT_F_VERBOSE[] = "-fverbose";
static const char OBGGCC_OPT_F_NO_VERBOSE[] = "-fno-verbose";

void obggcc_print_args(char* args[]);

#endif
