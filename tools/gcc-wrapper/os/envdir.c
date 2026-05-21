#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <fileapi.h>
#endif

#include "fs/stripsep.h"
#include "fs/sep.h"
#include "fs/exists.h"
#include "os/envdir.h"

#if defined(_WIN32)
	#if defined(_UNICODE)
		static const wchar_t WENV_APPDATA[] = L"APPDATA";
	#else
		static const char ENV_USERPROFILE[] = "USERPROFILE";
		static const char ENV_APPDATA[] = "APPDATA";
	#endif
#endif

#if !defined(_WIN32)
	static const char ENV_XDG_CONFIG_HOME[] = "XDG_CONFIG_HOME";
	static const char ENV_HOME[] = "HOME";
	
	static const char* const ENV_TEMPORARY_DIRECTORY[] = {
		"TMPDIR",
		"TEMP",
		"TMP",
		"TEMPDIR"
	};
	
	static const char* const TEMPORARY_DIRECTORIES[] = {
		"/var/tmp",
		"/usr/tmp",
		"/tmp"
	};
	
	static const char DEFAULT_CONFIGURATION_DIRECTORY[] = ".config";
#endif


char* get_config_dir(void) {
	/*
	Returns the config directory of the current user for applications.
	*/
	
	int err = 0;
	
	char* configuration_directory = NULL;
	
	#if defined(_WIN32)
		#if defined(_UNICODE)
			int directorys = 0;
			
			const wchar_t* const wdirectory = _wgetenv(WENV_APPDATA);
			
			if (wdirectory == NULL) {
				err = -1;
				goto end;
			}
			
			directorys = WideCharToMultiByte(CP_UTF8, 0, wdirectory, -1, NULL, 0, NULL, NULL);
			
			if (directorys == 0) {
				err = -1;
				goto end;
			}
			
			configuration_directory = malloc((size_t) directorys);
			
			if (configuration_directory == NULL) {
				err = -1;
				goto end;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, wdirectory, -1, configuration_directory, directorys, NULL, NULL) == 0) {
				err = -1;
				goto end;
			}
		#else
			const char* const directory = getenv(ENV_APPDATA);
			
			if (directory == NULL) {
				err = -1;
				goto end;
			}
			
			configuration_directory = malloc(strlen(directory) + 1);
			
			if (configuration_directory == NULL) {
				err = -1;
				goto end;
			}
			
			strcpy(configuration_directory, directory);
		#endif
	#else
		const char* const directory = getenv(ENV_XDG_CONFIG_HOME);
		const char* const home = getenv(ENV_HOME);
		
		if (directory != NULL) {
			configuration_directory = malloc(strlen(directory) + 1);
			
			if (configuration_directory == NULL) {
				err = -1;
				goto end;
			}
			
			strcpy(configuration_directory, directory);
			
			goto end;
		}
		
		if (home == NULL) {
			err = -1;
			goto end;
		}
		
		configuration_directory = malloc(strlen(home) + strlen(PATHSEP_S) + strlen(DEFAULT_CONFIGURATION_DIRECTORY) + 1);
		
		if (configuration_directory == NULL) {
			err = -1;
			goto end;
		}
		
		strcpy(configuration_directory, home);
		strcat(configuration_directory, PATHSEP_S);
		strcat(configuration_directory, DEFAULT_CONFIGURATION_DIRECTORY);
	#endif
	
	end:;
	
	if (err != 0) {
		free(configuration_directory);
		configuration_directory = NULL;
	}
	
	if (err == 0) {
		strip_path_sep(configuration_directory);
	}
	
	return configuration_directory;
	
}

