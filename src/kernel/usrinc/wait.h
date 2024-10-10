#ifndef __WAIT_H__

#define __WAIT_H__

int wait(int * status);
int waitpid(int pid, int *status, int options);
#endif