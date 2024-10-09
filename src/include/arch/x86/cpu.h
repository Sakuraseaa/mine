#ifndef __CPU_H__
#define __CPU_H__

#define NR_CPUS 8

/**
 * @brief Get the cpuid object
 *
 * @param Mop 用来选择cpuid的指令的功能
 * @param Sop
 * @param a 传出参数eax
 * @param b 传出参数ebx
 * @param c 传出参数ecx
 * @param d 传出参数edx
 */
static inline void get_cpuid(u32_t Mop, u32_t Sop, u32_t *a,
                             u32_t *b, u32_t *c, u32_t *d)
{
    __asm__ __volatile__("cpuid \n\t"
                         : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
                         : "0"(Mop), "2"(Sop));
}

void init_cpu(void);
#endif
