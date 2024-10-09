#ifndef __UNISTD_H__

#define __UNISTD_H__

#include "basetype.h"

int open(const char *path, int oflag);
int close(int fildes);
long read(int fildes, void *buf, long nbyte);
long write(int fildes, const void *buf, long nbyte);
long lseek(int fildes, long offset, int whence);

int getdents(int fildes, void* buf, long nbyte);
int fork(void);
int vfork(void);
int execve(const char* path, char* const argv[], char* const envp[]);
int wait4(int pid, int* status, int options);

int unlink(const char* path);
int mkdir(const char* path);
int rmdir(const char* path);

int info(const char order);
int cleanScreen(void);
u64 getNow(void);

#endif
