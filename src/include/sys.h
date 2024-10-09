#ifndef _SYS_H__
#define _SYS_H__
#include "basetype.h"
#include "task.h"
#include "stat.h"

unsigned long no_system_call(void);
unsigned long sys_putstring(unsigned int FRcolor, char *string);
unsigned long sys_open(char *filename, int flags);
unsigned long sys_close(int fd);
unsigned long sys_read(int fd, void *buf, long count);
unsigned long sys_write(int fd, void *buf, long count);
unsigned long sys_lseek(int filds, long offset, int whence);
unsigned long sys_fork();
unsigned long sys_vfork();
unsigned long sys_brk(unsigned long brk);
unsigned long sys_reboot(unsigned long cmd, void *arg);
unsigned long sys_exit(int exit_code);
unsigned long sys_stat(char* filename, stat_t* statbuf);
char *sys_getcwd(char *buf, u64_t size);
unsigned long sys_chdir(char* filename);
unsigned long sys_execve();
unsigned long sys_wait4(unsigned long pid, int *status, int options,void *rusage);
u64_t sys_mkdir(char* filename);
u64_t sys_rmdir(char* filename);
u64_t sys_unlink(char* filename);
void exit_mm(task_t *tsk);


unsigned long sys_info(char order);
unsigned long sys_cleanScreen(void);
unsigned long sys_getNow(void);

#define SYSTEM_REBOOT (1UL << 0)
#define SYSTEM_POWEROFF (1UL << 1)

#endif