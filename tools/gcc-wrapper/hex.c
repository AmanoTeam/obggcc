#include "hex.h"

unsigned char to_hex(const unsigned char ch) {
	
	return ch + (ch > 9 ? ('A' - 10) : '0');
	
}

unsigned char from_hex(const unsigned char ch) {
	
	if (ch <= '9' && ch >= '0') {
		return ch - '0';
	}
	
	 if (ch <= 'f' && ch >= 'a') {
		return ch - ('a' - 10);
	}
	
	if (ch <= 'F' && ch >= 'A') {
		return ch - ('A' - 10);
	}
	
	return '0';
	
}
