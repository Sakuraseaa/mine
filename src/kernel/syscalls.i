

# 6 "syscalls.c"
extern unsigned long no_system_call(void);
# 1 "../include/syscalls.h" 1
extern unsigned long sys_putstring(void);

extern unsigned long sys_open(void);
extern unsigned long sys_close(void);
extern unsigned long sys_read(void);
extern unsigned long sys_write(void);
extern unsigned long sys_lseek(void);

extern unsigned long sys_fork(void);
extern unsigned long sys_vfork(void);

extern unsigned long sys_brk(void);
# 8 "syscalls.c" 2

typedef unsigned long (*system_call_t)(void);

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

    [__NR_brk] = sys_brk,
# 18 "syscalls.c" 2
};
