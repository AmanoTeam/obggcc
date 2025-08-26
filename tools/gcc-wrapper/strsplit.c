#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if !defined(_WIN32)
	#include <sys/types.h>
#endif

#include "strsplit.h"

void strsplit_init(
	strsplit_t* const strsplit,
	const char* const string,
	const char* const sep
) {
	
	strsplit->sstart = string;
	strsplit->send = strchr(strsplit->sstart, '\0');
	
	strsplit->sep = sep;
	
	strsplit->cur_pbegin = NULL;
	strsplit->cur_pend = NULL;
	
	strsplit->pbegin = strsplit->sstart;
	strsplit->pend = strstr(strsplit->pbegin, strsplit->sep);
	
	strsplit->eof = 0;
	
}

const strsplit_part_t* strsplit_next(
	strsplit_t* const strsplit,
	strsplit_part_t* const part
) {
	
	int seek = 0;
	
	size_t part_size = 0;
	
	if (strsplit->eof) {
		return NULL;
	}
	
	if (strsplit->pend == NULL) {
		strsplit->pend = strsplit->send;
	}
	
	part_size = (size_t) (strsplit->pend - strsplit->pbegin);
	
	part->begin = strsplit->pbegin;
	part->size = part_size;
	
	if (part_size > 0) {
		const char* start = strsplit->pbegin;
		const char* end = strsplit->pend;
		
		while (start != end) {
			const char ch = *start;
			
			if (!isspace(ch)) {
				break;
			}
			
			start++;
		}
		
		seek = (start != end && end != strsplit->send);
		
		if (seek) {
			end -= strlen(strsplit->sep);
		}
		
		while (end != start) {
			const unsigned char ch = *end;
			
			if (!isspace(ch)) {
				break;
			}
			
			end--;
		}
		
		part->begin = NULL;
		part->size = 0;
		
		if (seek) {
			end++;
		}
		
		if (end != start) {
			part->begin = start;
			part->size = (size_t) (end - start);
		}
	}
	
	strsplit->eof = (strsplit->pend == strsplit->send);
	
	if (strsplit->eof && part_size < 1) {
		return NULL;
	}
	
	if (strsplit->pbegin != strsplit->sstart) {
		part->index++;
	}
	
	strsplit->cur_pbegin = strsplit->pbegin;
	strsplit->cur_pend = strsplit->pend;
	
	strsplit->pbegin = NULL;
	strsplit->pend = NULL;
	
	if (!strsplit->eof) {
		strsplit->pbegin = (strsplit->cur_pend + strlen(strsplit->sep));
		strsplit->pend = strstr(strsplit->pbegin, strsplit->sep);
	}
	
	return part;
	
}

int strsplit_resize(
	const strsplit_t* const strsplit,
	strsplit_part_t* const part
) {
	
	ssize_t value = 0;
	
	if ((part->begin + part->size) <= strsplit->cur_pend) {
		return 0;
	}
	
	value = (ssize_t) (strsplit->cur_pend - part->begin);
	
	if (value < 0) {
		return -1;
	}
	
	part->size = (size_t) value;
	
	return 0;
	
}