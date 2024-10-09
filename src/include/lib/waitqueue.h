#ifndef _WAITQUEUE_H__
#define _WAITQUEUE_H__

// 等待队列 - 结构体
typedef struct WAIT_QUEUE
{
    struct List wait_list;
    struct task_struct *tsk; // 记录待挂起的PCB
} wait_queue_t;



#endif