#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <fileapi.h>
#endif

#if !defined(_WIN32)
	#include <errno.h>
	#include <sys/stat.h>
#endif

#if defined(_WIN32)
	#include "fs/absrel.h"
#endif

#include "fs/sep.h"
#include "fs/mkdir.h"

static int raw_create_dir(const char* const directory) {
	/*
	Try to create one directory (not the whole path).
	
	This is a thin wrapper over mkdir() (or alternatives on other systems),
	so in case of a pre-existing path we don't check that it is a directory.
	
	Returns (1) on success, (0) if it already exists, (-1) on error.
	*/
	
	int err = 0;
	
	#if defined(_WIN32)
		BOOL status = FALSE;
		
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
			
			status = CreateDirectoryW(wdirectory, NULL);
		#else
			status = CreateDirectoryA(directory, NULL);
		#endif
		
		if (!status) {
			if (GetLastError() != ERROR_ALREADY_EXISTS) {
				err = -1;
				goto end;
			}
			
			goto end;
		}
	#else
		if (mkdir(directory, 0777) == -1) {
			if (errno == EEXIST) {
				goto end;
			}
			
			#if defined(__HAIKU__)
				if (errno == EROFS) {
					goto end;
				}
			#endif
			
			err = -1;
			goto end;
		}
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wdirectory);
	#endif
	
	return err;
	
}

int create_directory(const char* const directory) {
	/*
	Create a directory.
	
	The directory may contain several subdirectories that do not exist yet.
	The full path is created.
	
	It does not fail if the directory already exists.
	
	Returns (0) on success, (-1) on error.
	*/
	
	int err = 0;
	
	size_t index = 0;
	size_t size = 0;
	size_t len = 0;
	
	char* path = NULL;
	const char* start = directory;
	
	#if defined(_WIN32)
		int omit_next = isabsolute(directory);
	#endif
	
	len = strlen(directory) + 1;
	
	for (index = 1; index < len; index++) {
		const char* const pos = &directory[index];
		
		const unsigned char cur = pos[0];
		const unsigned char prev = (index > 1) ? *(pos - 1) : PATHSEP;
		
		if (!(cur == PATHSEP || (cur == '\0' && prev != PATHSEP))) {
			continue;
		}
		
		#if defined(_WIN32)
			if (omit_next) {
				omit_next = 0;
				continue;
			}
		#endif
		
		size = (size_t) (pos - start);
		path = malloc(size + 1);
		
		if (path == NULL) {
			err = -1;
			goto end;
		}
		
		memcpy(path, start, size);
		path[size] = '\0';
		
		if (raw_create_dir(path) == -1) {
			err = -1;
			goto end;
		}
		
		free(path);
		path = NULL;
	}
	
	end:;
	
	free(path);
	
	return err;
	
}
