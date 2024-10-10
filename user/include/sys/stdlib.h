#ifndef __STDLIB_H__
#define __STDLIB_H__


char* getcwd(char* buf, unsigned int size);
void * malloc(unsigned long size, int invalid);
void free(void * address);
unsigned long exit(int exit_code);
long getpid(void);

#endif
