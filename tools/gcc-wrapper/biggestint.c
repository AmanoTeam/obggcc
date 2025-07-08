#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "biggestint.h"

size_t intlen(const bigint_t value) {
	/*
	Calculates the number of digits required to represent this
	integer as a string.
	*/
	
	bigint_t val = value;
	size_t size = 0;
	
	if (val < 0) {
		size++;
	}
	
	do {
		val /= 10;
		size++;
	} while (val != 0);
	
	return size;
	
}

char* int_stringify(const bigint_t value) {
	
	int wsize = 0;
	
	const size_t size = intlen(value) + 1;
	char* string = malloc(size);
	
	if (string == NULL) {
		return string;
	}
	
	wsize = snprintf(
		string,
		size,
		"%"FORMAT_BIGGEST_INT_T,
		value
	);
	
	if (wsize < 1) {
		free(string);
		string = NULL;
	}
	
	return string;
	
}

size_t uintlen(const biguint_t value) {
	/*
	Calculates the number of digits required to represent this
	unsigned integer as a string.
	*/
	
	biguint_t val = value;
	size_t size = 0;
	
	do {
		val /= 10;
		size++;
	} while (val != 0);
	
	return size;
	
}

char* uint_stringify(const biguint_t value) {
	
	int wsize = 0;
	
	const size_t size = intlen(value) + 1;
	char* string = malloc(size);
	
	if (string == NULL) {
		return string;
	}
	
	wsize = snprintf(
		string,
		size,
		"%"FORMAT_BIGGEST_UINT_T,
		value
	);
	
	if (wsize < 1) {
		free(string);
		string = NULL;
	}
	
	return string;
	
}