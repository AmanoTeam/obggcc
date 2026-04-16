#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "walkdir.h"
#include "fs/getexec.h"
#include "fstream.h"
#include "fs/sep.h"

static const char* const LIBRARIES[] = {
	"libatomic-1.dll",
	"libquadmath-0.dll",
	"libgcc_s_seh-1.dll",
	"libstdc++-6.dll",
	"libgomp-1.dll",
	"libwinpthread-1.dll"
};

static const char* const TARGETS[] = {
	"x86_64-w64-mingw32-msvcrt",
	"x86_64-w64-mingw32-ucrt",
	"i686-w64-mingw32-msvcrt",
	"i686-w64-mingw32-ucrt"
};

static const char DLL_FILE_EXT[] = ".dll";

static const char NZ_DIRECTORY[] = "lib" PATHSEP_M "nouzen" PATHSEP_M "sysroot" PATHSEP_M "bin";

int main() {
	
	char* directory = get_app_directory();
	size_t index = 0;
	size_t subindex = 0;
	
	size_t size = 0;
	size_t buffer_size = 0;
	
	const char* name;
	char* libdir = NULL;
	char* library = NULL;
	char* match = NULL;
	
	walkdir_t walkdir = {0};
	const walkdir_item_t* item = NULL;
	
	fstream_t* stream = NULL;
	
	long int file_size = 0;
	
	char* buffer = NULL;
	
	for (index = 0; index < sizeof(TARGETS) / sizeof(*TARGETS); index++) {
		name = TARGETS[index];
		
		libdir = malloc(strlen(directory) + strlen(PATHSEP_S) + strlen(name) + strlen(PATHSEP_S) + strlen(NZ_DIRECTORY) + 1);
		
		strcpy(libdir, directory);
		strcat(libdir, PATHSEP_S);
		strcat(libdir, name);
		strcat(libdir, PATHSEP_S);
		strcat(libdir, NZ_DIRECTORY);
		
		if (walkdir_init(&walkdir, libdir) == -1) {
			continue;
		}
		
		while ((item = walkdir_next(&walkdir)) != NULL) {
			if (strcmp(item->name, ".") == 0 || strcmp(item->name, "..") == 0) {
				continue;
			}
			
			if (item->type != WALKDIR_ITEM_FILE) {
				continue;
			}
			
			name = strchr(item->name, '\0') - strlen(DLL_FILE_EXT);
			
			if (strcmp(name, DLL_FILE_EXT) != 0) {
				continue;
			}
			
			library = malloc(strlen(libdir) + strlen(PATHSEP_S) + strlen(item->name) + 1);
			
			strcpy(library, libdir);
			strcat(library, PATHSEP_S);
			strcat(library, item->name);
			
			printf("- checking %s\n", library);
			
			stream = fstream_open(library, FSTREAM_READ);
			file_size = fsream_size(stream);
			
			buffer_size = (size_t) file_size;
			buffer = malloc(buffer_size);
			
			fstream_read(stream, buffer, buffer_size);
			fstream_close(stream);
			
			for (subindex = 0; subindex < sizeof(LIBRARIES) / sizeof(*LIBRARIES); subindex++) {
				name = LIBRARIES[subindex];
				size = strlen(name);
				
				match = memmem(buffer, buffer_size, name, size);
				
				if (match == NULL) {
					continue;
				}
				
				match = strstr(match, "-");
				
				size = strlen(match);
				memset(match, 0, size);
				
				printf("- wrote %zu bytes of data at pos %zu\n", size, (size_t) (match - buffer));
				
				strcpy(match, DLL_FILE_EXT);
			}
			
			stream = fstream_open(library, FSTREAM_WRITE);
			fstream_write(stream, buffer, buffer_size);
			fstream_close(stream);
		}
	}
	
	return EXIT_SUCCESS;
	
}
