#include "signal.h"
#include "stddef.h"
#include "stdio.h"
extern sighadler_t SIGNAL(long signum, sighadler_t  hander, void (*restorer)(void));
extern int SYSKILL(long pid, int signum);
extern void sig_restore(void);

sighadler_t signal(long signum, sighadler_t handler) {
    if(signum < 1 && signum > NSIG) {
        printf("ERROR!!!! invaild signal_value");
        return NULL;
    }

    return SIGNAL(signum, handler, sig_restore);
}

extern int KILL(long pid, long signum);
int kill(long pid, long signum) {
    if(signum < 1 && signum > NSIG) {
        printf("ERROR!!!! invaild signal_value");
        return NULL;
    }

    return KILL(pid, signum);
}


