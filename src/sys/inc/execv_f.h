#ifndef __EXECV_H__
#define __EXECV_H__

file_t *open_exec_file(str_t path);
u64_t do_execve(pt_regs_t *regs, str_t name, str_t argv[], str_t envp[]);

#endif