#if !defined(BIGGESTINT_H)
#define BIGGESTINT_H

#include <stddef.h>
#include <inttypes.h>
#include <float.h>
#include <limits.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
	#define HAVE_LONG_LONG 1
#endif

typedef long double bigfloat_t;

#define BIGFLOAT_MIN DBL_MIN
#define BIGFLOAT_MAX DBL_MAX

#define FORMAT_BIGGEST_FLOAT_T "Lf"

#define FORMAT_UINT_PTR_T PRIuPTR
#define FORMAT_INT_PTR_T PRIiPTR

#define strtobf strtold

#if defined(HAVE_LONG_LONG)
	typedef long long int bigint_t;
	typedef unsigned long long int biguint_t;
	
	#define BIGINT_MIN LLONG_MIN
	#define BIGINT_MAX LLONG_MAX
	
	#define BIGUINT_MIN 0
	#define BIGUINT_MAX ULLONG_MAX
	
	#define FORMAT_BIGGEST_INT_T "lld"
	#define FORMAT_BIGGEST_UINT_T "llu"
	
	#define FORMAT_HEX_BIGGEST_INT_T "ll"
	#define FORMAT_HEX_BIGGEST_UINT_T "ll"
	
	#define strtobi strtoll
	#define strtobui strtoull
#else
	typedef long int bigint_t;
	typedef unsigned long int biguint_t;
	
	#define BIGINT_MIN LONG_MIN
	#define BIGINT_MAX LONG_MAX
	
	#define BIGUINT_MIN 0
	#define BIGUINT_MAX ULONG_MAX
	
	#define FORMAT_BIGGEST_INT_T "ld"
	#define FORMAT_BIGGEST_UINT_T "lu"
	
	#define FORMAT_HEX_BIGGEST_INT_T "l"
	#define FORMAT_HEX_BIGGEST_UINT_T "l"
	
	#define strtobi strtol
	#define strtobui strtoul
#endif

bigint_t ptobigint(const void* const pointer);
biguint_t ptobiguint(const void* const pointer);

struct bigint_storage {
	bigint_t a;
	biguint_t b;
	bigfloat_t c;
};

typedef struct bigint_storage bigint_storage_t;

size_t intlen(const bigint_t value);
char* int_stringify(const bigint_t value);
size_t uintlen(const biguint_t value);
char* uint_stringify(const biguint_t value);

#endif
