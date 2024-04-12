#include "syscall.h"

// ## 用于连接两个宏值， # 将其后的内容强制转换为字符串
#define SYSFUNC_DEF(name) _SYSFUNC_DEF_(name, __NR_##name)
#define _SYSFUNC_DEF_(name, nr) __SYSFUNC_DEF__(name, nr)

#define __SYSFUNC_DEF__(name, nr)                           \
    __asm__(                                                \
        ".global " #name "	\n\t"                            \
        ".type	" #name ",	@function \n\t" #name ":		\n\t" \
        "movq	$" #nr ",	%rax	\n\t"                        \
        "jmp	LABEL_SYSCALL	\n\t");

SYSFUNC_DEF(putstring)
SYSFUNC_DEF(open)
SYSFUNC_DEF(close)
SYSFUNC_DEF(read)
SYSFUNC_DEF(write)
SYSFUNC_DEF(lseek)
SYSFUNC_DEF(fork)
SYSFUNC_DEF(vfork)
SYSFUNC_DEF(brk)

// sysenter 不会保护当前环境，依据sysexit的用法，
// 我们需要保存需要恢复的rip到rdx,需要恢复的rsp到rcx
// rax寄存器把执行结果返回到应用层并保存在变量ret中
// asm(
//     "LABLE_SYSCALL: \n\t"
//     "pushq %%r10 \n\t"
//     "pushq %%r11 \n\t"
//     "leaq sysexit_return_address(%%rip), %%r10 \n\t" // 再次执行的rip
//     "movq %%rsp, %%r11 \n\t"                         // 再次执行的rsp
//     "sysenter \n\t"                                  // 进入内核层，跳转到entry.S的system_call
//     "sysexit_return_address:  \n\t"
//     "xchgq %%rdx, %%r10 \n\t"
//     "xchgq %%rcx, %%r11 \n\t"
//     "popq %%r11 \n\t"
//     "popq %%r10 \n\t"
//     "cmpq $-0x1000, %rax \n\t"
//     "jb LABLE_SYSCALL_RET \n\t"
//     "movq %rax, errno(%rip) \n\t"
//     "orq $-1, %rax \n\t"
//     "LABEL_SYSCALL_RET: \n\t"
//     "retq \n\t");

// 这里的汇编语法格式是什么东西？ 为什么寄存器前面不用%%了？
__asm__(
    "LABEL_SYSCALL:	\n\t"
    "pushq	%r10	\n\t"
    "pushq	%r11	\n\t"
    "leaq	sysexit_return_address(%rip),	%r10	\n\t"
    "movq	%rsp,	%r11		\n\t"
    "sysenter			\n\t"
    "sysexit_return_address:	\n\t"
    "xchgq	%rdx,	%r10	\n\t"
    "xchgq	%rcx,	%r11	\n\t"
    "popq	%r11	\n\t"
    "popq	%r10	\n\t"
    "cmpq	$-0x1000,	%rax	\n\t"
    "jb	LABEL_SYSCALL_RET	\n\t"
    "movq	%rax,	errno(%rip)	\n\t"
    "orq	$-1,	%rax	\n\t"
    "LABEL_SYSCALL_RET:	\n\t"
    "retq	\n\t");