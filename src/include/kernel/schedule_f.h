#ifndef _SCHEDULE_F_H_
#define _SCHEDULE_F_H_

void schedule();
void schedule_init();
void insert_task_queue(task_t *tsk);
task_t *get_next_task();

#endif // _SCHEDULE_F_H_