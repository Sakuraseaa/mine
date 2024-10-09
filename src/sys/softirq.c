#include "toolkit.h"
#include "syskit.h"

// 那种软中断触发了，就会置为softirq_status中对应的一位
void set_softirq_status(u64_t status)
{
    softirq_status |= status;
}

u64_t get_softirq_status()
{
    return softirq_status;
}

// register = 登记
void register_softirq(int nr, void (*action)(void *data), void *data)
{
    softirq_vector[nr].action = action;
    softirq_vector[nr].data = data;
}

void unregister_softirq(int nr)
{
    softirq_vector[nr].data = softirq_vector[nr].action = NULL;
}

void softirq_init()
{
    softirq_status = 0;
    memset(softirq_vector, 0, sizeof(struct softirq) * 64);
}

void do_softirq()
{
    sti();
    s32_t i;
    for (i = 0; i < 64 && softirq_status; i++)
    {
        if (softirq_status & (1UL << i))
        {
            softirq_vector[i].action(softirq_vector[i].data); // 执行中断处理程序
            softirq_status &= ~(1UL << i);                      // 取消该标记
        }
    }
    cli();
}