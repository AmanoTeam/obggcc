#include "errors.h"

const char* obggcc_strerror(const int err) {
	
	switch (err) {
		case ERR_SUCCESS:
			return "Success";
		case ERR_MEM_ALLOC_FAILURE:
			return "Could not allocate memory";
		case ERR_UNKNOWN_COMPILER:
			return "Unknown C/C++ compiler";
		case ERR_GET_APP_FILENAME_FAILURE:
			return "Could not get app filename";
		case ERR_EXECVE_FAILURE:
			return "Call to execve failed";
		case ERR_BAD_TRIPLET:
			return "The target triplet is invalid or was not recognized";
		case ERR_GETEXT_FAILURE:
			return "Could not get file extension of object file";
		case ERR_COPY_FILE_FAILURE:
			return "Could not copy file";
		case ERR_UNKNOWN_SYSTEM_VERSION:
			return "The system or C library version provided through the target triplet is invalid or was not recognized";
		case ERR_BINFMT_GUESS_FAILURE:
			return "Could not guess ELF architecture";
		default:
			return "Unknown error";
	}
	
}
