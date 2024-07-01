#ifndef _SEMAPHORE_H__
#define _SEMAPHORE_H__

#include "lib.h"
#include "task.h"
#include "atomic.h"
#include "semaphore.h"
#include "waitqueue.h"


// 信号量
typedef struct SEMAPHORE
{
    atomic_T conter;   // 原子量，记录信号量拥有的资源数量
    wait_queue_T wait; // 等待队列
} semaphore_T;

void semaphore_down(semaphore_T *semaphore);
void semaphore_up(semaphore_T *semaphore);
void semaphore_init(semaphore_T *semaphore, unsigned long count);
void wait_queue_init(wait_queue_T *wait_queue, struct task_struct *tsk);
#endif
