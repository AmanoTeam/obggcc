#include <stdlib.h>
#include <string.h>

#if defined(__OpenBSD__)
	#include <string.h>
#endif

#if defined(_WIN32)
	#include <windows.h>
	#include <fileapi.h>
#endif

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__OpenBSD__)
	#include <sys/types.h>
	#include <sys/sysctl.h>
#endif

#if defined(__APPLE__)
	#include <mach-o/dyld.h>
#endif

#if defined(__HAIKU__)
	#include <FindDirectory.h>
#endif

#if !defined(_WIN32)
	#include <unistd.h>
	#include <limits.h>
#endif

#if defined(_WIN32) || defined(__OpenBSD__)
	#include "path.h"
#endif

#if defined(__OpenBSD__)
	#include "fs/sep.h"
	#include "fs/exists.h"
#endif

#include "fs/getexec.h"
#include "fs/parentpath.h"
#include "fs/basename.h"

char* get_app_filename(void) {
	/*
	Returns the filename of the application's executable.
	
	Returns a null pointer on error.
	*/
	
	int err = 0;
	
	char* app_filename = NULL;
	
	#if defined(_WIN32)
		#if defined(_UNICODE)
			wchar_t* wfilename = NULL;
			DWORD filenames = 0;
			
			filenames = GetModuleFileNameW(0, NULL, 0);
			
			if (filenames == 0) {
				err = -1;
				goto end;
			}
			
			filenames++;
			
			wfilename = malloc(((size_t) filenames) * sizeof(*wfilename));
			
			if (wfilename == NULL) {
				err = -1;
				goto end;
			}
			
			filenames = GetModuleFileNameW(0, wfilename, filenames);
			
			if (filenames == 0) {
				err = -1;
				goto end;
			}
			
			app_filename = malloc((size_t) filenames);
			
			if (app_filename == NULL) {
				err = -1;
				goto end;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, wfilename, -1, app_filename, (int) filenames, NULL, NULL) == 0) {
				err = -1;
				goto end;
			}
		#else
			app_filename = malloc(PATH_MAX);
			
			if (app_filename == NULL) {
				err = -1;
				goto end;
			}
			
			if (GetModuleFileNameA(0, app_filename, PATH_MAX) == 0) {
				err = -1;
				goto end;
			}
		#endif
	#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
		#if defined(__NetBSD__)
			const int call[] = {
				CTL_KERN,
				KERN_PROC_ARGS,
				-1,
				KERN_PROC_PATHNAME
			};
		#else
			const int call[] = {
				CTL_KERN,
				KERN_PROC,
				KERN_PROC_PATHNAME,
				-1
			};
		#endif
		
		size_t size = PATH_MAX;
		app_filename = malloc(size);
		
		if (app_filename == NULL) {
			err = -1;
			goto end;
		}
		
		if (sysctl(call, sizeof(call) / sizeof(*call), app_filename, &size, NULL, 0) == -1) {
			err = -1;
			goto end;
		}
	#elif defined(__OpenBSD__)
		const pid_t pid = getpid();
		
		const int call[] = {
			CTL_KERN,
			KERN_PROC_ARGS,
			pid,
			KERN_PROC_ARGV
		};
		
		const char* path = NULL;
		const char* name = NULL;
		const char* start = NULL;
		
		char** argv = NULL;
		char* cwd = NULL;
		char* tmp = NULL;
		
		int status = 0;
		size_t index = 0;
		size_t size = 0;
		
		app_filename = malloc(PATH_MAX);
		
		if (app_filename == NULL) {
			err = -1;
			goto end;
		}
		
		app_filename[0] = '\0';
		
		if (sysctl(call, sizeof(call) / sizeof(*call), NULL, &size, NULL, 0) == -1) {
			err = -1;
			goto end;
		}
		
		argv = malloc(size);
		
		if (argv == NULL) {
			err = -1;
			goto end;
		}
		
		if (sysctl(call, sizeof(call) / sizeof(*call), argv, &size, NULL, 0) == -1) {
			err = -1;
			goto end;
		}
		
		name = argv[0];
		
		if (isabsolute(name)) {
			realpath(name, app_filename);
			goto end;
		}
		
		/*
		Not an absolute path, check if it's relative to the current
		working directory.
		*/
		for (index = 1; index < strlen(name); index++) {
			const char unsigned ch = name[index];
			
			status = (ch == PATHSEP);
			
			if (status) {
				break;
			}
		}
		
		if (status) {
			cwd = malloc(PATH_MAX);
			
			if (cwd == NULL) {
				err = -1;
				goto end;
			}
			
			if (getcwd(cwd, PATH_MAX) == NULL) {
				err = -1;
				goto end;
			}
			
			tmp = malloc(strlen(cwd) + strlen(PATHSEP_S) + strlen(name) + 1);
			
			if (tmp == NULL) {
				err = -1;
				goto end;
			}
			
			strcpy(tmp, cwd);
			strcat(tmp, PATHSEP_S);
			strcat(tmp, name);
			
			realpath(tmp, app_filename);
			
			goto end;
		}
		
		path = getenv("PATH");
		
		if (path == NULL) {
			err = -1;
			goto end;
		}
		
		start = path;
		
		for (index = 0; index < strlen(path) + 1; index++) {
			const char* const pos = &path[index];
			const unsigned char ch = *pos;
			
			if (!(ch == ':' || ch == '\0')) {
				continue;
			}
			
			size = (size_t) (pos - start);
			
			tmp = malloc(size + strlen(PATHSEP_S) + strlen(name) + 1);
			
			if (tmp == NULL) {
				goto end;
			}
			
			memcpy(tmp, start, size);
			tmp[size] = '\0';
			
			strcat(tmp, PATHSEP_S);
			strcat(tmp, name);
			
			status = file_exists(tmp);
			
			if (status) {
				realpath(tmp, app_filename);
				goto end;
			}
			
			if (status == -1) {
				err = -1;
				goto end;
			}
			
			free(tmp);
			tmp = NULL;
			
			start = (pos + 1);
		}
	#elif defined(__APPLE__)
		uint32_t paths = PATH_MAX;
		char* path = malloc((size_t) paths);
		
		if (path == NULL) {
			err = -1;
			goto end;
		}
		
		if (_NSGetExecutablePath(path, &paths) == -1) {
			err = -1;
			goto end;
		}
		
		app_filename = malloc(PATH_MAX);
		
		if (app_filename == NULL) {
			err = -1;
			goto end;
		}
		
		if (realpath(path, app_filename) == NULL) {
			err = -1;
			goto end;
		}
	#elif defined(__HAIKU__)
		app_filename = malloc(PATH_MAX);
		
		if (app_filename == NULL) {
			err = -1;
			goto end;
		}
		
		if (find_path(NULL, B_FIND_PATH_IMAGE_PATH, NULL, app_filename, PATH_MAX) != B_OK) {
			err = -1;
			goto end;
		}
	#else
		ssize_t wsize = 0;
		
		app_filename = malloc(PATH_MAX);
		
		if (app_filename == NULL) {
			err = -1;
			goto end;
		}
		
		wsize = readlink("/proc/self/exe", app_filename, PATH_MAX);
		
		if (wsize == -1) {
			err = -1;
			goto end;
		}
		
		app_filename[wsize] = '\0';
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wfilename);
	#endif
	
	#if defined(__OpenBSD__)
		free(argv);
		free(cwd);
		free(tmp);
	#endif
	
	#if defined(__APPLE__)
		free(path);
	#endif
	
	if (err != 0) {
		free(app_filename);
		app_filename = NULL;
	}
	
	return app_filename;
	
}

char* get_app_directory(void) {
	
	size_t status = 0;
	size_t size = 0;
	
	unsigned char ch = 0;
	
	char* app_filename = NULL;
	char* app_directory = NULL;
	
	const char* name = NULL;
	
	app_filename = get_app_filename();
	
	if (app_filename == NULL) {
		goto end;
	}
	
	app_directory = malloc(strlen(app_filename) + 1);
	
	if (app_directory == NULL) {
		goto end;
	}
	
	get_parent_path(app_filename, app_directory, 1);
	
	name = basename(app_directory);
	
	ch = name[0];
	size = strlen(name);
	
	status = (
		(size == 4 && (ch == 'x'  || ch == 's') && strcmp(name + 1, "bin") == 0) ||
		strcmp(name, "bin") == 0
	);
	
	if (!status) {
		goto end;
	}
	
	get_parent_path(app_filename, app_directory, 2);
	
	end:;
	
	free(app_filename);
	
	return app_directory;
	
}
