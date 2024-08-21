#if !defined(PATH_H)
#define PATH_H

char* basename(const char* const path);
size_t get_parent_directory(const char* const source, char* const destination, const size_t depth);

#endif
