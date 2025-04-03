#ifndef _TIME_F_H_
#define _TIME_F_H_

void timer_init();
s32_t get_cmos_time(struct time *time);
void localtime(u64_t stamp, struct time* tm);
u64_t NOW();
bool_t get_time(struct time *tm);

#endif // _TIME_F_H_