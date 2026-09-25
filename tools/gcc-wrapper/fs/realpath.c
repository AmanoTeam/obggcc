#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <fileapi.h>
#endif

#if !defined(_WIN32)
	#include <limits.h>
	#include <errno.h>
#endif

#include "fs/sep.h"
#include "fs/realpath.h"
#include "fs/absrel.h"

char* expand_filename(const char* const filename) {
	/*
	Returns the full absolute path of a filename.
	
	Returns a null pointer on error.
	*/
	
	int err = 0;
	
	char* expanded_filename = NULL;
	
	#if defined(_WIN32)
		HANDLE handle = 0;
		DWORD size = 0;
		
		#if defined(_UNICODE)
			wchar_t* wfilename = NULL;
			wchar_t* wexpanded_filename = NULL;
			DWORD expanded_filenames = 0;
			
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
			
			handle = CreateFileW(
				wfilename,
				0,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS,
				NULL
			);
			
			if (handle == INVALID_HANDLE_VALUE) {
				size = GetFullPathNameW(wfilename, 0, NULL, NULL);
			} else {
				size = GetFinalPathNameByHandleW(handle, NULL, 0, VOLUME_NAME_DOS | FILE_NAME_NORMALIZED);
			}
			
			if (size == 0) {
				err = -1;
				goto end;
			}
			
			size++;
			
			wexpanded_filename = malloc(((size_t) size) * sizeof(*wexpanded_filename));
			
			if (wexpanded_filename == NULL) {
				err = -1;
				goto end;
			}
			
			if (handle == INVALID_HANDLE_VALUE) {
				size = GetFullPathNameW(wfilename, size, wexpanded_filename, NULL);
			} else {
				size = GetFinalPathNameByHandleW(handle, wexpanded_filename, size, VOLUME_NAME_DOS | FILE_NAME_NORMALIZED);
			}
			
			if (size == 0) {
				err = -1;
				goto end;
			}
			
			expanded_filenames = (DWORD) WideCharToMultiByte(CP_UTF8, 0, wexpanded_filename, -1, NULL, 0, NULL, NULL);
			
			if (expanded_filenames == 0) {
				err = -1;
				goto end;
			}
			
			expanded_filename = malloc((size_t) expanded_filenames);
			
			if (expanded_filename == NULL) {
				err = -1;
				goto end;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, wexpanded_filename, -1, expanded_filename, (int) expanded_filenames, NULL, NULL) == 0) {
				err = -1;
				goto end;
			}
		#else
			handle = CreateFileA(
				filename,
				0,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS,
				NULL
			);
			
			if (handle == INVALID_HANDLE_VALUE) {
				size = GetFullPathNameA(filename, 0, NULL, NULL);
			} else {
				size = GetFinalPathNameByHandleA(handle, NULL, 0, VOLUME_NAME_DOS | FILE_NAME_NORMALIZED);
			}
			
			if (size == 0) {
				err = -1;
				goto end;
			}
			
			size++;
			
			expanded_filename = malloc((size_t) size);
			
			if (expanded_filename == NULL) {
				err = -1;
				goto end;
			}
			
			if (handle == INVALID_HANDLE_VALUE) {
				size = GetFullPathNameA(filename, size, expanded_filename, NULL);
			} else {
				size = GetFinalPathNameByHandleA(handle, expanded_filename, size, VOLUME_NAME_DOS | FILE_NAME_NORMALIZED);
			}
			
			if (size == 0) {
				err = -1;
				goto end;
			}
		#endif
		
		size = (DWORD) strlen(WIN10_LONG_PATH_PREFIX_S);
		
		if (strncmp(expanded_filename, WIN10_LONG_PATH_PREFIX_S, (size_t) size) == 0) {
			memmove(expanded_filename, expanded_filename + size, strlen(expanded_filename + size) + 1);
		}
	#else
		char* tmp = NULL;
		
		const char* pos = NULL;
		char ch = 0;
		
		size_t index = 0;
		size_t len = 0;
		size_t size = 0;
		
		expanded_filename = malloc(PATH_MAX);
		
		if (expanded_filename == NULL) {
			err = -1;
			goto end;
		}
		
		errno = 0;
		
		if (realpath(filename, expanded_filename) == NULL && errno != ENOENT) {
			err = -1;
			goto end;
		}
		
		if (errno != ENOENT) {
			goto end;
		}
		
		tmp = malloc(PATH_MAX);
		
		if (tmp == NULL) {
			err = -1;
			goto end;
		}
		
		if (isrelative(filename) && filename[0] != '.') {
			strcpy(tmp, ".");
			strcat(tmp, PATHSEP_S);
			strcat(tmp, filename);
			
			free(expanded_filename);
			expanded_filename = expand_filename(tmp);
			
			goto end;
		}
		
		len = strlen(filename);
		
		for (index = len ; index-- > 0 ;) {
			pos = &filename[index];
			ch = *pos;
			
			if (ch != PATHSEP) {
				continue;
			}
			
			size = (size_t) (pos - filename);
			
			tmp[0] = '\0';
			
			strncat(tmp, filename, size);
			
			if (realpath(tmp, expanded_filename) == NULL) {
				continue;
			}
			
			strcpy(tmp, expanded_filename);
			strcat(tmp, pos);
			
			free(expanded_filename);
			expanded_filename = tmp;
			
			tmp = NULL;
			
			break;
		}
	#endif
	
	end:;
	
	#if defined(_WIN32)
		CloseHandle(handle);
	#endif
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wfilename);
		free(wexpanded_filename);
	#endif
	
	#if !defined(_WIN32)
		free(tmp);
	#endif
	
	if (err != 0) {
		free(expanded_filename);
		expanded_filename = NULL;
	}
	
	return expanded_filename;
	
}
