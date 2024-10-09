#ifndef __SOFTIRQ_H__
#define __SOFTIRQ_H_

// 记录软中断的状态
u64_t softirq_status = 0;

#define TIMER_SIRQ (1UL << 0) // 只属于定时任务的软中断

// 该结构用于描述软中断的处理方法和参数
struct softirq
{
    void (*action)(void *data);
    void *data;
};


#endif