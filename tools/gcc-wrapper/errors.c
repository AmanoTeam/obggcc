#include "errors.h"

const char* obggcc_strerror(const int err) {
	
	switch (err) {
		case ERR_SUCCESS:
			return "Success";
		case ERR_MEM_ALLOC_FAILURE:
			return "Memory allocation failed";
		case ERR_UNKNOWN_COMPILER:
			return "Unrecognized or unsupported C/C++ compiler";
		case ERR_UNKNOWN_BINUTILS_WRAPPER:
			return "Unrecognized or unsupported binutils wrapper";
		case ERR_GET_APP_FILENAME_FAILURE:
			return "Failed to retrieve application filename";
		case ERR_EXECVE_FAILURE:
			return "Process execution failed (execve)";
		case ERR_EXECVE_SUBPROCESS_FAILURE:
			return "Subprocess execution failed (execve returned an error)";
		case ERR_FORK_FAILURE:
			return "Process creation failed (fork)";
		case ERR_BAD_TRIPLET:
			return "Invalid or unrecognized target triplet";
		case ERR_GETEXT_FAILURE:
			return "Failed to determine file extension for object file";
		case ERR_COPY_FILE_FAILURE:
			return "File copy operation failed";
		case ERR_UNKNOWN_SYSTEM_VERSION:
			return "Invalid or unrecognized system/C library version in target triplet";
		case ERR_BINFMT_GUESS_FAILURE:
			return "Failed to detect binary format (ELF architecture guessing failed)";
		case ERR_NO_INPUT_FILES:
			return "No input files";
		default:
			return "Unknown error";
	}
	
}
