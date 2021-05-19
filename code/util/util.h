#ifndef __UTIL_H__
#define __UTIL_H__

#include <iostream>
#include <string>
#include <time.h>

using namespace std;

#define UTIL_BUFSIZE 65535

#define UTIL_MAX_FNAME 256
#define UTIL_MAX_PATH 260
#define UTIL_MAX_DRIVE 3
#define UTIL_MAX_DIR 256
#define UTIL_MAX_EXT 256

extern char* util_strdup(const char *s);
extern int util_stricmp(const char *dst, const char *src);
extern int util_strnicmp(const char *dst, const char *src, int count);

extern long util_strtol(char *str, int base);
extern int util_freset(const char *filename);
extern int util_fread(const char *filename, char * buffer, int buf_size);
extern int util_fwrite(const char *filename, const char *fmt, ...);
extern int util_remove(char * filename);
extern int util_string_to_time(const string &string_time,time_t &time_data);
extern int util_time_to_string(string &time_string,const time_t &time_data);
extern long util_getdiftime(time_t maxt,time_t mint);
extern int util_get_directory_info(const string &string_dir, string &string_result);

extern int util_ini_get_string(char *title, char *key, char *buff_default, char *buff_return, int buff_size, char *filename) ;
extern int util_ini_get_int(char *title,char *key, int value_default, char *filename);

extern void util_splitpath(const char *path, char *drive, char *dir, char *fname, char *ext);
#endif