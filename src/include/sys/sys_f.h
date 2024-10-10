#ifndef _SYS_H__
#define _SYS_H__

u64_t no_system_call(void);
u64_t sys_putstring(u32_t FRcolor, char *string);
u64_t sys_open(char *filename, s32_t flags);
u64_t sys_close(s32_t fd);
u64_t sys_read(s32_t fd, void *buf, s64_t count);
u64_t sys_write(s32_t fd, void *buf, s64_t count);
u64_t sys_lseek(s32_t filds, s64_t offset, s32_t whence);
u64_t sys_fork();
u64_t sys_vfork();
u64_t sys_brk(u64_t brk);
u64_t sys_reboot(u64_t cmd, void *arg);
u64_t sys_exit(s32_t exit_code);
u64_t sys_stat(str_t filename, stat_t* statbuf);
char *sys_getcwd(char *buf, u64_t size);
u64_t sys_chdir(str_t filename);
u64_t sys_execve();
u64_t sys_wait4(u64_t pid, s32_t *status, s32_t options,void *rusage);
u64_t sys_mkdir(str_t filename);
u64_t sys_rmdir(str_t filename);
u64_t sys_unlink(str_t filename);
void exit_mm(struct task_struct *tsk);


u64_t sys_info(char order);
u64_t sys_cleanScreen(void);
u64_t sys_getNow(void);

#define SYSTEM_REBOOT (1UL << 0)
#define SYSTEM_POWEROFF (1UL << 1)

#endif