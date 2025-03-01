#include "toolkit.h"
#include "arch_x86kit.h"
#include "kernelkit.h"
schedule_t task_schedule_table;
extern void switch_to(task_t *prev, task_t *next);

task_t *runing_task()
{
	return task_schedule_table.is_running;
}

/* 传入参数是当前允许任务 */
task_t *get_next_task(task_t* curt)
{
	task_t *tsk = nullptr;
	// 就绪队列为空且当前进程不处于执行状态的时候，返回内核主线程, 这将会是一个空转的线程
	// 否则 就绪队列为空，继续执行当前进程
	if (list_is_empty(&task_schedule_table.task_queue.list))
	{
		return &init_task_union.task;
	}

	//  从就绪队列中得到一个进程
	tsk = container_of(list_next(&task_schedule_table.task_queue.list), task_t, list);
	list_del(&tsk->list);
    if (tsk == curt)
    {
        DEBUGK("it is the unforeseen that always happens\n");
    }
	task_schedule_table.running_task_count -= 1;
	return tsk;
}

// 加入一个任务到就绪队列, 该队列按照虚拟运行时间由小到大进行排序
#if 0
void insert_task_queue(struct task_struct *tsk)
{
	struct task_struct *tmp = NULL;

	if(tsk == init_task[SMP_cpu_id()])
		return ;

	tmp = container_of(list_next(&task_schedule_table.task_queue.list),struct task_struct,list);

	if(list_is_empty(&task_schedule_table.task_queue.list))
	{
	}
	else
	{
		while(tmp->vrun_time < tsk->vrun_time)
			tmp = container_of(list_next(&tmp->list),struct task_struct,list);
	}
	list_add_to_before(&tmp->list,&tsk->list);
	task_schedule_table.running_task_count += 1;
}
#endif
#if 1
void insert_task_queue(task_t *tsk)
{
	if (tsk == &init_task_union.task)
		return;
	
	task_t *tmp = nullptr;
	list_n_t* node = nullptr;
	list_h_t* head = &task_schedule_table.task_queue.list;
	

	if (list_is_empty(&task_schedule_table.task_queue.list))
	{
		list_add_to_behind(head, &tsk->list);
	}
	else
	{
		list_for_each(node, head)
		{
			tmp = container_of(node, task_t, list);
			if (tmp->vrun_time > tsk->vrun_time) // 把当前进程时间，插入到有序队列中第一个比[tsk]大的元素之前
			{
				list_add_to_before(&tmp->list, &tsk->list);
				break;
			}
		}
		if (node == head) // 队列中没有比新加入元素更大虚拟运行时间的元素
		{
			list_add_to_before(head, &tsk->list); //插入到头结点的前面
		}
	}
	task_schedule_table.running_task_count += 1;
}
#endif
static inline void supplement_process_time_slice()
{
	// 根据进程的优先级,填充进程允许队列中的处理器时间片
	switch (task_schedule_table.is_running->priority)
	{
		case 0:
		case 1:
			task_schedule_table.CPU_exec_task_jiffies = 4 / task_schedule_table.running_task_count;
			break;
		case 2:
		default:
			task_schedule_table.CPU_exec_task_jiffies = 4 / task_schedule_table.running_task_count * 3;
			break;
	}
}
void schedule()
{
    task_t* tsk = nullptr;
	
    cli();							  // 关闭外中断
	task_schedule_table.is_running->flags &= ~NEED_SCHEDULE; // 复位调度标志
	tsk = get_next_task(task_schedule_table.is_running);	// 从准备就绪队列中取出下一个待执行的进程

	// state != TASK_RUNNING,只要进程状态不对，
	// 则进程可被立即换下。证明进程阻塞后，可被立即换下，去执行其他进程
	if (task_schedule_table.is_running->vrun_time >= tsk->vrun_time 
		|| task_schedule_table.is_running->state != TASK_RUNNING)
	{ // 当前进程的虚拟运行时间大于待执行进程，执行切换

		// 只有当前进程是正在运行状态，才能进入就绪队列
		if (task_schedule_table.is_running->state == TASK_RUNNING)
            insert_task_queue(task_schedule_table.is_running);

		// 开启DEBUGK,会有不同的错误
		// DEBUGK("#schedule:%ld, pid:%ld(%ld) [switch to]-> pid:%ld(%ld)#\n", jiffies, current->pid, current->vrun_time, tsk->pid, tsk->vrun_time);
		
        // 按照进程优先级，给即将执行的进程计算PCB
		task_schedule_table.is_running = tsk; // 记录当前运行进程是tsk
        tsk->state = TASK_RUNNING;
		supplement_process_time_slice();
		switch_mm(current, tsk);
		switch_to(current, tsk); // 进程切换
	}
	else
	{ // 当前进程的虚拟运行时间小于待执行进程，不切换继续运行本进程
		insert_task_queue(tsk);
		supplement_process_time_slice();
	}

	sti();
}

void schedule_init()
{
	memset(&task_schedule_table, 0, sizeof(schedule_t));
	list_init(&task_schedule_table.task_queue.list);

	// 这是把内核主程序作为一个特殊进程囊括进入就绪队列的缘故。当内核主程序为操作系统创建出第一个进程后
	// 他将变为一个空闲进程，其作用是循环执行一个特殊的指令让系统保存低功耗待机，待到进程准备就绪队列
	// 为空时，处理器便会去执行空闲进程
	task_schedule_table.running_task_count = 1;

	task_schedule_table.CPU_exec_task_jiffies = 4;
	task_schedule_table.is_running = (task_t*)&init_task_union;
}
