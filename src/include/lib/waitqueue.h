#ifndef _WAITQUEUE_H__
#define _WAITQUEUE_H__
#include "memory.h"
#include "lib.h"

// 等待队列 - 结构体
typedef struct WAIT_QUEUE
{
    struct List wait_list;
    struct task_struct *tsk; // 记录待挂起的PCB
} wait_queue_t;

bool wait_queue_is_empty(wait_queue_t* wait_queue);
void wait_queue_init(wait_queue_t *wait_queue, struct task_struct *tsk);
void sleep_on(wait_queue_t *wait_queue_head);
void interruptible_sleep_on(wait_queue_t *wait_queue_head);
void wakeup(wait_queue_t *wait_queue_head, long state);
void wakeup_pid(wait_queue_t *wait_queue_head, long state, long pid);

#endif