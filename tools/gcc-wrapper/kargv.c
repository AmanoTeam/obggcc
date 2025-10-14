#include <stddef.h>
#include <stdlib.h>

#include "kargv.h"

int kargv_append(
	kargv_t* const args,
	char* const value
) {
	
	size_t size = 0;
	char** items = NULL;
	
	if (sizeof(*args->items) * (args->offset + 1) > args->size) {
		size = args->size + sizeof(*args->items) * ((args->offset + 1) * 2);
		items = realloc(args->items, size);
		
		if (items == NULL) {
			return -1;
		}
		
		args->size = size;
		args->items = items;
	}
	
	args->items[args->offset++] = value;
	
	return 0;
	
}

void kargv_free(kargv_t* const args) {
	
	free(args->items);
	
	args->items = NULL;
	args->offset = 0;
	args->size = 0;
	
}

int kargv_merge(
	kargv_t* const args,
	kargv_t* const subargs
) {
	
	size_t index = 0;
	char* value = NULL;
	
	if (kargv_getoffset(subargs) == 0) {
		return 0;
	}
	
	for (index = 0; index < kargv_getoffset(subargs); index++) {
		value = kargv_get(subargs, index);
		
		if (kargv_append(args, value) == -1) {
			return -1;
		}
	}
	
	kargv_free(subargs);
	
	return 0;
	
}