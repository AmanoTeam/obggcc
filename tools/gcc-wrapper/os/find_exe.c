#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#if defined(_WIN32)
	#include <windows.h>
#endif

#include "fs/sep.h"
#include "fs/exists.h"
#include "os/find_exe.h"

#if defined(_WIN32) && defined(_UNICODE)
	static const wchar_t WENV_PATH[] = L"PATH";
#else
	static const char ENV_PATH[] = "PATH";
#endif

char* find_exe(const char* const name) {
	
	char* executable = NULL;
	char* path = NULL;
	const char* component = NULL;
	
	int err = 0;
	
	size_t index = 0;
	size_t size = 0;
	size_t length = 0;
	
	#if defined(_WIN32)
		const char* const executable_extension = ".exe";
		const unsigned char separator = ';';
	#else
		const char* const executable_extension = "";
		const unsigned char separator = ':';
	#endif
	
	#if defined(_WIN32) && defined(_UNICODE)
		int paths = 0;
		
		const wchar_t* const wpath = _wgetenv(WENV_PATH);
		
		if (wpath == NULL) {
			err = -1;
			goto end;
		}
		
		paths = WideCharToMultiByte(CP_UTF8, 0, wpath, -1, NULL, 0, NULL, NULL);
		
		if (paths == 0) {
			err = -1;
			goto end;
		}
		
		path = malloc((size_t) paths);
		
		if (path == NULL) {
			err = -1;
			goto end;
		}
		
		if (WideCharToMultiByte(CP_UTF8, 0, wpath, -1, path, paths, NULL, NULL) == 0) {
			err = -1;
			goto end;
		}
	#else
		path = getenv(ENV_PATH);
		
		if (path == NULL) {
			err = -1;
			goto end;
		}
	#endif
	
	component = path;
	length = strlen(path) + 1;
	
	for (index = 0; index < length; index++) {
		const char* const pos = &path[index];
		const unsigned char ch = *pos;
		
		if (!(ch == separator || ch == '\0')) {
			continue;
		}
		
		size = (size_t) (pos - component);
		executable = malloc(size + strlen(PATHSEP_S) + strlen(name) + strlen(executable_extension) + 1);
		
		if (executable == NULL) {
			err = -1;
			goto end;
		}
		
		memcpy(executable, component, size);
		executable[size] = '\0';
		
		strcat(executable, PATHSEP_S);
		strcat(executable, name);
		strcat(executable, executable_extension);
		
		if (file_exists(executable)) {
			goto end;
		}
		
		free(executable);
		executable = NULL;
		
		component = (pos + 1);
	}
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(path);
	#endif
	
	if (err != 0) {
		free(executable);
		executable = NULL;
	}
	
	return executable;
	
}
