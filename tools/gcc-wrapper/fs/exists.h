#if !defined(FS_EXISTS_H)
#define FS_EXISTS_H

int directory_exists(const char* const directory);
int file_exists(const char* const filename);
int symlink_exists(const char* const path);

#endif
