#ifndef __TIME_H__USR_
#define __TIME_H__USR_

typedef struct time
{
    int second; // 00
    int minute; // 02
    int hour;   // 04
    int day;    // 07
    int month;  // 08
    int year;   // 09 + 32
    int week_day;  // 1 星期中的某天 [0，6] (星期天 =0)
    int year_day;  // 1 年中的某天 [0，365]
}tm;

// 更新时间
void localtime(unsigned long stamp, struct time* tm);

#endif