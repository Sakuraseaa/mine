#include "wait.h"
#include "unistd.h"
int wait(int *status)
{
    return wait4((int)-1, status, 0);
}

/**
 * @brief 等待进程死亡
 * 
 * @param pid 等待的进程号
 * @param status 子进程退出号
 * @param options 
 * @return int 
 */
int waitpid(int pid, int* status, int options)
{
    return wait4(pid, status, options);
}
