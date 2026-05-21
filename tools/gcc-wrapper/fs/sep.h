#if !defined(PATHSEP_H)
#define PATHSEP_H

static const unsigned char PATHSEP_POSIX = '/';
static const char PATHSEP_POSIX_S[] = "/";
#define PATHSEP_POSIX_M "/"

static const unsigned char PATHSEP_DOS = '\\';
static const char PATHSEP_DOS_S[] = "\\";
#define PATHSEP_DOS_M "\\"

#if defined(_WIN32)
	static const unsigned char PATHSEP = '\\';
	static const char* const PATHSEP_S = "\\";
	#define PATHSEP_M PATHSEP_DOS_M
#else
	static const unsigned char PATHSEP = '/';
	static const char* const PATHSEP_S = "/";
	#define PATHSEP_M PATHSEP_POSIX_M
#endif

#if defined(_WIN32) && defined(_UNICODE)
	static const wchar_t WIN10_LONG_PATH_PREFIX[] = L"\\\\?\\";
#else
	static const char WIN10_LONG_PATH_PREFIX[] = "\\\\?\\";
#endif

#endif
