
__asm__ ( ".section .text	\n\t" ".global ""putstring""	\n\t" ".type	""putstring"",	@function \n\t" "putstring"":		\n\t" "pushq   %rbp	\n\t" "movq    %rsp,	%rbp	\n\t" "movq	$""__NR_putstring"",	%rax	\n\t" "jmp	LABEL_SYSCALL	\n\t" );

__asm__ ( ".section .text	\n\t" ".global ""open""	\n\t" ".type	""open"",	@function \n\t" "open"":		\n\t" "pushq   %rbp	\n\t" "movq    %rsp,	%rbp	\n\t" "movq	$""2"",	%rax	\n\t" "jmp	LABEL_SYSCALL	\n\t" );
__asm__ ( ".section .text	\n\t" ".global ""close""	\n\t" ".type	""close"",	@function \n\t" "close"":		\n\t" "pushq   %rbp	\n\t" "movq    %rsp,	%rbp	\n\t" "movq	$""3"",	%rax	\n\t" "jmp	LABEL_SYSCALL	\n\t" );
__asm__ ( ".section .text	\n\t" ".global ""read""	\n\t" ".type	""read"",	@function \n\t" "read"":		\n\t" "pushq   %rbp	\n\t" "movq    %rsp,	%rbp	\n\t" "movq	$""0"",	%rax	\n\t" "jmp	LABEL_SYSCALL	\n\t" );
__asm__ ( ".section .text	\n\t" ".global ""write""	\n\t" ".type	""write"",	@function \n\t" "write"":		\n\t" "pushq   %rbp	\n\t" "movq    %rsp,	%rbp	\n\t" "movq	$""1"",	%rax	\n\t" "jmp	LABEL_SYSCALL	\n\t" );
__asm__ ( ".section .text	\n\t" ".global ""lseek""	\n\t" ".type	""lseek"",	@function \n\t" "lseek"":		\n\t" "pushq   %rbp	\n\t" "movq    %rsp,	%rbp	\n\t" "movq	$""8"",	%rax	\n\t" "jmp	LABEL_SYSCALL	\n\t" );

__asm__ ( ".section .text	\n\t" ".global ""fork""	\n\t" ".type	""fork"",	@function \n\t" "fork"":		\n\t" "pushq   %rbp	\n\t" "movq    %rsp,	%rbp	\n\t" "movq	$""57"",	%rax	\n\t" "jmp	LABEL_SYSCALL	\n\t" );
__asm__ ( ".section .text	\n\t" ".global ""vfork""	\n\t" ".type	""vfork"",	@function \n\t" "vfork"":		\n\t" "pushq   %rbp	\n\t" "movq    %rsp,	%rbp	\n\t" "movq	$""58"",	%rax	\n\t" "jmp	LABEL_SYSCALL	\n\t" );
__asm__ ( ".section .text	\n\t" ".global ""execve""	\n\t" ".type	""execve"",	@function \n\t" "execve"":		\n\t" "pushq   %rbp	\n\t" "movq    %rsp,	%rbp	\n\t" "movq	$""59"",	%rax	\n\t" "jmp	LABEL_SYSCALL	\n\t" );
__asm__ ( ".section .text	\n\t" ".global ""exit""	\n\t" ".type	""exit"",	@function \n\t" "exit"":		\n\t" "pushq   %rbp	\n\t" "movq    %rsp,	%rbp	\n\t" "movq	$""60"",	%rax	\n\t" "jmp	LABEL_SYSCALL	\n\t" );
__asm__ ( ".section .text	\n\t" ".global ""wait4""	\n\t" ".type	""wait4"",	@function \n\t" "wait4"":		\n\t" "pushq   %rbp	\n\t" "movq    %rsp,	%rbp	\n\t" "movq	$""61"",	%rax	\n\t" "jmp	LABEL_SYSCALL	\n\t" );

__asm__ ( ".section .text	\n\t" ".global ""brk""	\n\t" ".type	""brk"",	@function \n\t" "brk"":		\n\t" "pushq   %rbp	\n\t" "movq    %rsp,	%rbp	\n\t" "movq	$""12"",	%rax	\n\t" "jmp	LABEL_SYSCALL	\n\t" );
__asm__ ( ".section .text	\n\t" ".global ""reboot""	\n\t" ".type	""reboot"",	@function \n\t" "reboot"":		\n\t" "pushq   %rbp	\n\t" "movq    %rsp,	%rbp	\n\t" "movq	$""169"",	%rax	\n\t" "jmp	LABEL_SYSCALL	\n\t" );

__asm__ ( ".section .text	\n\t" ".global ""chdir""	\n\t" ".type	""chdir"",	@function \n\t" "chdir"":		\n\t" "pushq   %rbp	\n\t" "movq    %rsp,	%rbp	\n\t" "movq	$""80"",	%rax	\n\t" "jmp	LABEL_SYSCALL	\n\t" );
__asm__ ( ".section .text	\n\t" ".global ""getdents""	\n\t" ".type	""getdents"",	@function \n\t" "getdents"":		\n\t" "pushq   %rbp	\n\t" "movq    %rsp,	%rbp	\n\t" "movq	$""78"",	%rax	\n\t" "jmp	LABEL_SYSCALL	\n\t" );

__asm__ ( ".section .text	\n\t" ".global ""SIGNAL""	\n\t" ".type	""SIGNAL"",	@function \n\t" "SIGNAL"":		\n\t" "pushq   %rbp	\n\t" "movq    %rsp,	%rbp	\n\t" "movq	$""__NR_SIGNAL"",	%rax	\n\t" "jmp	LABEL_SYSCALL	\n\t" );
__asm__ ( ".section .text	\n\t" ".global ""KILL""	\n\t" ".type	""KILL"",	@function \n\t" "KILL"":		\n\t" "pushq   %rbp	\n\t" "movq    %rsp,	%rbp	\n\t" "movq	$""__NR_KILL"",	%rax	\n\t" "jmp	LABEL_SYSCALL	\n\t" );
__asm__ ( ".section .text	\n\t" ".global ""sleep""	\n\t" ".type	""sleep"",	@function \n\t" "sleep"":		\n\t" "pushq   %rbp	\n\t" "movq    %rsp,	%rbp	\n\t" "movq	$""__NR_sleep"",	%rax	\n\t" "jmp	LABEL_SYSCALL	\n\t" );
__asm__ ( ".section .text	\n\t" ".global ""getpid""	\n\t" ".type	""getpid"",	@function \n\t" "getpid"":		\n\t" "pushq   %rbp	\n\t" "movq    %rsp,	%rbp	\n\t" "movq	$""39"",	%rax	\n\t" "jmp	LABEL_SYSCALL	\n\t" );

__asm__ (
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
"retq	\n\t"
);
