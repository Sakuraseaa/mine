#ifndef _SYS_H__
#define _SYS_H__

u64_t no_system_call(void);
u64_t sys_putstring(u32_t FRcolor, char *string);
u64_t sys_open(char *filename, int flags);
u64_t sys_close(int fd);
u64_t sys_read(int fd, void *buf, long count);
u64_t sys_write(int fd, void *buf, long count);
u64_t sys_lseek(int filds, long offset, int whence);
u64_t sys_fork();
u64_t sys_vfork();
u64_t sys_brk(u64_t brk);
u64_t sys_reboot(u64_t cmd, void *arg);
u64_t sys_exit(int exit_code);
u64_t sys_stat(char* filename, stat_t* statbuf);
char *sys_getcwd(char *buf, u64_t size);
u64_t sys_chdir(char* filename);
u64_t sys_execve();
u64_t sys_wait4(u64_t pid, int *status, int options,void *rusage);
u64_t sys_mkdir(char* filename);
u64_t sys_rmdir(char* filename);
u64_t sys_unlink(char* filename);
void exit_mm(struct task_struct *tsk);


u64_t sys_info(char order);
u64_t sys_cleanScreen(void);
u64_t sys_getNow(void);

#define SYSTEM_REBOOT (1UL << 0)
#define SYSTEM_POWEROFF (1UL << 1)

#endif