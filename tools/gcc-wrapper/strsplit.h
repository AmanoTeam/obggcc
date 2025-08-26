#if !defined(STRSPLIT_H)
#define STRSPLIT_H

#include <stddef.h>

struct strsplit {
	const char* sstart;
	const char* send;
	const char* cur_pbegin;
	const char* cur_pend;
	const char* pbegin;
	const char* pend;
	const char* sep;
	int eof;
};

struct strsplit_part {
	size_t index;
	size_t size;
	const char* begin;
};

typedef struct strsplit strsplit_t;
typedef struct strsplit_part strsplit_part_t;

void strsplit_init(
	strsplit_t* const strsplit,
	const char* const string,
	const char* const sep
);

const strsplit_part_t* strsplit_next(
	strsplit_t* const strsplit,
	strsplit_part_t* const part
);

int strsplit_resize(
	const strsplit_t* const strsplit,
	strsplit_part_t* const part
);

#endif
