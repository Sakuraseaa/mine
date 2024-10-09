#ifndef _TIMER_F_H_
#define _TIMER_F_H_

void init_timer(struct timer_list *timer, void (*func)(void *data),
                void *data, unsigned long expire_jiffies);
void del_timer(struct timer_list *timer);
void add_timer(struct timer_list *timer);

#endif // _TIMER_F_H_