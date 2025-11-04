#include <stdlib.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <winbase.h>
	#include <string.h>
#endif

#if !defined(_WIN32)
	#include <unistd.h>
	#include <limits.h>
#endif

#if defined(_WIN32)
	#include "fs/absoluteness.h"
#endif

#include "fs/cwd.h"

char* get_current_directory(void) {
	/*
	Returns the current working directory.
	
	Returns a null pointer on error.
	*/
	
	int err = 0;
	char* cwd = NULL;
	
	#if defined(_WIN32)
		DWORD cwds = 0;
		
		#if defined(_UNICODE)
			wchar_t* wcwd = NULL;
			
			cwds = GetCurrentDirectoryW(0, NULL);
			
			if (cwds == 0) {
				err = -1;
				goto end;
			}
			
			wcwd = malloc((size_t) cwds * sizeof(*wcwd));
			
			if (wcwd == NULL) {
				err = -1;
				goto end;
			}
			
			cwds = GetCurrentDirectoryW(cwds, wcwd);
			
			if (cwds == 0) {
				err = -1;
				goto end;
			}
			
			cwds = (DWORD) WideCharToMultiByte(CP_UTF8, 0, wcwd, -1, NULL, 0, NULL, NULL);
			
			if (cwds == 0) {
				err = -1;
				goto end;
			}
			
			cwd = malloc((size_t) cwds);
			
			if (cwd == NULL) {
				err = -1;
				goto end;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, wcwd, -1, cwd, (int) cwds, NULL, NULL) == 0) {
				err = -1;
				goto end;
			}
		#else
			cwds = GetCurrentDirectoryA(0, NULL);
			
			if (cwds == 0) {
				err = -1;
				goto end;
			}
			
			cwd = malloc((size_t) cwds);
			
			if (cwd == NULL) {
				err = -1;
				goto end;
			}
			
			cwds = GetCurrentDirectoryA(cwds, cwd);
			
			if (cwds == 0) {
				err = -1;
				goto end;
			}
		#endif
	#else
		cwd = malloc(PATH_MAX);
		
		if (cwd == NULL) {
			err = -1;
			goto end;
		}
		
		if (getcwd(cwd, PATH_MAX) == NULL) {
			err = -1;
			goto end;
		}
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wcwd);
	#endif
	
	if (err != 0) {
		free(cwd);
		cwd = NULL;
	}
	
	return cwd;
	
}