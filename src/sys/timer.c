#include "toolkit.h"
#include "mmkit.h"

struct timer_list timer_list_head;

// 定时队列元素初始化, 定时器结构体需要动态申请创建
void init_timer(struct timer_list *timer, void (*func)(void *data),
                void *data, u64_t expire_jiffies)
{
    list_init(&timer->list);
    timer->data = data;
    timer->expire_jiffies = expire_jiffies;
    timer->func = func;
    timer->tl_flag = tts_ready;
}

// 定时队列任务添加
void add_timer(struct timer_list *timer)
{
    // 第一个定时任务指针
    struct timer_list *tmp = container_of(list_next(&timer_list_head.list), struct timer_list, list);
    if (list_is_empty(&timer_list_head.list))
    {
        // 队列空
    }
    else
    { // 队列非空
        while (tmp->expire_jiffies < timer->expire_jiffies)
            tmp = container_of(list_next(&tmp->list), struct timer_list, list);
    }
    // 失效时间由小到大排序
    list_add_to_before(&tmp->list, &timer->list);
}

// 定时任务删除
void del_timer(struct timer_list *timer)
{
    list_del(&timer->list);
    kdelete(timer, sizeof(timer_list_t));
}