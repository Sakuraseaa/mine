#include "time.h"
#include "lib.h"
#include "softirq.h"
#include "printk.h"
#include "HEPT.h"
#include "timer.h"
#include "memory.h"
extern struct timer_list timer_list_head;

#define COMS_READ(addr) ({ \
    io_out8(0x70, 0x80 | addr); \
    io_in8(0x71); })

// 读取cmos芯片获取当前时间
int get_cmos_time(struct time *time)
{
    do
    {
        time->year = COMS_READ(0x09) + COMS_READ(0x32) * 0x100;
        time->month = COMS_READ(0x08);
        time->day = COMS_READ(0x07);
        time->hour = COMS_READ(0x04);
        time->minute = COMS_READ(0x02);
        time->second = COMS_READ(0x00);
    } while (time->second != COMS_READ(0x00));
    // 本次读取的时间，如果和当前的秒不符合。则重新读取cmos芯片
    io_out8(0x70, 0x00);
}

void do_timer(void *data)
{
    struct timer_list *tmp = container_of(list_next(&timer_list_head.list), struct timer_list, list);

    while ((!list_is_empty(&timer_list_head.list)) && (tmp->expire_jiffies <= jiffies))
    {
        tmp->func(tmp->data);
        tmp = container_of(list_next(&timer_list_head.list), struct timer_list, list);
        del_timer(tmp);
    }

    color_printk(RED, WHITE, "(HPET:%ld)", jiffies);
}
int shell_up = 0;
void test_timer(void *data)
{
    color_printk(BLUE, WHITE, "Why does debbuggin timed queues fail?");
    shell_up = 1;
}

void timer_init()
{
    // 初始化定时任务队列
    struct timer_list *tmp = NULL;
    jiffies = 0;
    init_timer(&timer_list_head, NULL, NULL, -1UL);
    // 注册0号软中断
    register_softirq(0, &do_timer, NULL);

    // 给定时队列加入第一个任务
    tmp = (struct timer_list *)kmalloc(sizeof(struct timer_list), 0);
    init_timer(tmp, &test_timer, NULL, 50);
    add_timer(tmp);
}