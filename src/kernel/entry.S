#include "linkage.h"
R15	=	0x00
R14	=	0x08
R13	=	0x10
R12	=	0x18
R11	=	0x20
R10	=	0x28
R9	=	0x30
R8	=	0x38
RBX	=	0x40
RCX	=	0x48
RDX	=	0x50
RSI	=	0x58
RDI	=	0x60
RBP	=	0x68
DS	=	0x70
ES	=	0x78
RAX	=	0x80
FUNC	=	0x88
ERRCODE	=	0x90
RIP	=	0x98
CS	=	0xa0
RFLAGS	=	0xa8
OLDRSP	=	0xb0
OLDSS	=	0xb8

/// struct task_struct member offset
TSK_STATE = 0x00
TSK_FLAGS = 0x08
TSK_PREEMPT = 0x10
TSK_SIGNAL = 0x18

RESTORE_ALL:
	popq	%r15	
	popq	%r14		
	popq	%r13		
	popq	%r12		
	popq	%r11		
	popq	%r10	
	popq	%r9		
	popq	%r8		
	popq	%rbx		
	popq	%rcx		
	popq	%rdx		
	popq	%rsi	
	popq	%rdi		
	popq	%rbp		
	
	popq	%rax		
	movq	%rax,	%ds	
	popq	%rax		
	movq	%rax,	%es	
	
	popq	%rax	
	
	addq	$0x10,	%rsp;
	iretq

#define GET_CURRENT(reg)	\
	movq	$-32768,reg;	\
	andq	%rsp,	reg

// 中断,异常,退出函数
ret_from_exception:
ENTRY(ret_from_intr)
	movq $-1, %rcx
	testq softirq_status(%rip), %rcx // 在中断返回过程中检测是否有软中断，
	jnz softirq_handler				 // 如果有，则跳转执行do_softirq
ret_with_reschedule:	
	GET_CURRENT(%rbx)				 // 得到当前运行的进程的PCB
	movq TSK_PREEMPT(%rbx), %rcx	 // 检测本进程是否持有自旋锁(preemp > 0)，如果有，则跳过抢占检测代码,直接从中断返回
	cmpq $0, %rcx					 // 只有当preempt_count = 0 时，系统才会开启抢占功能
	jne RESTORE_ALL
	movq TSK_FLAGS(%rbx), %rcx		 // 检测本进程是否可调度(可抢占)，如果可调度,则跳转去执行reschedule	
	testq $2,  %rcx
	jnz reschedule
	jmp	RESTORE_ALL	

softirq_handler:
	callq do_softirq
	jmp ret_with_reschedule

reschedule:
	callq schedule
	jmp RESTORE_ALL

// =============== 进入系统调用 =========================
ENTRY(system_call)
	sti				 // 开中断
	subq $0x38, %rsp //给 ss,rsp, rflags, cs, rip, error_code, func 留出内存空间
	cld			 	 // 清除方向位

	pushq %rax	 	 // 保存上下文环境，要返回的rip和rsp，也被记录再这里（内核栈）
	
	movq %es, %rax
	pushq %rax
	movq %ds, %rax
	pushq %rax
	xorq %rax, %rax
	pushq %rbp
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rbx
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15

	movq $0x10, %rax	// 切换内核数据段
	movq %rax, %ds
	movq %rax, %es

	mov RAX(%rsp), %rax
	leaq system_call_table(%rip), %rbx
	callq *(%rbx, %rax, 8)	// *(rbx + rax * 8)
	movq %rax, RAX(%rsp)    // 把rax记录再要恢复的下文环境中


// ===============	返回用户进程	=========================
ENTRY(ret_system_call)
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbx
	popq %rcx
	popq %rdx
	popq %rsi
	popq %rdi
	popq %rbp
	
	popq %rax
	movq %rax, %ds
	popq %rax
	movq %rax, %es

	popq %rax
	addq $0x38, %rsp

	xchgq %rdx, %r10
	xchgq %rcx, %r11

	sti
	.byte 0x48
	sysexit
	
	/*使用SYSEXIT指令可以快速返回3特权级指令,
	/*1.需要IA32_SYSTEMR_CS寄存器提供3特权级的代码段选择子和栈段选择子*/
	/*2.RDX寄存器会被载入RIP寄存器，RCX会被载入RSP寄存器*/

