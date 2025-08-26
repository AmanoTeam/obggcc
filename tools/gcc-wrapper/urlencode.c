#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "hex.h"
#include "urlencode.h"

static const char QUOTE_SAFE_SYMBOLS[] = "-._~";

size_t urlencode(
	const char* const uri,
	char* const destination
) {
	
	size_t index = 0;
	size_t offset = 0;
	size_t size = 0;
	
	for (index = 0; index < strlen(uri); index++) {
		const unsigned char ch = uri[index];
		
		if (isalnum(ch) || strchr(QUOTE_SAFE_SYMBOLS, ch) != NULL) {
			if (destination != NULL) {
				destination[offset++] = ch;
			}
			
			size += 1;
		} else if (ch == ' ') {
			if (destination != NULL) {
				destination[offset++] = '%';
				destination[offset++] = '2';
				destination[offset++] = '0';
			}
			
			size += 3;
		} else {
			if (destination != NULL) {
				destination[offset++] = '%';
				destination[offset++] = to_hex(ch >> 4);
				destination[offset++] = to_hex(ch % 16);
			}
			
			size += 3;
		}
	}
	
	if (destination != NULL) {
		destination[offset++] = '\0';
	}
	
	size += 1;
	
	return size;
	
}
