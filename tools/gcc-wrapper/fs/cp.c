#include <stdlib.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <winbase.h>
	#include <string.h>
#endif

#if defined(__APPLE__)
	#include <copyfile.h>
#endif

#if defined(_WIN32)
	#include "fs/absoluteness.h"
#endif

#if !(defined(_WIN32) || defined(__APPLE__))
	#include "fstream.h"
#endif

#include "fs/cp.h"

int copy_file(const char* const source, const char* const destination) {
	/*
	Copies a file from source to destination.
	
	On the Windows platform this will copy the source file's attributes into destination.
	On Mac OS X, copyfile() C API will be used (available since OS X 10.5).
	
	If destination already exists, the file attributes will be preserved and the content overwritten.
	
	Returns (0) on success, (-1) on error.
	*/
	
	int err = 0;
	
	#if defined(_WIN32)
		#if defined(_UNICODE)
			wchar_t* wsource = NULL;
			wchar_t* wdestination = NULL;
			
			int wfilenames = 0;
			
			/* This prefix is required to support long paths in Windows 10+ */
			size_t prefixs = isabsolute(source) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			wfilenames = MultiByteToWideChar(CP_UTF8, 0, source, -1, NULL, 0);
			
			if (wfilenames == 0) {
				err = -1;
				goto end;
			}
			
			wsource = malloc((prefixs + (size_t) wfilenames) * sizeof(*wsource));
			
			if (prefixs > 0) {
				wcscpy(wsource, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, source, -1, wsource + prefixs, wfilenames) == 0) {
				err = -1;
				goto end;
			}
			
			prefixs = isabsolute(destination) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			wfilenames = MultiByteToWideChar(CP_UTF8, 0, destination, -1, NULL, 0);
			
			if (wfilenames == 0) {
				err = -1;
				goto end;
			}
			
			wdestination = malloc((prefixs + (size_t) wfilenames) * sizeof(*wdestination));
			
			if (prefixs > 0) {
				wcscpy(wdestination, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, destination, -1, wdestination + prefixs, wfilenames) == 0) {
				err = -1;
				goto end;
			}
			
			if (CopyFileW(wsource, wdestination, FALSE) == 0) {
				err = -1;
				goto end;
			}
		#else
			if (CopyFileA(source, destination, FALSE) == 0) {
				err = -1;
				goto end;
			}
		#endif
	#elif defined(__APPLE__)
		copyfile_state_t state = copyfile_state_alloc();
		
		if (copyfile(source, destination, state, COPYFILE_DATA) != 0) {
			err = -1;
			goto end;
		}
	#else
		/* Fallback implementation which works for any platform */
		fstream_t* input = fstream_open(source, FSTREAM_READ);
		fstream_t* output = NULL;
		
		ssize_t size = 0;
		
		char chunk[8192];
		
		if (input == NULL) {
			err = -1;
			goto end;
		}
		
		output = fstream_open(destination, FSTREAM_WRITE);
		
		if (output == NULL) {
			err = -1;
			goto end;
		}
		
		while (1) {
			size = fstream_read(input, chunk, sizeof(chunk));
			
			switch (size) {
				case FSTREAM_ERROR: {
					err = -1;
					goto end;
				}
				case FSTREAM_EOF: {
					goto end;
				}
				default: {
					if (fstream_write(output, chunk, (size_t) size) == -1) {
						err = -1;
						goto end;
					}
					
					break;
				}
			}
		}
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wsource);
		free(wdestination);
	#endif
	
	#if defined(__APPLE__)
		copyfile_state_free(state);
	#endif
	
	#if !(defined(_WIN32) || defined(__APPLE__))
		fstream_close(input);
		fstream_close(output);
	#endif
	
	return err;

}
