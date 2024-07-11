#ifndef __UNISTD_H__
#define __UNISTD_H__

#define __NR_putstring 1

#define __NR_open 2
#define __NR_close 3
#define __NR_read 4
#define __NR_write 5
#define __NR_lseek 6

#define __NR_fork 7
#define __NR_vfork 8

#define	__NR_fork	7
#define	__NR_vfork	8
#define	__NR_execve	9
#define	__NR_exit	10
#define	__NR_wait4	11

#define __NR_brk	12
#define __NR_reboot	13

#define __NR_chdir	14
#define __NR_getdents	15

#define __NR_SIGNAL	16
#define __NR_KILL	17

#define __NR_getpid 18
#define __NR_sleep 19

#define __NR_mkdir 20

#define __NR_getcwd 22
#define __NR_stat 23

#define __NR_tree 196

#endif