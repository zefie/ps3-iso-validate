#ifndef _ZEFIE_H
#define _ZEFIE_H

#define _LARGEFILE64_SOURCE 1
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/stat.h>
#include <dirent.h>

//#define snprintf _snprintf
//#define vsnprintf _vsnprintf
//#define strcasecmp _stricmp
//#define strncasecmp _strnicmp

#define FAILED 0
#define SUCCESS 1
#define NO 0
#define YES 1


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int64_t s64;

u64 prog_bar1_value;
u64 total_size;
u16 nb_directory;
u16 nb_file;

#ifndef __cplusplus
	typedef unsigned char bool;
	static const bool false = 0;
	static const bool true = 1;
#endif

void print_load(char *format, ...);
off_t seek64(int fd, off_t offset, int origin);
void Delete(char *file);
int strncmpi(const char *s1, const char *s2, size_t n);

#endif
