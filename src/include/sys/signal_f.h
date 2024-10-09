#ifndef _SIGNAL_F_H_
#define _SIGNAL_F_H_

// void (*signal(int _sig, void (*_func)(int)))(int);
int raise(int sig);
int kill(long pid, long signum);
sighadler_t sys_signal(long signum, sighadler_t hander,  void (*restorer)(void));
int sigaddset(sigset_t *mask, int signo);
int sigdelset(sigset_t *mask, int signo);
int sigemptyset(sigset_t *mask);
int sigfillset(sigset_t *mask);
int sigismember(sigset_t *mask, int signo); /* 1 - is, 0 - not, -1 error */
int sigpending(sigset_t *set);
int sigprocmask(int how, sigset_t *set, sigset_t *oldset);
int sigsuspend(sigset_t *sigmask);
int sigaction(int sig, struct sigaction *act, struct sigaction *oldact);



#endif // _SIGNAL_F_H_