#ifndef _SEMAPHORE_H__
#define _SEMAPHORE_H__

#include "lib.h"
#include "task.h"
#include "atomic.h"
#include "waitqueue.h"
#include "semaphore.h"

// 信号量
typedef struct SEMAPHORE
{
    atomic_t conter;   // 原子量，记录信号量拥有的资源数量
    wait_queue_t wait; // 等待队列
} semaphore_t;

void semaphore_down(semaphore_t *semaphore);
void semaphore_up(semaphore_t *semaphore);
void semaphore_init(semaphore_t *semaphore, unsigned long count);
void wait_queue_init(wait_queue_t *wait_queue, struct task_struct *tsk);
#endif