// ===============================================================
ENTRY(kernel_thread_func)
		popq	%r15	
		popq	%r14	
		popq	%r13	
		popq	%r12	
		popq	%r11
		popq	%r10	
		popq	%r9	
		popq	%r8	
		popq	%rbx	
		popq	%rcx	
		popq	%rdx	
		popq	%rsi	
		popq	%rdi	
		popq	%rbp	
		popq	%rax	
		movq	%rax,	%ds	
		popq	%rax		
		movq	%rax,	%es	
		popq	%rax		
		addq	$0x38,	%rsp	
	/////////////////////////////////
		movq	%rdx,	%rdi
		// sti
		callq	*%rbx		
		movq	%rax,	%rdi	
		callq	do_exit		
// ==============================================================
ENTRY(divide_error)
	pushq	$0        // 压入错误码
	pushq	%rax	  // 把目标程序地址入栈，这里之所以使用xchagq,是为了不覆盖rax内存
	leaq	do_divide_error(%rip),	%rax
	xchgq	%rax,	(%rsp) 	// 这里等于压入了regs中的func

error_code:
	pushq	%rax		 // 压入rax
	movq	%es,	%rax // 保存两个段选择子
	pushq	%rax
	movq	%ds,	%rax
	pushq	%rax
	xorq	%rax,	%rax

	pushq	%rbp		// 保护进程上文
	pushq	%rdi
	pushq	%rsi
	pushq	%rdx
	pushq	%rcx
	pushq	%rbx
	pushq	%r8
	pushq	%r9
	pushq	%r10
	pushq	%r11
	pushq	%r12
	pushq	%r13
	pushq	%r14
	pushq	%r15	
	
	cld
	movq	ERRCODE(%rsp),	%rsi	// rsi记录第二参数传入 错误码
	movq	FUNC(%rsp),	%rdx		// rdx 中保存中断处理函数

	movq	$0x10,	%rdi	// 切换内核数据段选择子
	movq	%rdi,	%ds
	movq	%rdi,	%es


	movq	%rsp,	%rdi	// rsp记录第一参数 传入栈基址

	callq 	*%rdx	// 调用C程序中断处理例程

	jmp	ret_from_exception		//从异常返回

ENTRY(debug)
	pushq	$0
	pushq	%rax
	leaq	do_debug(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(nmi)
	pushq	%rax
	cld			
	pushq	%rax;	
	
	pushq	%rax
	movq	%es,	%rax
	pushq	%rax
	movq	%ds,	%rax
	pushq	%rax
	xorq	%rax,	%rax
	
	pushq	%rbp;		
	pushq	%rdi;		
	pushq	%rsi;		
	pushq	%rdx;		
	pushq	%rcx;		
	pushq	%rbx;		
	pushq	%r8;		
	pushq	%r9;		
	pushq	%r10;		
	pushq	%r11;		
	pushq	%r12;		
	pushq	%r13;		
	pushq	%r14;		
	pushq	%r15;
	
	movq	$0x10,	%rdx;	
	movq	%rdx,	%ds;	
	movq	%rdx,	%es;
	
	movq	$0,	%rsi
	movq	%rsp,	%rdi

	callq	do_nmi

	jmp	RESTORE_ALL

ENTRY(int3)
	pushq	$0
	pushq	%rax
	leaq	do_int3(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(overflow)
	pushq	$0
	pushq	%rax
	leaq	do_overflow(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(bounds)
	pushq	$0
	pushq	%rax
	leaq	do_bounds(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(undefined_opcode)
	pushq	$0
	pushq	%rax
	leaq	do_undefined_opcode(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(dev_not_available)	/* need rewrite */
	pushq	$0
	pushq	%rax
	leaq	do_dev_not_available(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code


ENTRY(double_fault)
	pushq	%rax
	leaq	do_double_fault(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(coprocessor_segment_overrun)
	pushq	$0
	pushq	%rax
	leaq	do_coprocessor_segment_overrun(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(invalid_TSS)
	pushq	%rax
	leaq	do_invalid_TSS(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(segment_not_present)
	pushq	%rax
	leaq	do_segment_not_present(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(stack_segment_fault)
	pushq	%rax
	leaq	do_stack_segment_fault(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(general_protection)
	pushq	%rax
	leaq	do_general_protection(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(page_fault)
	pushq	%rax
	leaq	do_page_fault(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(x87_FPU_error)
	pushq	$0
	pushq	%rax
	leaq	do_x87_FPU_error(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(alignment_check)
	pushq	%rax
	leaq	do_alignment_check(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(machine_check)
	pushq	$0
	pushq	%rax
	leaq	do_machine_check(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(SIMD_exception)
	pushq	$0
	pushq	%rax
	leaq	do_SIMD_exception(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code

ENTRY(virtualization_exception)
	pushq	$0
	pushq	%rax
	leaq	do_virtualization_exception(%rip),	%rax
	xchgq	%rax,	(%rsp)
	jmp	error_code


