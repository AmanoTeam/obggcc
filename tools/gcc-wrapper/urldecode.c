#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "hex.h"
#include "urldecode.h"

static const char URI_SAFE_SYMBOLS[] = "!#$%&'()*+,-./:;=?@[]_~";

size_t urldecode(
	const char* const uri,
	char* const destination
) {
	
	size_t index = 0;
	size_t offset = 0;
	
	const size_t length = strlen(uri);
	
	for (index = 0; index < length; index++) {
		const unsigned char ch = uri[index];
		
		if (destination != NULL) {
			destination[offset] = ch;
		}
		
		if (ch == '%' && length > (index + 2)) {
			const unsigned char a = uri[index + 1];
			const unsigned char b = uri[index + 2];
			
			const unsigned char c = ((from_hex(a) << 4) | from_hex(b));
			
			if (isalnum(ch) || strchr(URI_SAFE_SYMBOLS, ch) != NULL) {
				if (destination != NULL) {
					destination[offset] = c;
				}
				
				index += 2;
			}
		}
		
		offset++;
	}
	
	if (destination != NULL) {
		destination[offset] = '\0';
	}
	
	offset++;
	
	return offset;
	
}
