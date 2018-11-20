#ifndef _ZEFIE_H
#define _ZEFIE_H

#define APP_NAME "PS3 CLI ISO Validator"
#define APP_VERSION "1.01"
#define APP_VERSION_DETAILS "by zefie, with many thanks to Zarh (ManaGunZ)"

#define _LARGEFILE64_SOURCE

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif


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

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int64_t s64;
typedef _off64_t off64_t;

u64 prog_bar1_value;
u64 total_size;
u16 nb_directory;
u16 nb_file;
u8 cancel;

#ifndef __cplusplus
	typedef unsigned char bool;
	static const bool false = 0;
	static const bool true = 1;
#endif

void print_load(char *format, ...);
unsigned char * bin_to_strhex(const unsigned char *bin, unsigned int binsz, unsigned char **result);
void stripStringISO9660(char *line);
off64_t seek64(int fd, off64_t offset, int origin);
off64_t fseek64(FILE *fd, u64 offset, int origin);
void Delete(char *file);
int strncmpi(const char *s1, const char *s2, size_t n);

#endif
