#ifndef __GATE_H__
#define __GATE_H__

struct desc_struct
{
	unsigned char x[8];
};

struct gate_struct
{
	unsigned char x[16];
};

extern struct desc_struct GDT_Table[];
extern struct gate_struct IDT_Table[];
extern unsigned int TSS64_Table[26];

/* 输入: rax = (0x8 << 16) = 段选择子(内核代码段)，ecx = ist(8位), rdx = code_addr, i = 立即数约束 = attr(8位属性)

*/
#define _set_gate(gate_selector_addr, attr, ist, code_addr)                                                 \
	do                                                                                                      \
	{                                                                                                       \
		u64_t __d0, __d1;                                                                           \
		__asm__ __volatile__("movw	%%dx,	%%ax	\n\t"                                                         \
							 "andq	$0x7,	%%rcx	\n\t"                                                        \
							 "addq	%4,	%%rcx	\n\t"                                                          \
							 "shlq	$32,	%%rcx	\n\t"                                                         \
							 "addq	%%rcx,	%%rax	\n\t"                                                       \
							 "xorq	%%rcx,	%%rcx	\n\t"                                                       \
							 "movl	%%edx,	%%ecx	\n\t"                                                       \
							 "shrq	$16,	%%rcx	\n\t"                                                         \
							 "shlq	$48,	%%rcx	\n\t"                                                         \
							 "addq	%%rcx,	%%rax	\n\t"                                                       \
							 "movq	%%rax,	%0	\n\t"                                                          \
							 "shrq	$32,	%%rdx	\n\t"                                                         \
							 "movq	%%rdx,	%1	\n\t"                                                          \
							 : "=m"(*((u64_t *)(gate_selector_addr))),                              \
							   "=m"(*(1 + (u64_t *)(gate_selector_addr))), "=&a"(__d0), "=&d"(__d1) \
							 : "i"(attr << 8),                                                              \
							   "3"((u64_t *)(code_addr)), "2"(0x8 << 16), "c"(ist)                  \
							 : "memory");                                                                   \
	} while (0)

#define load_TR(n)                         \
	do                                     \
	{                                      \
		__asm__ __volatile__("ltr	%%ax"    \
							 :             \
							 : "a"(n << 3) \
							 : "memory");  \
	} while (0)

inline static void set_intr_gate(unsigned int n, unsigned char ist, void *addr)
{
	_set_gate(IDT_Table + n, 0x8E, ist, addr); // P,DPL=0,TYPE=E
}

inline static void set_trap_gate(unsigned int n, unsigned char ist, void *addr)
{
	_set_gate(IDT_Table + n, 0x8F, ist, addr); // P,DPL=0,TYPE=F
}

inline static void set_system_gate(unsigned int n, unsigned char ist, void *addr)
{
	_set_gate(IDT_Table + n, 0xEF, ist, addr); // P,DPL=3,TYPE=F
}

inline static void set_system_intr_gate(unsigned int n, unsigned char ist, void *addr) // int3
{
	_set_gate(IDT_Table + n, 0xEE, ist, addr); // P,DPL=3,TYPE=E
}

inline static void set_tss64(u64_t rsp0, u64_t rsp1, u64_t rsp2, u64_t ist1, u64_t ist2, u64_t ist3,
			   u64_t ist4, u64_t ist5, u64_t ist6, u64_t ist7)
{
	*(u64_t *)(TSS64_Table + 1) = rsp0;
	*(u64_t *)(TSS64_Table + 3) = rsp1;
	*(u64_t *)(TSS64_Table + 5) = rsp2;

	*(u64_t *)(TSS64_Table + 9) = ist1;
	*(u64_t *)(TSS64_Table + 11) = ist2;
	*(u64_t *)(TSS64_Table + 13) = ist3;
	*(u64_t *)(TSS64_Table + 15) = ist4;
	*(u64_t *)(TSS64_Table + 17) = ist5;
	*(u64_t *)(TSS64_Table + 19) = ist6;
	*(u64_t *)(TSS64_Table + 21) = ist7;

}

#endif
