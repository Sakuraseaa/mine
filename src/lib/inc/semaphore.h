#ifndef _SEMAPHORE_H__
#define _SEMAPHORE_H__
// 信号量
typedef struct SEMAPHORE
{
    atomic_t conter;   // 原子量，记录信号量拥有的资源数量
    wait_queue_t wait; // 等待队列
} semaphore_t;

#endif
