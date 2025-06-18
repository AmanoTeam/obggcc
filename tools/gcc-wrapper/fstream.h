#if !defined(FSTREAM_H)
#define FSTREAM_H

#include <stdlib.h>

#if defined(_WIN32)
	#include <windows.h>
#endif

#if !defined(_WIN32)
	#include <stdio.h>
	#include <sys/types.h>
#endif

#define FSTREAM_SUCCESS (0)
#define FSTREAM_ERROR (-1)
#define FSTREAM_EOF (0)

enum FStreamMode {
	FSTREAM_WRITE,
	FSTREAM_READ,
	FSTREAM_APPEND
};

typedef enum FStreamMode fstream_mode_t;

enum FStreamSeek {
	FSTREAM_SEEK_BEGIN,
	FSTREAM_SEEK_CURRENT,
	FSTREAM_SEEK_END
};

typedef enum FStreamSeek fstream_seek_t;

struct FStream {
	fstream_mode_t mode;
#if defined(_WIN32)
	HANDLE stream;
#else
	FILE* stream;
#endif
};

typedef struct FStream fstream_t;

fstream_t* fstream_open(const char* const filename, const fstream_mode_t mode);
int fstream_lock(fstream_t* const stream);
ssize_t fstream_read(fstream_t* const stream, char* const buffer, const size_t size);
int fstream_write(fstream_t* const stream, const char* const buffer, const size_t size);
int fstream_seek(fstream_t* const stream, const long int offset, const fstream_seek_t method);
long int fstream_tell(fstream_t* const stream);
int fstream_close(fstream_t* const stream);

#endif
