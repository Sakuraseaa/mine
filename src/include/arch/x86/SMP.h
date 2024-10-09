

#ifndef __SMP_H__

#define __SMP_H__

// #include "spinlock.h"

extern unsigned char _APU_boot_start[];
extern unsigned char _APU_boot_end[];

// spinlock_t SMP_lock;

void SMP_init();
int SMP_cpu_id();
void Start_SMP();

#endif
