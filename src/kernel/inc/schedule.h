#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

typedef struct schedule
{
	s64_t running_task_count;	   // 当前队列内的进程的数量
	s64_t CPU_exec_task_jiffies;	   // 每次进程调度时保存进程可执行的时间片数量
	task_t task_queue; // 队列头
	task_t* is_running;
}schedule_t;

extern schedule_t task_schedule_table;
static inline task_t* get_nowtask()
{
	return task_schedule_table.is_running;
}
#endif
