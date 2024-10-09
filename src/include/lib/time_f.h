#ifndef _TIME_F_H_
#define _TIME_F_H_

void timer_init();
int get_cmos_time(struct time *time);
void localtime(unsigned long stamp, struct time* tm);
unsigned long NOW();

#endif // _TIME_F_H_