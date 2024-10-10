#ifndef _SOFTIRQ_F_H_
#define _SOFTIRQ_F_H_

void register_softirq(s32_t nr, void (*action)(void *data), void *data);
void unregister_softirq(s32_t nr);
void set_softirq_status(u64_t status);
u64_t get_softirq_status();

void softirq_init();
struct softirq softirq_vector[64] = {0};
void do_softirq();

#endif // _SOFTIRQ_F_H_