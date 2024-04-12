#ifndef __TASK_H__ // #ifndef 当且仅当变量未定义时为真，#ifdef 当前仅当
// 变量已定义时为真。 一旦检测结构为真，则执行后续操作直至遇见#endif指令为止

#define __TASK_H__

#include "memory.h"
#include "cpu.h"
#include "lib.h"
#include "ptrace.h"
#include "printk.h"
#include "VFS.h"
#include "sched.h"

// 每个任务的文件描述符最大数
#define TASK_FILE_MAX 10

#define KERNEL_CS (0x08)
#define KERNEL_DS (0x10)

#define USER_CS (0x28)
#define USER_DS (0x30)

// stack size 32K
#define STACK_SIZE 32768

extern char _text;
extern char _etext;
extern char _data;
extern char _edata;
extern char _rodata;
extern char _erodata;
extern char _bss;
extern char _ebss;
extern char _end;

extern unsigned long _stack_start;
extern long global_pid;

extern void ret_system_call();

//// struct task_struct.falgs
#define PF_KTHREAD (1UL << 0)
#define NEED_SCHEDULE (1UL << 1) // 若此标志被置位，则表明当前进程可在适当时机进行调度
#define PF_VFORK (1UL << 2)		 // 当前进程的资源是否存在共享，区分fork与vfork创建出的进程

// 定义进程状态
#define TASK_RUNNING (1 << 0)
#define TASK_INTERRUPTIBLE (1 << 1)
#define TASK_UNINTERRUPTIBLE (1 << 2)
#define TASK_ZOMBIE (1 << 3)
#define TASK_STOPPED (1 << 4)
// =================================== 定义关于进程的结构体 =======================
// 描述进程占用内存的结构体
struct mm_struct
{
	pml4t_t *pgd; // page table point

	unsigned long start_code, end_code;		// code segment space
	unsigned long start_data, end_data;		// data segment space
	unsigned long start_rodata, end_rodata; // rodata(read-only-data) segment space
	unsigned long start_bss, end_bss;
	unsigned long start_brk, end_brk; // 堆空间-动态内存分配区
	unsigned long start_stack;		  // 应用层栈基地址
};

struct thread_struct
{
	unsigned long rsp0; // in tss 内核层栈基地址

	unsigned long rip; // 内核层代码指针
	unsigned long rsp; // 内核层当前栈指针

	unsigned long fs;
	unsigned long gs;

	unsigned long cr2;		  // CR2控制寄存器
	unsigned long trap_nr;	  // 产生异常的异常号
	unsigned long error_code; // 异常的错误码
};

// 进程PCB
struct task_struct
{

	volatile long state; // 进程状态: 运行态，停止态，可中断态
	unsigned long flags; // 进程标志：进程，线程，内核线程
	long preempt_count;
	long signal;

	struct mm_struct *mm;		  // 内存空间分布结构体，记录内存页表和程序段信息
	struct thread_struct *thread; // 进程切换时保留的状态信息
	struct List list;			  // 双向链表节点

	unsigned long addr_limit; // 进程地址空间范围
	/*0x0000,0000,0000,0000 - 0x0000,7fff,ffff,ffff user, 对应第255个PML4页表项， 0 ~ 255*/
	/*0xffff,8000,0000,0000 - 0xffff,ffff,ffff,ffff kernel, 对应第256个PML4页表项， 256 ~ 511*/

	long pid;
	long priority;	// 进程可用时间片
	long vrun_time; // 记录进程虚拟运行时间的成员变量 vrun_time
	struct file *file_struct[TASK_FILE_MAX];

	struct task_struct *next;	// next 用于连接所有进程
	struct task_struct *parent; // parent 用于记录当前进程的父进程
};

// 进程PCB和内核栈空间 32kb
union task_union
{
	struct task_struct task;
	unsigned long stack[STACK_SIZE / sizeof(unsigned long)];
} __attribute__((aligned(8))); // 8Bytes align

// ========================== 初始化内核进程的PCB ==============================
#define INIT_TASK(tsk)                    \
	{                                     \
		.state = TASK_UNINTERRUPTIBLE,    \
		.flags = PF_KTHREAD,              \
		.mm = &init_mm,                   \
		.thread = &init_thread,           \
		.addr_limit = 0xffff800000000000, \
		.pid = 0,                         \
		.preempt_count = 0,               \
		.vrun_time = 0,                   \
		.signal = 0,                      \
		.priority = 2,                    \
		.file_struct = {0},               \
		.next = &tsk,                     \
		.parent = &tsk,                   \
	}
