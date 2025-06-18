#include <stdlib.h>
#include <stdio.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <fileapi.h>
#endif

#if !defined(_WIN32)
	#include <stdio.h>
	#include <sys/file.h>
#endif

#include "fstream.h"

#if defined(_WIN32) && defined(_UNICODE)
	#include "fs/absoluteness.h"
	#include "fs/sep.h"
#endif

fstream_t* fstream_open(const char* const filename, const fstream_mode_t mode) {
	/*
	Opens a file on disk.
	
	Returns a null pointer on error.
	*/
	
	fstream_t* stream = NULL;
	
	#if defined(_WIN32)
		#if defined(_UNICODE)
			size_t prefixs = 0;
			wchar_t* wfilename = NULL;
			int wfilenames = 0;
		#endif
		
		DWORD access = 0;
		DWORD disposition = 0;
		const DWORD flags = FILE_ATTRIBUTE_NORMAL;
		
		HANDLE handle = 0;
		
		switch (mode) {
			case FSTREAM_WRITE:
				access |= GENERIC_WRITE;
				disposition |= CREATE_ALWAYS;
				break;
			case FSTREAM_READ:
				access |= GENERIC_READ;
				disposition |= OPEN_EXISTING;
				break;
			case FSTREAM_APPEND:
				access |= FILE_APPEND_DATA;
				disposition |= OPEN_EXISTING;
				break;
		}
		
		#if defined(_UNICODE)
			/* This prefix is required to support long paths in Windows 10+ */
			prefixs = isabsolute(filename) ? wcslen(WIN10_LONG_PATH_PREFIX) : 0;
			
			wfilenames = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
			
			if (wfilenames == 0) {
				return NULL;
			}
			
			wfilename = malloc((prefixs + (size_t) wfilenames) * sizeof(*wfilename));
			
			if (prefixs > 0) {
				wcscpy(wfilename, WIN10_LONG_PATH_PREFIX);
			}
			
			if (MultiByteToWideChar(CP_UTF8, 0, filename, -1, wfilename + prefixs, wfilenames) == 0) {
				free(wfilename);
				return NULL;
			}
			
			handle = CreateFileW(wfilename, access, 0, NULL, disposition, flags, NULL);
			
			free(wfilename);
		#else
			handle = CreateFileA(filename, access, 0, NULL, disposition, flags, NULL);
		#endif
		
		if (handle == INVALID_HANDLE_VALUE) {
			return NULL;
		}
		
		if (mode == FSTREAM_APPEND) {
			if (SetFilePointer(handle, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER) {
				CloseHandle(handle);
				return NULL;
			}
		}
	#else
		const char* flags = NULL;
		FILE* file = NULL;
		
		switch (mode) {
			case FSTREAM_WRITE:
				flags = "wb";
				break;
			case FSTREAM_READ:
				flags = "rb";
				break;
			case FSTREAM_APPEND:
				flags = "a";
				break;
		}
		
		file = fopen(filename, flags);
		
		if (file == NULL) {
			return NULL;
		}
	#endif
	
	stream = malloc(sizeof(*stream));
	
	if (stream == NULL) {
		#if defined(_WIN32)
			CloseHandle(handle);
		#else
			fclose(file);
		#endif
		
		return NULL;
	}
	
	#if defined(_WIN32)
		stream->stream = handle;
	#else
		stream->stream = file;
	#endif
	
	stream->mode = mode;
	
	return stream;
	
}

int fstream_lock(fstream_t* const stream) {
	/*
	Lock the file.
	
	Returns (0) on success, (-1) on error.
	*/
	
	int status = 0;
	
	#if !defined(_WIN32)
		int fd = 0;
	#endif
	
	#if defined(_WIN32)
		status = LockFile(stream->stream, 0, 0, 0, 0) == TRUE;
	#else
		fd = fileno(stream->stream);
		
		if (fd == -1) {
			return FSTREAM_ERROR;
		}
		
		status = flock(fd, LOCK_EX) == 0;
	#endif
	
	if (!status) {
		return FSTREAM_ERROR;
	}
	
	return FSTREAM_SUCCESS;
	
}

ssize_t fstream_read(fstream_t* const stream, char* const buffer, const size_t size) {
	/*
	Reads a block of data.
	
	Returns (>=1) on success, (0) on EOF, (-1) on error.
	*/
	
	#if defined(_WIN32)
		DWORD rsize = 0;
		BOOL status = FALSE;
	#endif
	
	#if !defined(_WIN32)
		size_t rsize = 0;
	#endif
	
	if (stream->mode == FSTREAM_WRITE) {
		return FSTREAM_ERROR;
	}
	
	#if defined(_WIN32)
		status = ReadFile(stream->stream, buffer, (DWORD) size, &rsize, NULL);
		
		if (!status) {
			return FSTREAM_ERROR;
		}
		
		if (rsize > 0) {
			return (ssize_t) rsize;
		}
		
		return FSTREAM_EOF;
	#else
		rsize = fread(buffer, sizeof(*buffer), size, stream->stream);
		
		if (rsize == 0) {
			if (ferror(stream->stream) != 0) {
				return FSTREAM_ERROR;
			}
			
			return FSTREAM_EOF;
		}
		
		return (ssize_t) rsize;
	#endif
	
}

int fstream_write(fstream_t* const stream, const char* const buffer, const size_t size) {
	/*
	Writes a block of data.
	
	Returns (0) on success, (-1) on error.
	*/
	
	#if defined(_WIN32)
		DWORD wsize = 0;
		BOOL status = FALSE;
	#endif
	
	#if !defined(_WIN32)
		size_t wsize = 0;
	#endif
	
	if (stream->mode == FSTREAM_READ) {
		return FSTREAM_ERROR;
	}
	
	#if defined(_WIN32)
		status = WriteFile(stream->stream, buffer, (DWORD) size, &wsize, NULL);
		
		if (status == 0 || wsize != (DWORD) size) {
			return FSTREAM_ERROR;
		}
	#else
		wsize = fwrite(buffer, sizeof(*buffer), size, stream->stream);
		
		if (wsize != size) {
			return FSTREAM_ERROR;
		}
	#endif
	
	return FSTREAM_SUCCESS;
	
}

int fstream_seek(fstream_t* const stream, const long int offset, const fstream_seek_t method) {
	/*
	Sets the current file position.
	
	Returns (0) on success, (-1) on error.
	*/
	
	#if defined(_WIN32)
		DWORD whence = 0;
		
		switch (method) {
			case FSTREAM_SEEK_BEGIN:
				whence = FILE_BEGIN;
				break;
			case FSTREAM_SEEK_CURRENT:
				whence = FILE_CURRENT;
				break;
			case FSTREAM_SEEK_END:
				whence = FILE_END;
				break;
		}
		
		if (SetFilePointer(stream->stream, offset, NULL, whence) == INVALID_SET_FILE_POINTER) {
			return FSTREAM_ERROR;
		}
	#else
		int whence = 0;
		
		switch (method) {
			case FSTREAM_SEEK_BEGIN:
				whence = SEEK_SET;
				break;
			case FSTREAM_SEEK_CURRENT:
				whence = SEEK_CUR;
				break;
			case FSTREAM_SEEK_END:
				whence = SEEK_END;
				break;
		}
		
		if (fseeko(stream->stream, offset, whence) != 0) {
			return FSTREAM_ERROR;
		}
	#endif
	
	return FSTREAM_SUCCESS;
	
}

long int fstream_tell(fstream_t* const stream) {
	/*
	Returns the current file offset.
	
	Returns (>=0) on success, (-1) on error.
	*/
	
	#if defined(_WIN32)
		const DWORD value = SetFilePointer(stream->stream, 0, NULL, FILE_CURRENT);
		
		if (value == INVALID_SET_FILE_POINTER) {
			return FSTREAM_ERROR;
		}
	#else
		const long int value = ftello(stream->stream);
		
		if (value == -1) {
			return FSTREAM_ERROR;
		}
	#endif
	
	return (long int) value;
	
}

int fstream_close(fstream_t* const stream) {
	/*
	Closes the stream.
	
	Returns (0) on success, (-1) on error.
	*/
	
	if (stream == NULL) {
		return 0;
	}
	
	#if defined(_WIN32)
		if (stream->stream != 0) {
			const BOOL status = CloseHandle(stream->stream);
			
			if (status == 0) {
				return FSTREAM_ERROR;
			}
			
			stream->stream = 0;
		}
	#else
		if (stream->stream != NULL) {
			if (fclose(stream->stream) != 0) {
				return FSTREAM_ERROR;
			}
			
			stream->stream = NULL;
		}
	#endif
	
	free(stream);
	
	return FSTREAM_SUCCESS;
	
}
