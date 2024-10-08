SYSCALL_COMMON(__NR_putstring, sys_putstring)

SYSCALL_COMMON(__NR_open, sys_open)
SYSCALL_COMMON(__NR_close, sys_close)
SYSCALL_COMMON(__NR_read, sys_read)
SYSCALL_COMMON(__NR_write, sys_write)
SYSCALL_COMMON(__NR_lseek, sys_lseek)
SYSCALL_COMMON(__NR_fork, sys_fork)
SYSCALL_COMMON(__NR_vfork, sys_vfork)
SYSCALL_COMMON(__NR_execve, sys_execve)
SYSCALL_COMMON(__NR_exit, sys_exit)
SYSCALL_COMMON(__NR_wait4, sys_wait4)


SYSCALL_COMMON(__NR_brk, sys_brk)
SYSCALL_COMMON(__NR_reboot, sys_reboot)
SYSCALL_COMMON(__NR_chdir, sys_chdir)
SYSCALL_COMMON(__NR_getdents, sys_getdents)
SYSCALL_COMMON(__NR_SIGNAL, sys_signal)
SYSCALL_COMMON(__NR_KILL, sys_kill)
SYSCALL_COMMON(__NR_getpid,sys_getpid)
SYSCALL_COMMON(__NR_sleep,sys_sleep)
SYSCALL_COMMON(__NR_mkdir, sys_mkdir)
SYSCALL_COMMON(__NR_rmdir, sys_rmdir)

SYSCALL_COMMON(__NR_getcwd, sys_getcwd)
SYSCALL_COMMON(__NR_stat, sys_stat)

SYSCALL_COMMON(__NR_unlink, sys_unlink)

SYSCALL_COMMON(__NR_info, sys_info)
SYSCALL_COMMON(__NR_cleanScreen, sys_cleanScreen)
SYSCALL_COMMON(__NR_getNow, sys_getNow)