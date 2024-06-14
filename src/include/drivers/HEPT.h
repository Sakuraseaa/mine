#ifndef __DEVICE_TIME_H
#define __DEVICE_TIME_H

#include "ptrace.h"
extern unsigned long volatile jiffies;
void HEPT_init();
void mtime_sleep(unsigned int m_seconds);
void time_sleep(unsigned int seconds);
void intr_timer_handler(unsigned long nr, unsigned long parameter, struct pt_regs *regs);
#endif