#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__


struct schedule
{
	long running_task_count;	   // 当前队列内的进程的数量
	long CPU_exec_task_jiffies;	   // 每次进程调度时保存进程可执行的时间片数量
	task_t task_queue; // 队列头
};

extern struct schedule task_schedule;

void schedule();
void schedule_init();
void insert_task_queue(task_t *tsk);
task_t *get_next_task();
#endif
