#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include "task.h"

struct schedule
{
	long running_task_count;	   // 当前队列内的进程的数量
	long CPU_exec_task_jiffies;	   // 每次进程调度时保存进程可执行的时间片数量
	struct task_struct task_queue; // 队列头
};

extern struct schedule task_schedule;

void schedule();
void schedule_init();
void insert_task_queue(struct task_struct *tsk);
struct task_struct *get_next_task();
#endif
