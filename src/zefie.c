#include "zefie.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "iso.h"
#include "md5.h"
#include "portable_endian.h"
#include "manamain.h"

#ifdef __WINDOWS__
off64_t seek64(int fd, off64_t offset, int origin) {
	return (off64_t) lseek(fd, offset, origin);
}
off64_t fseek64(FILE *fd, u64 offset, int origin) {
	return (off64_t) _fseeki64(fd, offset, origin);
}
#else
off64_t seek64(int fd, off64_t offset, int origin) {
	return lseek64(fd, offset, origin);
}
off64_t fseek64(FILE *fd, u64 offset, int origin) {
	return fseek(fd, offset, origin);
}
#endif


void Delete(char *file) {
	//unlink(file);
}

int strncmpi(const char *s1, const char *s2, size_t n) {
	return strncasecmp(s1, s2, n);
}

void print_load(char *format, ...)
{
	char * new_str ;
	if((new_str = malloc(strlen(format)+2)) != NULL){
		new_str[0] = '\0';   // ensures the memory is an empty string
		strcat(new_str,format);
		strcat(new_str,"\n");
		va_list args;
		va_start(args,format);
		printf(new_str, args);
		va_end(args);
	}
}

unsigned char *     bin_to_strhex(const unsigned char *bin, unsigned int binsz,
                                  unsigned char **result)
{
  unsigned char     hex_str[]= "0123456789abcdef";
  unsigned int      i;

  if (!(*result = (unsigned char *)malloc(binsz * 2 + 1)))
    return (NULL);

  (*result)[binsz * 2] = 0;

  if (!binsz)
    return (NULL);

  for (i = 0; i < binsz; i++)
    {
      (*result)[i * 2 + 0] = hex_str[(bin[i] >> 4) & 0x0F];
      (*result)[i * 2 + 1] = hex_str[(bin[i]     ) & 0x0F];
    }
  return (*result);
}