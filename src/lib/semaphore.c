#include "lib.h"
#include "task.h"
#include "atomic.h"
#include "semaphore.h"
#include "schedule.h"
// 等待队列初始化
void wait_queue_init(wait_queue_T *wait_queue, struct task_struct *tsk)
{
    list_init(&wait_queue->wait_list);
    wait_queue->tsk = tsk;
}

// 信号量初始化
void semaphore_init(semaphore_T *semaphore, unsigned long count)
{
    atomic_set(&semaphore->conter, count);
    wait_queue_init(&semaphore->wait, NULL);
}

void __down(semaphore_T *semaphore)
{
    wait_queue_T wait; // 这个给等待队列变量此处使用的是内核栈内存， 这合理吗 ？？
    // 执行这个函数, 证明本进程即将切换，该wait的声明周期将一直延续到，本进程再次运行，
    // 退出down函数的时候，wait将被释放掉
    wait_queue_init(&wait, current);
    current->state = TASK_UNINTERRUPTIBLE;
    list_add_to_before(&semaphore->wait.wait_list, &wait.wait_list);
    schedule();
}

// 信号量加锁操作
void semaphore_down(semaphore_T *semaphore)
{
    if (atomic_read(&semaphore->conter) > 0)
        atomic_dec(&semaphore->conter);
    else
        __down(semaphore);
}

// 使得加入了阻塞队列的进程重新加入就绪队列
void __up(semaphore_T *semaphore)
{
    // container_of 真是神来之笔
    wait_queue_T *wait = container_of(list_next(&semaphore->wait.wait_list), wait_queue_T, wait_list);
    list_del(&wait->wait_list);
    wait->tsk->state = TASK_RUNNING;
    insert_task_queue(wait->tsk);

    // 抢占、调度本程序，执行刚才被唤醒的进程
    current->flags |= NEED_SCHEDULE;
}

// 信号量解锁操作
void semaphore_up(semaphore_T *semaphore)
{

    if (list_is_empty(&semaphore->wait.wait_list))
        atomic_inc(&semaphore->conter);
    else
        __up(semaphore);
}
