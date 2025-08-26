#if !defined(WALKDIR_H)
#define WALKDIR_H

#include <stdlib.h>

#if defined(_WIN32)
	#include <windows.h>
#endif

#if !defined(_WIN32)
	#include <dirent.h>
#endif

enum WalkDirType {
	WALKDIR_ITEM_DIRECTORY,
	WALKDIR_ITEM_FILE,
	WALKDIR_ITEM_UNKNOWN
};

typedef enum WalkDirType walkdir_type_t;

struct WalkDirItem {
	walkdir_type_t type;
	size_t index;
	char* name;
};

typedef struct WalkDirItem walkdir_item_t;

struct WalkDir {
	walkdir_item_t item;
#if defined(_WIN32)
	HANDLE handle;
	WIN32_FIND_DATA data;
#else
	DIR* dir;
#endif
#if defined(__HAIKU__)
	char* directory;
#endif
};

typedef struct WalkDir walkdir_t;

int walkdir_init(walkdir_t* const walkdir, const char* const directory);
const walkdir_item_t* walkdir_next(walkdir_t* const walkdir);
void walkdir_free(walkdir_t* const walkdir);

#endif
