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

const char* gcc_version_stringify(const gcc_version_t value) {

	switch (value) {
		case GCC_3_1:
			return "3.1";
		case GCC_3_2:
			return "3.2";
		case GCC_3_3:
			return "3.3";
		case GCC_3_4:
			return "3.4";
		case GCC_4_0:
			return "4.0";
		case GCC_4_1:
			return "4.1";
		case GCC_4_2:
			return "4.2";
		case GCC_4_3:
			return "4.3";
		case GCC_4_4:
			return "4.4";
		case GCC_4_5:
			return "4.5";
		case GCC_4_6:
			return "4.6";
		case GCC_4_7:
			return "4.7";
		case GCC_4_8:
			return "4.8";
		case GCC_4_9:
			return "4.9";
		case GCC_5:
			return "5";
		case GCC_6:
			return "6";
		case GCC_7:
			return "7";
		case GCC_8:
			return "8";
		case GCC_9:
			return "9";
		case GCC_10:
			return "10";
		case GCC_11:
			return "11";
		case GCC_12:
			return "12";
		case GCC_13:
			return "13";
		case GCC_14:
			return "14";
		case GCC_15:
			return "15";
		case GCC_16:
			return "16";
	}
	
	return NULL;
	
}
