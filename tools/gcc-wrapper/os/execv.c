#include <stdlib.h>

#if !defined(_WIN32)
	#include <unistd.h>
	#include <spawn.h>
#endif

#if defined(_WIN32)
	#include "posix_spawn.h"
#endif

#include "execv.h"

int execute_command(const char* const cmd, char** arg) {
	
	int err = 0;
	
	#if defined(_WIN32)
		err = spawn_command(cmd, arg);
	#else
		if (execv(cmd, arg) == -1) {
			err = -1;
			goto end;
		}
	#endif
	
	end:;
	
	#if defined(_WIN32) && defined(_UNICODE)
		free(wqarg);
		free(wcmd);
	#endif
	
	return err;
	
}
