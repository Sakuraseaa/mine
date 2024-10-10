#ifndef __TIME_H__USR_
#define __TIME_H__USR_

struct time
{
    s32_t second; // 00
    s32_t minute; // 02
    s32_t hour;   // 04
    s32_t day;    // 07
    s32_t month;  // 08
    s32_t year;   // 09 + 32
    s32_t week_day;  // 1 星期中的某天 [0，6] (星期天 =0)
    s32_t year_day;  // 1 年中的某天 [0，365]
};

typedef struct time tm;
#define BCD2BIN(value) (((value)&0xf) + ((value) >> 4) * 10)

#endif