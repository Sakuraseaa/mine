#include "task.h"
#include "memory.h"
#include "waitqueue.h"
#include "lib.h"
#include "schedule.h"

/**
 * @brief 初始化等待队列
 *
 * @param wait_queue
 * @param tsk
 */
void wait_queue_init(wait_queue_t *wait_queue, task_t *tsk)
{
    wait_queue->tsk = tsk;
    list_init(&wait_queue->wait_list);
}

/**
 * @brief 睡眠当前线程
 *
 * @param wait_queue_head
 */
void sleep_on(wait_queue_t *wait_queue_head)
{   
    if(wait_queue_head == NULL)
        return;

    // a. 创建等待结点, 将node和当前进程相关联
    wait_queue_t node;
    wait_queue_init(&node, current);
    // b. 修改当前进程运行状态为不可中断状态
    current->state = TASK_UNINTERRUPTIBLE;
    // c. 添加等待结点到等待就绪队列上
    list_add_to_before(&wait_queue_head->wait_list, &node.wait_list);
    // d. 调度其他进程运行
    schedule();
}

/**
 * @brief 睡眠当前线程
 *
 * @param wait_queue_head
 */
void interruptible_sleep_on(wait_queue_t *wait_queue_head)
{
    // a. 创建等待结点, 将node和当前进程相关联
    wait_queue_t node;
    wait_queue_init(&node, current);
    // b. 修改当前进程运行状态为可中断状态
    current->state = TASK_INTERRUPTIBLE;
    // c. 添加等待结点到等待就绪队列上
    list_add_to_before(&wait_queue_head->wait_list, &node.wait_list);
    // d. 调度其他进程运行
    schedule();
}

/**
 * @brief 唤醒线程
 *
 * @param wait_queue_head 等待队列头结点
 * @param state 需要被唤醒的线程，现在处于什么状态
 */
void wakeup(wait_queue_t *wait_queue_head, long state)
{
    wait_queue_t *node = NULL;
    if (list_is_empty(&wait_queue_head->wait_list))
        return;

    node = container_of(list_next(&wait_queue_head->wait_list), wait_queue_t, wait_list);

    // 需要唤醒的进程和传入参数state相等 ，才进行唤醒
    if (node->tsk->state & state)
    {
        list_del(&node->wait_list);
        wakeup_process(node->tsk);
    }
}

/**
 * @brief 唤醒线程
 */
void wakeup_pid(wait_queue_t *wait_queue_head, long state, long pid)
{
    wait_queue_t *node = wait_queue_head;
    if (list_is_empty(&wait_queue_head->wait_list))
        return;

    // 在等待队列上寻找进程号是pid的进程
    do {
    
        node = container_of(list_next(&node->wait_list), wait_queue_t, wait_list);

    } while(node->tsk->pid != pid);

    // 需要唤醒的进程和传入参数state相等 ，才进行唤醒
    if ((node->tsk->state & state) && (node->tsk->pid == pid))
    {
        list_del(&node->wait_list);
        wakeup_process(node->tsk);
    }
}



bool wait_queue_is_empty(wait_queue_t* wait_queue) {
    return list_is_empty(&wait_queue->wait_list);
}
