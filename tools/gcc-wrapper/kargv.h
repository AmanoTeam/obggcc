#include <stddef.h>

struct argv {
	size_t size;
	size_t offset;
	char** items;
};

typedef struct argv kargv_t;

int kargv_append(
	kargv_t* const args,
	char* const value
);

char* kargv_get(
	kargv_t* const args,
	const size_t index
);

int kargv_merge(
	kargv_t* const args,
	kargv_t* const subargs
);

void kargv_free(kargv_t* const args);

#define kargv_get(args, index) ((args)->items[index])
#define kargv_getoffset(args) ((args)->offset)
#define kargv_getargs(args) ((args)->items)
