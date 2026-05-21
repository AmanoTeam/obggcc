#if !defined(OS_ENV_H)
#define OS_ENV_H

char* get_config_dir(void);
char* get_temp_dir(void);
char* get_home_dir(void);
char* find_exe(const char* const name);

#endif
