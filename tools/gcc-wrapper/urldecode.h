#if !defined(URLDECODE_H)
#define URLDECODE_H

#include <stddef.h>

size_t urldecode(
	const char* const uri,
	char* const destination
);

#endif