char* get_temp_dir(void) {
	/*
	Returns the temporary directory of the current user for applications to
	save temporary files in.
	*/
	
	int err = 0;
	
	char* temporary_directory = NULL;
	
	#if defined(_WIN32)
		DWORD directorys = 0;
		
		#if defined(_UNICODE)
			wchar_t* wdirectory = NULL;
			
			directorys = GetTempPathW(0, NULL);
			
			if (directorys == 0) {
				return NULL;
			}
			
			directorys++;
			
			wdirectory = malloc((size_t) directorys * sizeof(*wdirectory));
			
			if (wdirectory == NULL) {
				err = -1;
				goto end;
			}
			
			directorys = GetTempPathW((DWORD) directorys, wdirectory);
			
			if (directorys == 0) {
				err = -1;
				goto end;
			}
			
			directorys = (DWORD) WideCharToMultiByte(CP_UTF8, 0, wdirectory, -1, NULL, 0, NULL, NULL);
			
			if (directorys == 0) {
				err = -1;
				goto end;
			}
			
			temporary_directory = malloc((size_t) directorys);
			
			if (temporary_directory == NULL) {
				err = -1;
				goto end;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, wdirectory, -1, temporary_directory, (int) directorys, NULL, NULL) == 0) {
				err = -1;
				goto end;
			}
		#else
			directorys = GetTempPathA(0, NULL);
			
			if (directorys == 0) {
				err = -1;
				goto end;
			}
			
			directorys++;
			
			temporary_directory = malloc(directorys);
			
			if (temporary_directory == NULL) {
				err = -1;
				goto end;
			}
			
			directorys = GetTempPathA((DWORD) directorys, temporary_directory);
			
			if (directorys == 0) {
				err = -1;
				goto end;
			}
		#endif
		
		strip_path_sep(temporary_directory);
	#else
		size_t index = 0;
		
		const char* name = NULL;
		const char* directory = NULL;
		
		/*
		We should check first for the TEMP* and TMP* environment variables.
		*/
		for (index = 0; index < sizeof(ENV_TEMPORARY_DIRECTORY) / sizeof(*ENV_TEMPORARY_DIRECTORY); index++) {
			name = ENV_TEMPORARY_DIRECTORY[index];
			directory = getenv(name);
			
			if (directory == NULL) {
				continue;
			}
			
			if (directory_exists(directory) != 1) {
				continue;
			}
			
			temporary_directory = malloc(strlen(directory) + 1);
			
			if (temporary_directory == NULL) {
				err = -1;
				goto end;
			}
			
			strcpy(temporary_directory, directory);
			
			goto end;
		}
		
		/*
		The directories are checked in the following order:
		
		- /var/tmp
		- /usr/tmp
		- /tmp
		
		We prefer /var/tmp and /usr/tmp over /tmp because these locations are more
		suitable for storing large amounts of data.
		
		Since /tmp is commonly mounted as a ramdisk-based filesystem, its size is generally
		set to be 50% of the total amount of RAM. On embedded devices this size might be
		too low for us.
		*/
		for (index = 0; index < sizeof(TEMPORARY_DIRECTORIES) / sizeof(*TEMPORARY_DIRECTORIES); index++) {
			directory = TEMPORARY_DIRECTORIES[index];
			
			if (directory_exists(directory) != 1) {
				continue;
			}
			
			temporary_directory = malloc(strlen(directory) + 1);
			
			if (temporary_directory == NULL) {
				err = -1;
				goto end;
			}
			
			strcpy(temporary_directory, directory);
			
			goto end;
		}
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wdirectory);
	#endif
	
	if (err != 0) {
		free(temporary_directory);
		temporary_directory = NULL;
	}
	
	return temporary_directory;
	
}

char* get_home_dir(void) {
	/*
	Returns the home directory of the current user.
	*/
	
	int err = 0;
	
	char* home = NULL;
	
	#if defined(_WIN32)
		#if defined(_UNICODE)
			int directorys = 0;
			
			const wchar_t* const wdirectory = _wgetenv(WENV_APPDATA);
			
			if (wdirectory == NULL) {
				return NULL;
			}
			
			directorys = WideCharToMultiByte(CP_UTF8, 0, wdirectory, -1, NULL, 0, NULL, NULL);
			
			if (directorys == 0) {
				err = -1;
				goto end;
			}
			
			home = malloc((size_t) directorys);
			
			if (home == NULL) {
				err = -1;
				goto end;
			}
			
			if (WideCharToMultiByte(CP_UTF8, 0, wdirectory, -1, home, directorys, NULL, NULL) == 0) {
				err = -1;
				goto end;
			}
		#else
			const char* const directory = getenv(ENV_USERPROFILE);
			
			if (directory == NULL) {
				err = -1;
				goto end;
			}
			
			home = malloc(strlen(directory) + 1);
			
			if (home == NULL) {
				err = -1;
				goto end;
			}
			
			strcpy(home, directory);
		#endif
	#else
		const char* const directory = getenv(ENV_HOME);
		
		if (directory == NULL) {
			err = -1;
			goto end;
		}
		
		home = malloc(strlen(directory) + 1);
		
		if (home == NULL) {
			err = -1;
			goto end;
		}
		
		strcpy(home, directory);
	#endif
	
	end:;
	
	if (err != 0) {
		free(home);
		home = NULL;
	}
	
	return home;
	
}
