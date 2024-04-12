#ifndef _spinlock_h_
#define _spinlock_h_
#include "task.h"
#include "preempt.h"
// 本文件定义自旋锁，自旋锁用来防止多核侵略存储区
typedef struct
{
    __volatile__ unsigned long lock;
} spinlock_T;
void spin_init(spinlock_T *lock);
void spin_lock(spinlock_T *lock);
void spin_unlock(spinlock_T *lock);
// 尝试加锁 - 本系统并没有用到该函数
long spin_trylock(spinlock_T *lock);
#endif