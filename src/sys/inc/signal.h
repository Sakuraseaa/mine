#ifndef _SIGNAL_H
#define _SIGNAL_H

typedef u32_t sigset_t; /* 32 bits */

#define _NSIG 32
#define NSIG _NSIG

#define SIGHUP 1
#define SIGINT 2 // signal interrupt --来自键盘的中断
#define SIGQUIT 3
#define SIGILL 4
#define SIGTRAP 5
#define SIGABRT 6
#define SIGIOT 6
#define SIGUNUSED 7
#define SIGFPE 8
#define SIGKILL 9
#define SIGUSR1 10
#define SIGSEGV 11
#define SIGUSR2 12
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15
#define SIGSTKFLT 16
#define SIGCHLD 17 // Child 子进程停止或被终止
#define SIGCONT 18
#define SIGSTOP 19
#define SIGTSTP 20
#define SIGTTIN 21
#define SIGTTOU 22

/* Ok, I haven't implemented sigactions, but trying to keep headers POSIX */
#define SA_NOCLDSTOP 1
#define SA_NOMASK 0x40000000
#define SA_ONESHOT 0x80000000

#define SIG_BLOCK 0	  /* for blocking signals */
#define SIG_UNBLOCK 1 /* for unblocking signals */
#define SIG_SETMASK 2 /* for setting the signal mask */

#define SIG_DFL ((void (*)(s64_t))0) /* default signal handling 默认信号处理程序*/
#define SIG_IGN ((void (*)(s64_t))1) /* ignore signal 忽略信号处理程序*/

typedef void(*sighadler_t)(s64_t);

typedef struct sigaction
{
	void (*sa_handler)(s64_t);   // 某信号指定要采取的行动，可用SIG_DFL, SIG_IGN来填写
	sigset_t sa_mask;		   // 信号屏蔽码，在信号执行时会阻塞对这些信号的处理
	s32_t sa_flags;			   // 指定代表信号处理过程中的信号集
	void (*sa_restorer)(void); // 恢复函数指针，由Libc提供，用于清理用户态堆栈
}sigaction_t;

#endif /* _SIGNAL_H */