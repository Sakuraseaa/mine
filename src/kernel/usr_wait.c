#include "wait.h"
#include "signal.h"
#include "lib.h"

int wait(int *status)
{
    return wait4((int)-1, status, 0);
}

int waitpid(int pid, int* status, int options)
{
    return wait4(pid, status, options);
}

extern void sig_restore(void);
sighadler_t signal(long signum, sighadler_t handler) {
    if(signum < 1 && signum > NSIG) {
        printf("ERROR!!!! invaild signal_value");
        return NULL;
    }

    return SIGNAL(signum, handler, sig_restore);
}

int kill(long pid, long signum) {
    if(signum < 1 && signum > NSIG) {
        printf("ERROR!!!! invaild signal_value");
        return NULL;
    }

    return KILL(pid, signum);
}

