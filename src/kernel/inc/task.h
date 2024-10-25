#ifndef __TASK_H__ // #ifndef 当且仅当变量未定义时为真，#ifdef 当前仅当
// 变量已定义时为真。 一旦检测结构为真，则执行后续操作直至遇见#endif指令为止
#define __TASK_H__


// 每个任务的文件描述符最大数
#define TASK_FILE_MAX 10

#define KERNEL_CS (0x08)
#define KERNEL_DS (0x10)

#define USER_CS (0x28)
#define USER_DS (0x30)

// stack size 32K
#define STACK_SIZE 32768

extern char_t _text;
extern char_t _etext;
extern char_t _data;
extern char_t _edata;
extern char_t _rodata;
extern char_t _erodata;
extern char_t _bss;
extern char_t _ebss;
extern char_t _end;

extern u64_t _stack_start;
extern s64_t global_pid;

extern void ret_system_call();

extern u64_t kallsyms_addresses[] __attribute__((__weak__));
extern s64_t kallsyms_syms_num __attribute__((__weak__));
extern s64_t kallsyms_index[] __attribute__((__weak__));
extern char_t* kallsyms_names __attribute((__weak__));

//// task_t.falgs
#define PF_KTHREAD (1UL << 0)
#define NEED_SCHEDULE (1UL << 1) // 若此标志被置位，则表明当前进程可在适当时机进行调度
#define PF_VFORK (1UL << 2)		 // 当前进程的资源是否存在共享，区分fork与vfork创建出的进程

// 定义进程状态, 没有进程新建状态
#define TASK_RUNNING (1UL << 0)
#define TASK_INTERRUPTIBLE (1UL << 1)
#define TASK_UNINTERRUPTIBLE (1UL << 2)
#define TASK_ZOMBIE (1UL << 3)
#define TASK_STOPPED (1UL << 4)
// =================================== 定义关于进程的结构体 =======================
typedef struct thread_struct
{
	u64_t rsp0; // in tss, 记录着应用程序在内核层使用的栈基地址

	u64_t rip; // 保存着进程切换回来时执行代码的地址
	u64_t rsp; // 保存着进程切换时的栈指针

	u64_t fs;
	u64_t gs;

	u64_t cr2;		  // CR2控制寄存器
	u64_t trap_nr;	  // 产生异常的异常号
	u64_t error_code; // 异常的错误码
}thread_t;

// 进程PCB
typedef struct task_struct
{

	volatile s64_t state;		// 进程状态: 运行态，停止态，可中断态
	u64_t flags; 		// 进程标志：进程，线程，内核线程
	s64_t preempt_count;	 		// 持有的自旋锁的数量, Linux使用自旋锁来标记非抢占区域: 在持有自旋锁期间关闭抢占功能，直至释放自旋锁为止
	s64_t signal;
	s64_t blocked;  		// 信号位图 和 bitmap of masked signals
	sigaction_t* sigaction;		// 信号将要执行的操作和标志信息, 每一项对应一个信号, 一共三十二项

	
	mmdsc_t *mm;		  // 内存空间分布结构体
	struct thread_struct *thread; // 进程切换时保存的寄存器(上下文)信息
	struct List list;			  // 双向链表节点

	u64_t addr_limit; 	  // 进程地址空间范围
	/*	0x0000,0000,0000,0000 - 0x0000,7fff,ffff,ffff user,   对应第255个PML4页表项， 0 ~ 255   */
	/*	0xffff,8000,0000,0000 - 0xffff,ffff,ffff,ffff kernel, 对应第256个PML4页表项， 256 ~ 511 */

	u64_t pid;

	u32_t uid;				// 用户 id
	u32_t gid;				// 用户组 id
	u16_t umask;				// 进程用户文件掩码

	s64_t priority;			// 进程可用时间片
	s64_t vrun_time; 		// 记录进程虚拟运行时间的成员变量 vrun_time
	s64_t exit_code;
	file_t *file_struct[TASK_FILE_MAX];

	wait_queue_t wait_childexit;
	struct task_struct *next;	// next 用于连接所有进程
	struct task_struct *parent; // parent 用于记录当前进程的父进程

	dir_entry_t *i_pwd;	 // 进程当前目录 inode program work directory
	dir_entry_t *i_root; // 进程根目录 inode
}task_t;

