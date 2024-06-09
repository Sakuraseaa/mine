#include "wait.h"

int wait(int *status)
{
    return wait4((int)-1, status, 0);
}

int waitpid(int pid, int* status, int options)
{
    return wait4(pid, status, options);
}


