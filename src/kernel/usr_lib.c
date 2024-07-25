#include "SYSCALL.h"

int errno = 0;

#define SYSFUNC_DEF(name) _SYSFUNC_DEF_(name, __NR_##name)
#define _SYSFUNC_DEF_(name, nr) __SYSFUNC_DEF__(name, nr)
#define __SYSFUNC_DEF__(name, nr)                           \
    __asm__(                                                \
        ".section .text	\n\t"                               \
        ".global " #name "	\n\t"                            \
        ".type	" #name ",	@function \n\t" #name ":		\n\t" \
        "pushq   %rbp	\n\t"                                 \
        "movq    %rsp,	%rbp	\n\t"                           \
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
SYSFUNC_DEF(execve)
SYSFUNC_DEF(exit)
SYSFUNC_DEF(wait4)

SYSFUNC_DEF(brk)
SYSFUNC_DEF(reboot)

SYSFUNC_DEF(chdir)
SYSFUNC_DEF(getdents)

SYSFUNC_DEF(SIGNAL)
SYSFUNC_DEF(KILL)
SYSFUNC_DEF(sleep)
SYSFUNC_DEF(getpid)

SYSFUNC_DEF(mkdir)

SYSFUNC_DEF(getcwd)
SYSFUNC_DEF(stat)

SYSFUNC_DEF(info)
SYSFUNC_DEF(cleanScreen)

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
    "leaveq	\n\t"
    "retq	\n\t");
