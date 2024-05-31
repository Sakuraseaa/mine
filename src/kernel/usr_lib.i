# 3 "usr_lib.c"
int errno = 0;
# 16 "usr_lib.c"
__asm__(".global "
        "putstring"
        "	\n\t"
        ".type	"
        "putstring"
        ",	@function \n\t"
        "putstring"
        ":		\n\t"
        "pushq   %rbp	\n\t"
        "movq    %rsp,	%rbp	\n\t"
        "movq	$"
        "__NR_putstring"
        ",	%rax	\n\t"
        "jmp	LABEL_SYSCALL	\n\t");

__asm__(".global "
        "open"
        "	\n\t"
        ".type	"
        "open"
        ",	@function \n\t"
        "open"
        ":		\n\t"
        "pushq   %rbp	\n\t"
        "movq    %rsp,	%rbp	\n\t"
        "movq	$"
        "__NR_open"
        ",	%rax	\n\t"
        "jmp	LABEL_SYSCALL	\n\t");
__asm__(".global "
        "close"
        "	\n\t"
        ".type	"
        "close"
        ",	@function \n\t"
        "close"
        ":		\n\t"
        "pushq   %rbp	\n\t"
        "movq    %rsp,	%rbp	\n\t"
        "movq	$"
        "__NR_close"
        ",	%rax	\n\t"
        "jmp	LABEL_SYSCALL	\n\t");
__asm__(".global "
        "read"
        "	\n\t"
        ".type	"
        "read"
        ",	@function \n\t"
        "read"
        ":		\n\t"
        "pushq   %rbp	\n\t"
        "movq    %rsp,	%rbp	\n\t"
        "movq	$"
        "__NR_read"
        ",	%rax	\n\t"
        "jmp	LABEL_SYSCALL	\n\t");
__asm__(".global "
        "write"
        "	\n\t"
        ".type	"
        "write"
        ",	@function \n\t"
        "write"
        ":		\n\t"
        "pushq   %rbp	\n\t"
        "movq    %rsp,	%rbp	\n\t"
        "movq	$"
        "__NR_write"
        ",	%rax	\n\t"
        "jmp	LABEL_SYSCALL	\n\t");
__asm__(".global "
        "lseek"
        "	\n\t"
        ".type	"
        "lseek"
        ",	@function \n\t"
        "lseek"
        ":		\n\t"
        "pushq   %rbp	\n\t"
        "movq    %rsp,	%rbp	\n\t"
        "movq	$"
        "__NR_lseek"
        ",	%rax	\n\t"
        "jmp	LABEL_SYSCALL	\n\t");

__asm__(".global "
        "fork"
        "	\n\t"
        ".type	"
        "fork"
        ",	@function \n\t"
        "fork"
        ":		\n\t"
        "pushq   %rbp	\n\t"
        "movq    %rsp,	%rbp	\n\t"
        "movq	$"
        "__NR_fork"
        ",	%rax	\n\t"
        "jmp	LABEL_SYSCALL	\n\t");
__asm__(".global "
        "vfork"
        "	\n\t"
        ".type	"
        "vfork"
        ",	@function \n\t"
        "vfork"
        ":		\n\t"
        "pushq   %rbp	\n\t"
        "movq    %rsp,	%rbp	\n\t"
        "movq	$"
        "__NR_vfork"
        ",	%rax	\n\t"
        "jmp	LABEL_SYSCALL	\n\t");

__asm__(".global "
        "brk"
        "	\n\t"
        ".type	"
        "brk"
        ",	@function \n\t"
        "brk"
        ":		\n\t"
        "pushq   %rbp	\n\t"
        "movq    %rsp,	%rbp	\n\t"
        "movq	$"
        "__NR_brk"
        ",	%rax	\n\t"
        "jmp	LABEL_SYSCALL	\n\t");

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
