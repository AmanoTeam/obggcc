#include <stdlib.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <fileapi.h>
	#include <string.h>
#endif

#if !defined(_WIN32)
	#include <errno.h>
	#include <sys/stat.h>
#endif

#if defined(_WIN32)
	#include "fs/absoluteness.h"
	#include "fs/sep.h"
#endif

#include "fs/exists.h"

int directory_exists(const char* const directory) {
	/*
	Checks if directory exists.
	
	Returns (1) if directory exists, (0) if it does not exists, (-1) on error.
	*/
	
	int err = 0;
	
	#if defined(_WIN32)
		DWORD error = 0;
		DWORD attributes = 0;
		
		#if defined(_UNICODE)
			wchar_t* wdirectory = NULL;
			
			/* This prefix is required to support long paths in Windows 10+ */
			const size_t prefixs = isabsolute(directory) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			const int wdirectorys = MultiByteToWideChar(CP_UTF8, 0, directory, -1, NULL, 0);
			
			if (wdirectorys == 0) {
				err = -1;
				goto end;
			}
			
			wdirectory = malloc((prefixs + (size_t) wdirectorys) * sizeof(*wdirectory));
			
			if (wdirectory == NULL) {
				err = -1;
				goto end;
			}
			
			if (prefixs > 0) {
				wcscpy(wdirectory, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, directory, -1, wdirectory + prefixs, wdirectorys) == 0) {
				err = -1;
				goto end;
			}
			
			attributes = GetFileAttributesW(wdirectory);
		#else
			attributes = GetFileAttributesA(directory);
		#endif
		
		if (attributes == INVALID_FILE_ATTRIBUTES) {
			error = GetLastError();
			
			if (!(error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)) {
				err = -1;
				goto end;
			}
			
			goto end;
		}
		
		err = (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	#else
		struct stat st = {0};
		
		if (stat(directory, &st) == -1) {
			if (errno != ENOENT) {
				err = -1;
				goto end;
			}
			
			goto end;
		}
		
		err = S_ISDIR(st.st_mode);
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wdirectory);
	#endif
	
	return err;
	
}

int file_exists(const char* const filename) {
	/*
	Checks if file exists and is a regular file or symlink.
	
	Returns (1) if file exists, (0) if it does not exists, (-1) on error.
	*/
	
	int err = 0;
	
	#if defined(_WIN32)
		DWORD error = 0;
		DWORD attributes = 0;
		
		#if defined(_UNICODE)
			wchar_t* wfilename = NULL;
			
			/* This prefix is required to support long paths in Windows 10+ */
			const size_t prefixs = isabsolute(filename) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			const int wfilenames = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
			
			if (wfilenames == 0) {
				err = -1;
				goto end;
			}
			
			wfilename = malloc((prefixs + (size_t) wfilenames) * sizeof(*wfilename));
			
			if (wfilename == NULL) {
				err = -1;
				goto end;
			}
			
			if (prefixs > 0) {
				wcscpy(wfilename, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, filename, -1, wfilename + prefixs, wfilenames) == 0) {
				err = -1;
				goto end;
			}
			
			attributes = GetFileAttributesW(wfilename);
		#else
			attributes = GetFileAttributesA(filename);
		#endif
		
		if (attributes == INVALID_FILE_ATTRIBUTES) {
			error = GetLastError();
			
			if (!(error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)) {
				err = -1;
				goto end;
			}
			
			goto end;
		}
		
		err = (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
	#else
		struct stat st = {0};
		
		if (stat(filename, &st) == -1) {
			if (errno != ENOENT) {
				err = -1;
				goto end;
			}
			
			goto end;
		}
		
		err = S_ISREG(st.st_mode);
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wfilename);
	#endif
	
	return err;
	
}