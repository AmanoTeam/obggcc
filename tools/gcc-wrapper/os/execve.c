#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <processthreadsapi.h>
#endif

#if !defined(_WIN32)
	#include <unistd.h>
#endif

static int quotable(const char* value) {
	
	size_t index = 0;
	size_t size = strlen(value);
	
	unsigned char ch = 0;
	
	for (index = 0; index < size; index++) {
		ch = value[index];
		
		if (!isspace(ch)) {
			continue;
		}
		
		return 1;
	}
	
	return 0;
	
}

static char* quote(char** arguments) {

	char* string = NULL;
	char* item = NULL;
	
	size_t size = 0;
	size_t index = 0;
	
	int status = 0;
	
	for (index = 0; 1; index++) {
		item = arguments[index];
		
		if (item == NULL) {
			break;
		}
		
		size += strlen(item) + (sizeof(char) * 3);
	}
	
	string = malloc(size + 1);
	
	if (string == NULL) {
		return NULL;
	}
	
	string[0] = '\0';
	
	for (index = 0; 1; index++) {
		item = arguments[index];
			
		if (item == NULL) {
			break;
		}
		
		if (index != 0) {
			strcat(string, " ");
		}
		
		status = quotable(item);
		
		if (status) {
			strcat(string, "\"");
		}
		
		strcat(string, item);
		
		if (status) {
			strcat(string, "\"");
		}
	}
	
	return string;
	
}
	

int execute_command(const char* const cmd, char** arg) {
	
	int err = 0;
	
	#if defined(_WIN32)
		char* qarg = NULL;
		
		const int flags = NORMAL_PRIORITY_CLASS;
		BOOL status = FALSE;
		
		PROCESS_INFORMATION info = {0};
		
		#if defined(_UNICODE)
			wchar_t* wqarg = NULL;
			wchar_t* wcmd = NULL;
			
			STARTUPINFOW startup = {0};
			
			int wsize = 0;
		#else
			STARTUPINFOA startup = {0};
		#endif
		
		startup.cb = sizeof(startup);
		startup.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		startup.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		startup.dwFlags = STARTF_USESTDHANDLES;
		
		qarg = quote(arg);
		
		if (qarg == NULL) {
			return -1;
		}
		
		#if defined(_UNICODE)
			wsize = MultiByteToWideChar(CP_UTF8, 0, cmd, -1, NULL, 0);
			
			if (wsize == 0) {
				err = -1;
				goto end;
			}
			
			wcmd = malloc(wsize * sizeof(*wcmd));
			
			if (wcmd == NULL) {
				err = -1;
				goto end;
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, cmd, -1, wcmd, wsize) == 0) {
				err = -1;
				goto end;
			}
			
			wsize = MultiByteToWideChar(CP_UTF8, 0, qarg, -1, NULL, 0);
			
			if (wsize == 0) {
				err = -1;
				goto end;
			}
			
			wqarg = malloc(wsize * sizeof(*wqarg));
			
			if (wqarg == NULL) {
				err = -1;
				goto end;
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, qarg, -1, wqarg, wsize) == 0) {
				err = -1;
				goto end;
			}
			
			status = CreateProcessW(wcmd, wqarg, NULL, NULL, TRUE, flags, NULL, NULL, &startup, &info);
		#else
			status = CreateProcessA(cmd, qarg, NULL, NULL, TRUE, flags, NULL, NULL, &startup, &info);
		#endif
		
		if (status == FALSE) {
			err = -1;
			goto end;
		}
		
		if (WaitForSingleObject(info.hProcess, INFINITE) == WAIT_FAILED) {
			err = -1;
			goto end;
		}
	#else
		if (execv(cmd, arg) == -1) {
			err = -1;
			goto end;
		}
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wqarg);
		free(wcmd);
	#endif
	
	return err;
	
}
