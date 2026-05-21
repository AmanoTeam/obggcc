#if defined(_WIN32)
	#include <windows.h>
#endif

#if !defined(_WIN32)
	#include <stddef.h>
	#include <time.h>
#endif

#if defined(_WIN32)
	static const unsigned long long int WINDOWS_EPOCH_DIFF = 116444736000000000;
	static const unsigned long long int WINDOWS_HNSECS_PER_SEC = 10000000;
#endif

unsigned long long int get_time(void) {
	
	unsigned long long int seconds = 0;
	
	#if defined(_WIN32)
		FILETIME time = {0};
		GetSystemTimePreciseAsFileTime(&time);
		
		seconds = (((unsigned long long int) time.dwHighDateTime << 32 | time.dwLowDateTime) - WINDOWS_EPOCH_DIFF) / WINDOWS_HNSECS_PER_SEC;
	#else
		seconds = time(NULL);
	#endif
	
	return seconds;
	
}
