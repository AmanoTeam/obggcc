#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
	#include <windows.h>
#endif

#if !defined(_WIN32)
	#include <dirent.h>
#endif

#if defined(__HAIKU__)
	#include <sys/stat.h>
#endif

#include "walkdir.h"

#if defined(_WIN32)
	#include "fs/absrel.h"
#endif

#if defined(_WIN32) || defined(__HAIKU__)
	#include "fs/sep.h"
#endif

#if defined(_WIN32)
	static const char ASTERISK[] = "*";
#endif

int walkdir_init(walkdir_t* const walkdir, const char* const directory) {
	
	int err = 0;
	
	#if defined(_WIN32)
		#if defined(_UNICODE)
			int wpatterns = 0;
			int wpathseps = 0;
			int wasterisks = 0;
			
			wchar_t* wpathsep = NULL;
			wchar_t* wasterisk = NULL;
			wchar_t* wpattern = NULL;
			
			/* This prefix is required to support long paths in Windows 10+ */
			const size_t prefixs = isabsolute(directory) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			wpatterns = MultiByteToWideChar(CP_UTF8, 0, directory, -1, NULL, 0);
			
			if (wpatterns == 0) {
				err = -1;
				goto end;
			}
			
			wpathseps = MultiByteToWideChar(CP_UTF8, 0, PATHSEP_S, -1, NULL, 0);
			
			if (wpathseps == 0) {
				err = -1;
				goto end;
			}
			
			wpathsep = malloc(((size_t) wpathseps) * sizeof(*wpathsep));
			
			if (wpathsep == NULL) {
				err = -1;
				goto end;
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, PATHSEP_S, -1, wpathsep, wpathseps) == 0) {
				err = -1;
				goto end;
			}
			
			wasterisks = MultiByteToWideChar(CP_UTF8, 0, ASTERISK, -1, NULL, 0);
			
			if (wasterisks == 0) {
				err = -1;
				goto end;
			}
			
			wasterisk = malloc(((size_t) wasterisks) * sizeof(*wasterisk));
			
			if (wasterisk == NULL) {
				err = -1;
				goto end;
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, ASTERISK, -1, wasterisk, wasterisks) == 0) {
				err = -1;
				goto end;
			}
			
			wpattern = malloc((prefixs + (size_t) wpatterns + wcslen(wpathsep) + wcslen(wasterisk)) * sizeof(*wpattern));
			
			if (wpattern == NULL) {
				err = -1;
				goto end;
			}
			
			if (prefixs > 0) {
				wcscpy(wpattern, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, directory, -1, wpattern + prefixs, wpatterns) == 0) {
				err = -1;
				goto end;
			}
			
			wcscat(wpattern, wpathsep);
			wcscat(wpattern, wasterisk);
			
			walkdir->handle = FindFirstFileW(wpattern, &walkdir->data);
		#else
			char* pattern = malloc(strlen(directory) + strlen(PATHSEP_S) + strlen(ASTERISK) + 1);
			
			if (pattern == NULL) {
				err = -1;
				goto end;
			}
			
			strcpy(pattern, directory);
			strcat(pattern, PATHSEP_S);
			strcat(pattern, ASTERISK);
			
			walkdir->handle = FindFirstFileA(pattern, &walkdir->data);
		#endif
		
		if (walkdir->handle == INVALID_HANDLE_VALUE) {
			err = -1;
			goto end;
		}
	#else
		walkdir->dir = opendir(directory);
		
		if (walkdir->dir == NULL) {
			err = -1;
			goto end;
		}
		
		#if defined(__HAIKU__)
			walkdir->directory = malloc(strlen(directory) + 1);
			
			if (walkdir->directory == NULL) {
				err = -1;
				goto end;
			}
			
			strcpy(walkdir->directory, directory);
		#endif
	#endif
	
	end:;
	
	#if defined(_WIN32)
		#if defined(_UNICODE)
			free(wasterisk);
			free(wpathsep);
			free(wpattern);
		#else
			free(pattern);
		#endif
	#endif
	
	if (err != 0) {
		walkdir_free(walkdir);
	}
	
	return err;
	
}

