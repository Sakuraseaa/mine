#include "types.h"
#include "time.h"

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

static bool is_leap_year(int year)
{
    return ((year % 4 == 0) && (year % 100 != 0)) || ((year) % 400 == 0);
}

/**
 * @brief 计算year年总和是多少天
 * 
 * @param year 
 * @return unsigned long 
 */
static unsigned long elapsed_leap_years(int year)
{
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


// 更新时间
void localtime(unsigned long stamp, struct time* tm)
{

    
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