# 6 "syscalls.c"
extern u64_t no_system_call(void);
# 1 "../include/syscalls.h" 1
extern u64_t sys_putstring(void);

extern u64_t sys_open(void);
extern u64_t sys_close(void);
extern u64_t sys_read(void);
extern u64_t sys_write(void);
extern u64_t sys_lseek(void);
extern u64_t sys_fork(void);
extern u64_t sys_vfork(void);
extern u64_t sys_execve(void);
extern u64_t sys_exit(void);
extern u64_t sys_wait4(void);


extern u64_t sys_brk(void);
extern u64_t sys_reboot(void);
extern u64_t sys_chdir(void);
extern u64_t sys_getdents(void);
extern u64_t sys_signal(void);
extern u64_t sys_kill(void);
extern u64_t sys_getpid(void);
extern u64_t sys_sleep(void);
extern u64_t sys_mkdir(void);
# 8 "syscalls.c" 2





typedef u64_t (*system_call_t)(void);

system_call_t system_call_table[128] = {
    [0 ... 128 - 1] = no_system_call,
# 1 "../include/syscalls.h" 1
[__NR_putstring] = sys_putstring,

[__NR_open] = sys_open,
[__NR_close] = sys_close,
[__NR_read] = sys_read,
[__NR_write] = sys_write,
[__NR_lseek] = sys_lseek,
[__NR_fork] = sys_fork,
[__NR_vfork] = sys_vfork,
[__NR_execve] = sys_execve,
[__NR_exit] = sys_exit,
[__NR_wait4] = sys_wait4,


[__NR_brk] = sys_brk,
[__NR_reboot] = sys_reboot,
[__NR_chdir] = sys_chdir,
[__NR_getdents] = sys_getdents,
[__NR_SIGNAL] = sys_signal,
[__NR_KILL] = sys_kill,
[__NR_getpid] = sys_getpid,
[__NR_sleep] = sys_sleep,
[__NR_mkdir] = sys_mkdir,
# 18 "syscalls.c" 2
};
