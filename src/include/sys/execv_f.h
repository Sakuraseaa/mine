#ifndef __EXECV_H__
#define __EXECV_H__

struct file *open_exec_file(char *path);
unsigned long do_execve(pt_regs_t *regs, char *name, char* argv[], char *envp[]);

#endif

