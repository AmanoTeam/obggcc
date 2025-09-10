#if !defined(OBGGCC_H)
#define OBGGCC_H

#if !(defined(OBGGCC) || defined(PINO))
	#error "Please define the cross-compiler flavor for which we will be a wrapper"
#endif

#if defined(OBGGCC)
	#define WRAPPER_FLAVOR_NAME "OBGGCC"
#elif defined(PINO)
	#define WRAPPER_FLAVOR_NAME "PINO"
#else
	#error "I don't know how to handle this"
#endif

static const char ENV_SYSTEM_PREFIX[] = WRAPPER_FLAVOR_NAME "_SYSTEM_PREFIX";
static const char ENV_SYSTEM_LIBRARIES[] = WRAPPER_FLAVOR_NAME "_SYSTEM_LIBRARIES";
static const char ENV_BUILTIN_LOADER[] = WRAPPER_FLAVOR_NAME "_BUILTIN_LOADER";
static const char ENV_STATIC[] = WRAPPER_FLAVOR_NAME "_STATIC";
static const char ENV_NZ[] = WRAPPER_FLAVOR_NAME "_NZ";
static const char ENV_RUNTIME_RPATH[] = WRAPPER_FLAVOR_NAME "_RUNTIME_RPATH";
static const char ENV_VERBOSE[] = WRAPPER_FLAVOR_NAME "_VERBOSE";
static const char ENV_NEON[] = WRAPPER_FLAVOR_NAME "_NEON";
static const char ENV_ARM_MODE[] = WRAPPER_FLAVOR_NAME "_ARM_MODE";
static const char ENV_SIMD[] = WRAPPER_FLAVOR_NAME "_SIMD";
static const char ENV_LTO[] = WRAPPER_FLAVOR_NAME "_LTO";

void obggcc_print_args(char* args[]);

#endif
