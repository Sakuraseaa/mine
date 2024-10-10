#ifndef __TIMER_H__
#define __TIMER_H__

typedef struct timer_list
{
    struct List list;
    u64_t expire_jiffies; // 失效日期
    void (*func)(void *data);     // 定时任务
    void *data;                   // 定时任务的参数
}timer_list_t;
// 定时功能的队列头，所有定时任务均有序挂在于此队列中
extern struct timer_list timer_list_head;


#endif