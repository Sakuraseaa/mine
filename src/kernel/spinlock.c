#include "task.h"
#include "preempt.h"
#include "spinlock.h"
void spin_init(spinlock_T *lock)
{
    lock->lock = 1;
}
void spin_lock(spinlock_T *lock)
{
    preempt_disable();
    __asm__ __volatile__("1: \n\t"
                         "lock decq %0 \n\t" // --lock
                         "jns 3f       \n\t" // 结果为正则跳转
                         "2:           \n\t"
                         "pause        \n\t" // pause是一个空转指令，于nop指令相似，但pause更低耗
                         "cmpq $0, %0  \n\t"
                         "jle 2b       \n\t" // lock小于等于0，则跳转到2
                         "jmp 1b       \n\t"
                         "3:           \n\t"
                         : "=m"(lock->lock)::"memory");
}
// 解锁
void spin_unlock(spinlock_T *lock)
{
    __asm__ __volatile__("movq $1, %0\n\t"
                         : "=m"(lock->lock)::"memory");
    preempt_enable();
}

// 尝试加锁 - 本系统并没有用到该函数
long spin_trylock(spinlock_T *lock)
{
    unsigned long tmp_value = 0;
    preempt_disable();
    __asm__ __volatile__("xchgq	%0,	%1	\n\t"
                         : "=q"(tmp_value), "=m"(lock->lock)
                         : "0"(0)
                         : "memory");
    if (!tmp_value)
        preempt_enable();
    return tmp_value;
}