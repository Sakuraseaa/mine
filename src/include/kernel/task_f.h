#ifndef _TASK_F_H_
#define _TASK_F_H_
/* fork exit 不应该出现在这里 */
u64_t do_exit(u64_t exit_code);
u64_t do_fork(pt_regs_t *regs, u64_t clone_flags, u64_t stack_start, u64_t stack_size);
void task_init();
void switch_mm(task_t *prev, task_t *next);
void wakeup_process(task_t *tsk);
void exit_files(task_t *tsk);
void __switch_to(task_t *prev, task_t *next);

#endif // _TASK_F_H_