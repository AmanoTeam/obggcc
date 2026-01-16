#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <fileapi.h>
#endif

#if !defined(_WIN32)
	#include <errno.h>
	#include <unistd.h>
#endif

#if defined(_WIN32)
	#include "fs/absrel.h"
#endif

#include "walkdir.h"
#include "fs/sep.h"
#include "fs/rm.h"

int remove_file(const char* const filename) {
	/*
	Removes a file from disk.
	
	On Windows, ignores the read-only attribute.
	This does not fail if the file never existed in the first place.
	
	Returns (0) on success, (-1) on error.
	*/
	
	int err = 0;
	
	#if defined(_WIN32)
		BOOL status = FALSE;
		DWORD error = GetLastError();
		
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
			
			if (prefixs > 0) {
				wcscpy(wfilename, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, filename, -1, wfilename + prefixs, wfilenames) == 0) {
				err = -1;
				goto end;
			}
			
			status = DeleteFileW(wfilename);
		#else
			status = DeleteFileA(filename);
		#endif
		
		if (!status) {
			error = GetLastError();
			
			switch (error) {
				case ERROR_ACCESS_DENIED: {
					/*
					We could not delete the file due to lack of permissions. There are two possible causes for this:
					
					* The file is owned by other user.
					* The file has the "read-only" attribute set.
					*/
					
					/* We will assume that this is the second case and try to remove that read-only attribute. */
					#if defined(_UNICODE)
						status = SetFileAttributesW(wfilename, FILE_ATTRIBUTE_NORMAL);
					#else
						status = SetFileAttributesA(filename, FILE_ATTRIBUTE_NORMAL);
					#endif
					
					if (!status) {
						err = -1;
						goto end;
					}
					
					/* Now that this attribute is gone, let's try to delete that file again. */
					#if defined(_UNICODE)
						status = DeleteFileW(wfilename);
					#else
						status = DeleteFileA(filename);
					#endif
					
					if (!status) {
						err = -1;
						goto end;
					}
				
					break;
				}
				case ERROR_FILE_NOT_FOUND:
				case ERROR_PATH_NOT_FOUND: {
					/* The file never existed in the first place; that's not an error. */
					break;
				}
				default: {
					err = -1;
					goto end;
				}
			}
		}
	#else
		if (unlink(filename) == -1 && errno != ENOENT) {
			err = -1;
			goto end;
		}
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wfilename);
	#endif
	
	return err;
	
}

int remove_empty_directory(const char* const directory) {
	/*
	Deletes an existing empty directory.
	
	Returns (0) on success, (-1) on error.
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
			
			status = RemoveDirectoryW(wdirectory);
		#else
			status = RemoveDirectoryA(directory);
		#endif
		
		if (!status) {
			err = -1;
			goto end;
		}
	#else
		if (rmdir(directory) == -1) {
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

static int remove_recursive(const char* const directory, const int remove_itself) {
	/*
	Recursively removes a directory from disk.
	
	Returns (0) on success, (-1) on error.
	*/
	
	int err = 0;
	
	char* path = NULL;
	
	walkdir_t walkdir = {0};
	const walkdir_item_t* item = NULL;
	
	if (walkdir_init(&walkdir, directory) == -1) {
		err = -1;
		goto end;
	}
	
	while (1) {
		item = walkdir_next(&walkdir);
		
		if (item == NULL) {
			goto end;
		}
		
		if (strcmp(item->name, ".") == 0 || strcmp(item->name, "..") == 0) {
			continue;
		}
		
		path = malloc(strlen(directory) + strlen(PATHSEP_S) + strlen(item->name) + 1);
		
		if (path == NULL) {
			err = -1;
			goto end;
		}
		
		strcpy(path, directory);
		strcat(path, PATHSEP_S);
		strcat(path, item->name);
		
		switch (item->type) {
			case WALKDIR_ITEM_DIRECTORY: {
				if (remove_recursive(path, 1) == -1) {
					err = -1;
					goto end;
				}
				
				break;
			}
			case WALKDIR_ITEM_FILE:
			case WALKDIR_ITEM_UNKNOWN: {
				if (remove_file(path) == -1) {
					err = -1;
					goto end;
				}
				
				break;
			}
		}
		
		free(path);
		path = NULL;
	}
	
	end:;
	
	free(path);
	walkdir_free(&walkdir);
	
	if (err == -1) {
		return err;
	}
	
	if (remove_itself && remove_empty_directory(directory) == -1) {
		err = -1;
	}
	
	return err;
	
}

int remove_directory(const char* const directory) {
	
	return remove_recursive(directory, 1);
	
}

int remove_directory_contents(const char* const directory) {
	
	return remove_recursive(directory, 0);
	
}
