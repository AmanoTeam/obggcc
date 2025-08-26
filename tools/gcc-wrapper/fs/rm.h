#if !defined(FS_RM_H)
#define FS_RM_H

int remove_file(const char* const filename);
int remove_directory(const char* const directory);
int remove_empty_directory(const char* const directory);
int remove_directory_contents(const char* const directory);

#endif
