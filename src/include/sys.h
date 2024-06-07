#ifndef _SYS_H__
#define _SYS_H__

unsigned long no_system_call(void);
unsigned long sys_putstring(char *string);
unsigned long sys_open(char *filename, int flags);
unsigned long sys_close(int fd);
unsigned long sys_read(int fd, void *buf, long count);
unsigned long sys_write(int fd, void *buf, long count);
unsigned long sys_lseek(int filds, long offset, int whence);
unsigned long sys_fork();
unsigned long sys_vfork();
unsigned long sys_brk(unsigned long brk);
unsigned long sys_reboot(unsigned long cmd, void *arg);

#define SYSTEM_REBOOT (1UL << 0)
#define SYSTEM_POWEROFF (1UL << 1)

#endif