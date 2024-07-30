#ifndef _spinlock_h_
#define _spinlock_h_
#include "preempt.h"
// 本文件定义自旋锁，自旋锁用来防止多核侵略存储区
typedef struct
{
    __volatile__ unsigned long lock;
} spinlock_T;

// 排队自旋锁 
typedef struct equity_spinlock_t{
    union {
        __volatile__ u32 slock;//真正的锁值变量
        struct {
            volatile u16 owner;
            volatile u16 next;
        };
    };
}fair_spinlock_t;

void spin_init(spinlock_T *lock);
void spin_lock(spinlock_T *lock);
void spin_unlock(spinlock_T *lock);
// 尝试加锁 - 本系统并没有用到该函数
long spin_trylock(spinlock_T *lock);
#endif