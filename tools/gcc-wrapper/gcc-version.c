#include <string.h>

#include "gcc-version.h"

gcc_version_t gcc_version_unstringify(const char* const value) {
	
	if (strcmp(value, "3.1") == 0) {
		return GCC_3_1;
	}

	if (strcmp(value, "3.2") == 0) {
		return GCC_3_2;
	}

	if (strcmp(value, "3.3") == 0) {
		return GCC_3_3;
	}

	if (strcmp(value, "3.4") == 0) {
		return GCC_3_4;
	}

	if (strcmp(value, "4.0") == 0) {
		return GCC_4_0;
	}

	if (strcmp(value, "4.1") == 0) {
		return GCC_4_1;
	}

	if (strcmp(value, "4.2") == 0) {
		return GCC_4_2;
	}

	if (strcmp(value, "4.3") == 0) {
		return GCC_4_3;
	}

	if (strcmp(value, "4.4") == 0) {
		return GCC_4_4;
	}

	if (strcmp(value, "4.5") == 0) {
		return GCC_4_5;
	}

	if (strcmp(value, "4.6") == 0) {
		return GCC_4_6;
	}

	if (strcmp(value, "4.7") == 0) {
		return GCC_4_7;
	}

	if (strcmp(value, "4.8") == 0) {
		return GCC_4_8;
	}

	if (strcmp(value, "4.9") == 0) {
		return GCC_4_9;
	}

	if (strcmp(value, "5") == 0) {
		return GCC_5;
	}

	if (strcmp(value, "6") == 0) {
		return GCC_6;
	}

	if (strcmp(value, "7") == 0) {
		return GCC_7;
	}

	if (strcmp(value, "8") == 0) {
		return GCC_8;
	}

	if (strcmp(value, "9") == 0) {
		return GCC_9;
	}

	if (strcmp(value, "10") == 0) {
		return GCC_10;
	}

	if (strcmp(value, "11") == 0) {
		return GCC_11;
	}

	if (strcmp(value, "12") == 0) {
		return GCC_12;
	}

	if (strcmp(value, "13") == 0) {
		return GCC_13;
	}

	if (strcmp(value, "14") == 0) {
		return GCC_14;
	}

	if (strcmp(value, "15") == 0) {
		return GCC_15;
	}

	if (strcmp(value, "16") == 0) {
		return GCC_16;
	}

	return GCC_VERSION_UNKNOWN;
	
}
