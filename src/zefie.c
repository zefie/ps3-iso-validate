#include "zefie.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "iso.h"
#include "md5.h"

off64_t seek64(int fd, off64_t offset, int origin) {
	return lseek64(fd, offset, origin);
}

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
