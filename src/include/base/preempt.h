#ifndef __PREEMPT_H__
#define __PREEMPT_H__

// 自旋锁适合短时间枷锁，信号量适合长时间加锁
#define preempt_enable()          \
    do                            \
    {                             \
        current->preempt_count--; \
    } while (0)

#define preempt_disable()         \
    do                            \
    {                             \
        current->preempt_count++; \
    } while (0)

#endif
