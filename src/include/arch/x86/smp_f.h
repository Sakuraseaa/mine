#ifndef __SMP_H__
#define __SMP_H__


extern u8_t _APU_boot_start[];
extern u8_t _APU_boot_end[];

// spinlock_t SMP_lock;

void SMP_init();
int SMP_cpu_id();
void Start_SMP();

#endif
