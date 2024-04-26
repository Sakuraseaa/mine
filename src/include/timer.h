#ifndef __TIMER_H__
#define __TIMER_H__

#include "lib.h"

struct timer_list
{
    struct List list;
    unsigned long expire_jiffies; // 失效日期
    void (*func)(void *data);     // 定时任务
    void *data;                   // 定时任务的参数
};
// 定时功能的队列头，所有定时任务均有序挂在于此队列中
extern struct timer_list timer_list_head;

void init_timer(struct timer_list *timer, void (*func)(void *data),
                void *data, unsigned long expire_jiffies);
void del_timer(struct timer_list *timer);
void add_timer(struct timer_list *timer);
#endif