// 进程PCB和内核栈空间 32kb
union task_union
{
	task_t task;
	u64_t stack[STACK_SIZE / sizeof(u64_t)];
} __attribute__((aligned(8))); // 8Bytes align

// ========================== 初始化内核进程的PCB ==============================
// root
#define INIT_TASK(tsk)                    \
	{                                     \
		.state = TASK_UNINTERRUPTIBLE,    \
		.flags = PF_KTHREAD,              \
		.mm = &initmm,                   \
		.thread = &init_thread,           \
		.addr_limit = 0xffff800000000000, \
		.pid = (adr_t)&tsk,                      \
		.preempt_count = 0,               \
		.vrun_time = 0,                   \
		.signal = 0,                      \
		.blocked = 0,                     \
		.sigaction = (nullptr),              \
		.priority = 2,                    \
		.file_struct = {0},               \
		.next = nullptr,                     \
		.parent = &tsk,                   \
		.uid = 0,	\
		.gid = 0,  \
		.umask = 0022,					\
		.i_pwd = nullptr, \
		.i_root = nullptr, \
	}
extern task_t *init_task[NR_CPUS];
extern union task_union init_task_union;
extern struct thread_struct init_thread;
// ================================ 初始化TSS结构体 ============================
struct tss_struct
{
	u32_t reserved0;
	u64_t rsp0;
	u64_t rsp1;
	u64_t rsp2;
	u64_t reserved1;
	u64_t ist1;
	u64_t ist2;
	u64_t ist3;
	u64_t ist4;
	u64_t ist5;
	u64_t ist6;
	u64_t ist7;
	u64_t reserved2;
	u16_t reserved3;
	u16_t iomapbaseaddr;
} __attribute__((packed));

#define INIT_TSS                                                                             \
	{                                                                                        \
		.reserved0 = 0,                                                                      \
		.rsp0 = (u64_t)(init_task_union.stack + STACK_SIZE / sizeof(u64_t)), \
		.rsp1 = (u64_t)(init_task_union.stack + STACK_SIZE / sizeof(u64_t)), \
		.rsp2 = (u64_t)(init_task_union.stack + STACK_SIZE / sizeof(u64_t)), \
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
extern task_t *my_cur;
static inline task_t *get_current()
{
	task_t *current = nullptr;

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
/*
// 线程切换函数
// 保存next的rsp, 加载prev的rsp, 加载再次被调度的地址给prev.rip
// 压入next的rip, 跳转执行__switch_to，返回时会执行next的rip
// #define switch_to(prev, next)                                                                       \
// 	do                                                                                              \
// 	{                                                                                               \
// 		__asm__ __volatile__("pushq	%%rbp	\n\t"                                                     \
// 							 "pushq	%%rax	\n\t"                                                     \
// 							 "movq	%%rsp,	%0	\n\t"                                                  \
// 							 "movq	%2,	%%rsp	\n\t"                                                  \
// 							 "leaq	1f(%%rip),	%%rax	\n\t"                                           \
// 							 "movq	%%rax,	%1	\n\t"                                                  \
// 							 "pushq	%3		\n\t"                                                       \
// 							 "jmp	__switch_to	\n\t"                                                 \
// 							 "1:	\n\t"                                                              \
// 							 "popq	%%rax	\n\t"                                                      \
// 							 "popq	%%rbp	\n\t"                                                      \
// 							 : "=m"(prev->thread->rsp), "=m"(prev->thread->rip)                     \
// 							 : "m"(next->thread->rsp), "m"(next->thread->rip), "D"(prev), "S"(next) \
// 							 : "memory");                                                           \
// 	} while (0)*/




#endif
