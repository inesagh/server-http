#ifndef FILES
#define FILES

#include <stddef.h>
#include <sys/types.h>

ssize_t read_line(int, char*, size_t);
int read_from(int, void*, size_t);
int write_into(int, const void*, size_t);
int read_file_to_buffer(const char*, char**, size_t*);

#endif