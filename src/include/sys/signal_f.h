#ifndef _SIGNAL_F_H_
#define _SIGNAL_F_H_

// void (*signal(s32_t _sig, void (*_func)(s32_t)))(s32_t);
s32_t raise(s32_t sig);
s32_t kill(s64_t pid, s64_t signum);
sighadler_t sys_signal(s64_t signum, sighadler_t hander,  void (*restorer)(void));
s32_t sigaddset(sigset_t *mask, s32_t signo);
s32_t sigdelset(sigset_t *mask, s32_t signo);
s32_t sigemptyset(sigset_t *mask);
s32_t sigfillset(sigset_t *mask);
s32_t sigismember(sigset_t *mask, s32_t signo); /* 1 - is, 0 - not, -1 error */
s32_t sigpending(sigset_t *set);
s32_t sigprocmask(s32_t how, sigset_t *set, sigset_t *oldset);
s32_t sigsuspend(sigset_t *sigmask);
s32_t sigaction(s32_t sig, struct sigaction *act, struct sigaction *oldact);



#endif // _SIGNAL_F_H_