extern struct task_struct *init_task[NR_CPUS];
extern union task_union init_task_union;
extern struct mm_struct init_mm;
extern struct thread_struct init_thread;
// ================================ 初始化TSS结构体 ============================
struct tss_struct
{
	unsigned int reserved0;
	unsigned long rsp0;
	unsigned long rsp1;
	unsigned long rsp2;
	unsigned long reserved1;
	unsigned long ist1;
	unsigned long ist2;
	unsigned long ist3;
	unsigned long ist4;
	unsigned long ist5;
	unsigned long ist6;
	unsigned long ist7;
	unsigned long reserved2;
	unsigned short reserved3;
	unsigned short iomapbaseaddr;
} __attribute__((packed));

#define INIT_TSS                                                                             \
	{                                                                                        \
		.reserved0 = 0,                                                                      \
		.rsp0 = (unsigned long)(init_task_union.stack + STACK_SIZE / sizeof(unsigned long)), \
		.rsp1 = (unsigned long)(init_task_union.stack + STACK_SIZE / sizeof(unsigned long)), \
		.rsp2 = (unsigned long)(init_task_union.stack + STACK_SIZE / sizeof(unsigned long)), \
		.reserved1 = 0,                                                                      \
		.ist1 = 0xffff800000007c00,                                                          \
		.ist2 = 0xffff800000007c00,                                                          \
		.ist3 = 0xffff800000007c00,                                                          \
		.ist4 = 0xffff800000007c00,                                                          \
		.ist5 = 0xffff800000007c00,                                                          \
		.ist6 = 0xffff800000007c00,                                                          \
		.ist7 = 0xffff800000007c00,                                                          \
		.reserved2 = 0,                                                                      \
		.reserved3 = 0,                                                                      \
		.iomapbaseaddr = 0                                                                   \
	}

extern struct tss_struct init_tss[NR_CPUS];
// ========================================================================================
// 得到当前运行进程的PCB
extern struct task_struct *my_cur;
static inline struct task_struct *get_current()
{
	struct task_struct *current = NULL;

	__asm__ __volatile__("andq %%rsp,%0	\n\t"
						 : "=r"(current)
						 : "0"(~32767UL));
	my_cur = current;

	return current;
}

#define current get_current()

#define GET_CURRENT        \
	"movq	%rsp,	%rbx	\n\t" \
	"andq	$-32768,%rbx	\n\t"

// 线程切换函数
// 保存next的rsp, 加载prev的rsp, 加载再次被调度的地址给prev.rip
// 压入next的rip, 跳转执行__switch_to，返回时会执行next的rip
#define switch_to(prev, next)                                                                       \
	do                                                                                              \
	{                                                                                               \
		__asm__ __volatile__("pushq	%%rbp	\n\t"                                                     \
							 "pushq	%%rax	\n\t"                                                     \
							 "movq	%%rsp,	%0	\n\t"                                                  \
							 "movq	%2,	%%rsp	\n\t"                                                  \
							 "leaq	1f(%%rip),	%%rax	\n\t"                                           \
							 "movq	%%rax,	%1	\n\t"                                                  \
							 "pushq	%3		\n\t"                                                       \
							 "jmp	__switch_to	\n\t"                                                 \
							 "1:	\n\t"                                                              \
							 "popq	%%rax	\n\t"                                                      \
							 "popq	%%rbp	\n\t"                                                      \
							 : "=m"(prev->thread->rsp), "=m"(prev->thread->rip)                     \
							 : "m"(next->thread->rsp), "m"(next->thread->rip), "D"(prev), "S"(next) \
							 : "memory");                                                           \
	} while (0)

unsigned long system_call_function(struct pt_regs *regs);
unsigned long do_exit(unsigned long code);
unsigned long do_fork(struct pt_regs *regs, unsigned long clone_flags, unsigned long stack_start, unsigned long stack_size);
void task_init();
void switch_mm(struct task_struct *prev, struct task_struct *next);

#endif
