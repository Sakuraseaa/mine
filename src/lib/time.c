#include "time.h"
#include "lib.h"
#include "softirq.h"
#include "printk.h"
#include "HEPT.h"
#include "timer.h"
#include "memory.h"
#include "debug.h"
#include "types.h"

extern struct timer_list timer_list_head;
struct time Time;

#define COMS_READ(addr) ({ \
    io_out8(0x70, 0x80 | addr); \
    io_in8(0x71); })
// 把 1个字节的BCD码转换成二进制，1个字节表示2个10进制数
#define BCD_TO_BIN(val) ((val) = ((val) & 15) + ((val) >> 4) * 10)

#define MINUTE 60          // 每分钟的秒数
#define HOUR (60 * MINUTE) // 每小时的秒数
#define DAY (24 * HOUR)    // 每天的秒数
#define YEAR (365 * DAY)   // 每年的秒数，以 365 天算

// 每个月开始时的已经过去天数, 平年2月28天，润年2月29天
static int month[13] = {
    0, // 这里占位，没有 0 月，从 1 月开始
    0,
    (31),
    (31 + 29),
    (31 + 29 + 31),
    (31 + 29 + 31 + 30),
    (31 + 29 + 31 + 30 + 31),
    (31 + 29 + 31 + 30 + 31 + 30),
    (31 + 29 + 31 + 30 + 31 + 30 + 31),
    (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31),
    (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30),
    (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),
    (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30)
};

bool is_leap_year(int year)
{
    return ((year % 4 == 0) && (year % 100 != 0)) || ((year) % 400 == 0);
}

/**
 * @brief 计算year年总和是多少天
 * 
 * @param year 
 * @return unsigned long 
 */
unsigned long elapsed_leap_years(int year) {
    unsigned long res;
    res = year * 365 +  ((year + 1) / 4);
    
    // 另外一种算法
    // int i = 0;
    // for(res = 0; i < year; i++) {
    //     res += 365;
    //     if(is_leap_year(1970 + i))
    //         res++;
    // }

    return res;
}

unsigned long startup_time;
// 计算从 1970-1-1-0时起到开机当日经过的秒数，作为开机的时间
unsigned long kernel_mktime(struct time* tm) {
    unsigned long res = 0;
    int year;

    // 距今过去了多少年 - 已然遥远的理想之城。
    year = tm->year - 1970;

    // 注意到 1972年就是第一个闰年. 这样从1970年经过三年(70, 71, 72)就到了第一个闰年
    // 每个闰年多一天的秒数可以如此计算 (year - 3) / 4  +  1 = (year + 1) / 4. 
    // 整理后的式子有点轮回的味道，正所谓生与死轮回不止
    
    // res = 这些年经过的秒数 + 每个润年多一天的秒数 + 当年到当月时的秒数
    res = YEAR * year + DAY * ((year + 1) / 4);
    res += (month[tm->month] * DAY);
    
    //因为month数组中的2月规定为29天，所以此处对于不是闰年的年份要进行减一天秒数的操作
    if(tm->month > 1 && !is_leap_year(tm->year))
        res -= DAY;
    
    res += DAY * (tm->day - 1); // + 本月已经过去的天数   的秒数
    res += HOUR * tm->hour; // + 当天过去的小时数        的秒数
    res += MINUTE * tm->minute; // + 当小时过去的分钟数  的秒数
    res += tm->second;

    return res;
}

// 更新时间
void localtime(unsigned long stamp, struct time* tm) {
    // 确定秒
    tm->second = stamp % 60;
    
    // 确定分
    unsigned long remain = stamp / 60;
    tm->minute = remain % 60;
    
    // 确定时
    remain /= 60;
    tm->hour = remain % 24;
    
    // 计算1970年到现在已经过去了多少天
    unsigned long days = remain / 24; // 天,
    // 确定星期,1970-01-01 是周四
    tm->week_day = (days + 4) % 7;
    
    // 确认年, 这里产生误差显然需要 365 个闰年，不管了
    int year = days / 365;  // 1970 年到目前一共经过了多少年
    tm->year = year + 1970;


    int offset = 1;
    if (is_leap_year(tm->year))
        offset = 0;
    
    //修改days 今年已经过去了多少天（不包括当天）
    //公式 days = days - 1970年到本年之间的天数
    days -= elapsed_leap_years(year);
    
    tm->year_day = days % (366 - offset);

    int mon = 1;
    for(; mon < 13; mon++) {
        if((month[mon] - offset) > days)
            break;
    }
    // 确定月日
    tm->month = mon - 1;
    // 此处+1的原因是，在计算 hour second minite 中，
    // 已经把当天的那些时钟跳数 在整除运算中忽略掉了
    tm->day = (tm->year_day + 1) - (month[tm->month]);

    return;
}

// 读取cmos芯片获取当前时间
int get_cmos_time(struct time *time)
{
    do
    {
        time->second = COMS_READ(0x00);
        time->year = COMS_READ(0x09) + COMS_READ(0x32) * 0x100;
        time->month = COMS_READ(0x08);
        time->day = COMS_READ(0x07);
        time->hour = COMS_READ(0x04);
        time->minute = COMS_READ(0x02);
        time->week_day = COMS_READ(0x06);
    } while (time->second != COMS_READ(0x00));
    // 本次读取的时间，如果和当前的秒不符合。则重新读取cmos芯片
    io_out8(0x70, 0x00);

    time->year = BCD2BIN(time->year & 0xff) + BCD2BIN(time->year >> 8) * 100;
    time->month = BCD2BIN(time->month);
    time->day = BCD2BIN(time->day);
    time->hour = BCD2BIN(time->hour);
    time->minute = BCD2BIN(time->minute);
    time->second = BCD2BIN(time->second);
    time->week_day = BCD2BIN(time->week_day);// 为什么 week_day 读取的不正确
    return 1;
}

void do_timer(void *data)
{
    struct timer_list *tmp = container_of(list_next(&timer_list_head.list), struct timer_list, list);

    while ((!list_is_empty(&timer_list_head.list)) && (tmp->expire_jiffies <= jiffies))
    {
        tmp->func(tmp->data);
        tmp = container_of(list_next(&timer_list_head.list), struct timer_list, list);
        del_timer(tmp);
        //BUG:: 此处使用 DEBUGK 会报错,程序会 混乱跳转,重启
        // debugk(__BASE_FILE__, __LINE__, "(HPET:%ld):: A timing task is completed\n", jiffies);
    }
}
int shell_up = 0;
void test_timer(void *data)
{
    //BUG:: 此处使用 DEBUGK 会报错
    //DEBUGK("Why does debbuggin timed queues fail?\n");
    color_printk(BLUE, WHITE, "Why does debbuggin timed queues fail?  \n");
    shell_up = 1;
}

void timer_init()
{
    // 初始化定时任务队列
    struct timer_list *tmp = NULL;
    

    jiffies = 0;
    // +++++++++++++++实在不知道放哪里初始化了,就姑且放这里吧+++++++++++++
    startup_time = kernel_mktime(&Time);
    // ==============================================
    init_timer(&timer_list_head, NULL, NULL, -1UL);
    // 注册0号软中断
    register_softirq(0, &do_timer, NULL);

    // 给定时队列加入第一个任务
    tmp = (struct timer_list *)kmalloc(sizeof(struct timer_list), 0);
    init_timer(tmp, &test_timer, NULL, 40);
    add_timer(tmp);
}

