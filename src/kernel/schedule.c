#include "schedule.h"
#include "task.h"
#include "lib.h"
#include "printk.h"
#include "timer.h"
#include "HEPT.h"
struct schedule task_schedule;
extern void switch_to(struct task_struct *prev, struct task_struct *next);
struct task_struct *get_next_task()
{
	struct task_struct *tsk = NULL;
	// 就绪队列为空，返回内核主线程, 这将会是一个空转的线程
	if (list_is_empty(&task_schedule.task_queue.list))
		return &init_task_union.task;

	//  从就绪队列中得到一个进程
	tsk = container_of(list_next(&task_schedule.task_queue.list), struct task_struct, list);
	list_del(&tsk->list);
	task_schedule.running_task_count -= 1;
	return tsk;
}

// 加入一个任务到就绪队列, 该队列按照虚拟运行时间由小到大进行排序
void insert_task_queue(struct task_struct *tsk)
{
	struct task_struct *tmp = container_of(list_next(&task_schedule.task_queue.list), struct task_struct, list);
	if (tsk == &init_task_union.task)
		return;

	if (list_is_empty(&task_schedule.task_queue.list))
	{
	}
	else
	{
		while (tmp->vrun_time < tsk->vrun_time)
			tmp = container_of(list_next(&tmp->list), struct task_struct, list);
	}
	list_add_to_before(&tmp->list, &tsk->list);
	task_schedule.running_task_count += 1;
}


struct task_struct *tsk = NULL;
// 调度器
void schedule()
{
	//struct task_struct *tsk = NULL;
	cli();							  // 关闭外中断·
	current->flags &= ~NEED_SCHEDULE; // 复位调度标志
	tsk = get_next_task();			  // 从准备就绪队列中取出下一个待执行的进程

	// state != TASK_RUNNING,只要进程状态不对，
	// 则进程可被立即换下。证明进程阻塞后，可被立即换下，去执行其他进程
	if (current->vrun_time >= tsk->vrun_time || current->state != TASK_RUNNING)
	{ // 当前进程的虚拟运行时间大于待执行进程，执行切换

		// 只有当前进程是正在运行状态，才能进入就绪队列
		if (current->state == TASK_RUNNING)
			insert_task_queue(current);

		// 按照进程优先级，给即将执行的进程计算PCB
		if (!task_schedule.CPU_exec_task_jiffies)
			switch (tsk->priority)
			{
			case 0:
			case 1:
				task_schedule.CPU_exec_task_jiffies = 4 / task_schedule.running_task_count;
				break;
			case 2:
			default:
				task_schedule.CPU_exec_task_jiffies = 4 / task_schedule.running_task_count * 3;
				break;
			}
		// color_printk(YELLOW, BLACK, "#schedule:%ld, pid:%ld(%ld)=>>pid:%ld(%ld)#\n",
		// 			 jiffies, current->pid, current->vrun_time, tsk->pid, tsk->vrun_time);
		switch_mm(current, tsk);
		switch_to(current, tsk); // 进程切换
	}
	else
	{ // 当前进程的虚拟运行时间小于待执行进程，不切换继续运行本进程
		insert_task_queue(tsk);
		// 根据进程的优先级填充处理器时间片
		if (!task_schedule.CPU_exec_task_jiffies)
			switch (current->priority)
			{
			case 0:
			case 1:
				task_schedule.CPU_exec_task_jiffies = 4 / task_schedule.running_task_count;
				break;
			case 2:
			default:
				task_schedule.CPU_exec_task_jiffies = 4 / task_schedule.running_task_count * 3;
				break;
			}
	}
	sti();
}

void schedule_init()
{
	memset(&task_schedule, 0, sizeof(struct schedule));
	list_init(&task_schedule.task_queue.list);

	// 给内核主程序赋值最大值, 主程序无法被动的 被schedule调度
	task_schedule.task_queue.vrun_time = 0x7fffffffffffffff;

	// 这是把内核主程序作为一个特殊进程囊括进入就绪队列的缘故。当内核主程序为操作系统创建出第一个进程后
	// 他将变为一个空闲进程，其作用是循环执行一个特殊的指令让系统保存低功耗待机，待到进程准备就绪队列
	// 为空时，处理器便会去执行空闲进程
	task_schedule.running_task_count = 1;

	task_schedule.CPU_exec_task_jiffies = 4;
}
