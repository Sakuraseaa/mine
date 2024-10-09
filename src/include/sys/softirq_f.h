#ifndef _SOFTIRQ_F_H_
#define _SOFTIRQ_F_H_

void register_softirq(int nr, void (*action)(void *data), void *data);
void unregister_softirq(int nr);
void set_softirq_status(unsigned long status);
unsigned long get_softirq_status();

void softirq_init();
struct softirq softirq_vector[64] = {0};
void do_softirq();

#endif // _SOFTIRQ_F_H_