const walkdir_item_t* walkdir_next(walkdir_t* const walkdir) {
	
	int err = 0;
	
	#if defined(_WIN32) && defined(_UNICODE)
		int names = 0;
	#endif
	
	#if !defined(_WIN32)
		const struct dirent* item = NULL;
	#endif
	
	#if defined(__HAIKU__)
		char* path = NULL;
		struct stat st = {0};
	#endif
	
	const walkdir_item_t* next = &walkdir->item;
	
	free(walkdir->item.name);
	walkdir->item.name = NULL;
	
	#if defined(_WIN32)
		#if defined(_UNICODE)
			if (walkdir->item.index > 0) {
				if (FindNextFileW(walkdir->handle, &walkdir->data) == 0) {
					err = -1;
					goto end;
				}
			}
			
			names = WideCharToMultiByte(CP_UTF8, 0, walkdir->data.cFileName, -1, NULL, 0, NULL, NULL);
			
			if (names == 0) {
				err = -1;
				goto end;
			}
			
			walkdir->item.name = malloc((size_t) names);
			
			if (walkdir->item.name == NULL) {
				err = -1;
				goto end;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, walkdir->data.cFileName, -1, walkdir->item.name, names, NULL, NULL) == 0) {
				err = -1;
				goto end;
			}
		#else
			if (walkdir->item.index > 0) {
				if (FindNextFileA(walkdir->handle, &walkdir->data) == 0) {
					err = -1;
					goto end;
				}
			}
			
			walkdir->item.name = malloc(strlen(walkdir->data.cFileName) + 1);
			
			if (walkdir->item.name == NULL) {
				err = -1;
				goto end;
			}
			
			strcpy(walkdir->item.name, walkdir->data.cFileName);
		#endif
		
		walkdir->item.type = (walkdir->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0 ? WALKDIR_ITEM_DIRECTORY : WALKDIR_ITEM_FILE;
	#else
		item = readdir(walkdir->dir);
		
		if (item == NULL) {
			err = -1;
			goto end;
		}
		
		walkdir->item.name = malloc(strlen(item->d_name) + 1);
		
		if (walkdir->item.name == NULL) {
			err = -1;
			goto end;
		}
		
		strcpy(walkdir->item.name, item->d_name);
		
		#if defined(__HAIKU__)
			path = malloc(strlen(walkdir->directory) + strlen(PATHSEP_S) + strlen(item->d_name) + 1);
			
			if (path == NULL) {
				err = -1;
				goto end;
			}
			
			strcpy(path, walkdir->directory);
			strcat(path, PATHSEP_S);
			strcat(path, item->d_name);
			
			if (stat(path, &st) == -1) {
				err = -1;
				goto end;
			}
			
			switch (st.st_mode & S_IFMT) {
				case S_IFDIR:
				case S_IFBLK:
					walkdir->item.type = WALKDIR_ITEM_DIRECTORY;
					break;
				case S_IFLNK:
				case S_IFIFO:
				case S_IFREG:
				case S_IFSOCK:
				case S_IFCHR:
					walkdir->item.type = WALKDIR_ITEM_FILE;
					break;
				default:
					walkdir->item.type = WALKDIR_ITEM_UNKNOWN;
					break;
			}
		#else
			switch (item->d_type) {
				case DT_DIR:
				case DT_BLK:
					walkdir->item.type = WALKDIR_ITEM_DIRECTORY;
					break;
				case DT_LNK:
				case DT_FIFO:
				case DT_REG:
				case DT_SOCK:
				case DT_CHR:
					walkdir->item.type = WALKDIR_ITEM_FILE;
					break;
				case DT_UNKNOWN:
					walkdir->item.type = WALKDIR_ITEM_UNKNOWN;
					break;
			}
		#endif
	#endif
	
	walkdir->item.index++;
	
	end:;
	
	#if defined(__HAIKU__)
		free(path);
	#endif
	
	if (err != 0) {
		walkdir_free(walkdir);
		next = NULL;
	}
	
	return next;
	
}

void walkdir_free(walkdir_t* const walkdir) {
	
	#if defined(_WIN32)
		FindClose(walkdir->handle);
		walkdir->handle = NULL;
	#endif
	
	#if !defined(_WIN32)
		if (walkdir->dir != NULL) {
			closedir(walkdir->dir);
			walkdir->dir = NULL;
		}
	#endif
	
	free(walkdir->item.name);
	
	walkdir->item.index = 0;
	walkdir->item.name = NULL;
	walkdir->item.type = WALKDIR_ITEM_UNKNOWN;
	
	#if defined(__HAIKU__)
		free(walkdir->directory);
		walkdir->directory = NULL;
	#endif
	
}
