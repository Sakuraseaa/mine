#ifndef _HEPT_F_H_
#define _HEPT_F_H_

void HEPT_init();
void mtime_sleep(unsigned int m_seconds);
void time_sleep(unsigned int seconds);
void intr_timer_handler(u64_t nr, u64_t parameter, pt_regs_t *regs);

#endif // _HEPT_F_H_