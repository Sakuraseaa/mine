#ifndef __EXECV_H__
#define __EXECV_H__

#include "ptrace.h"

struct file *open_exec_file(char *path);
unsigned long do_execve(struct pt_regs *regs, char *name, char* argv[], char *envp[]);

#endif

