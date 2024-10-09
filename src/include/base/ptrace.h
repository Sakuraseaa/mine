#ifndef __PTRACE_H__

#define __PTRACE_H__

typedef struct pt_regs
{
	u64_t r15;
	u64_t r14;
	u64_t r13;
	u64_t r12;
	u64_t r11;
	u64_t r10;
	u64_t r9;
	u64_t r8;
	u64_t rbx;
	u64_t rcx;
	u64_t rdx;
	u64_t rsi;
	u64_t rdi;
	u64_t rbp;
	u64_t ds;
	u64_t es;
	u64_t rax;
	u64_t func;
	u64_t errcode;
	u64_t rip;
	u64_t cs;
	u64_t rflags;
	u64_t rsp;
	u64_t ss;
}pt_regs_t;

#endif
