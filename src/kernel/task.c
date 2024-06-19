#include "ptrace.h"
#include "printk.h"
#include "lib.h"
#include "memory.h"
#include "linkage.h"
#include "gate.h"
#include "schedule.h"
#include "task.h"
#include "unistd.h"
#include "fcntl.h"
#include "stdio.h"
#include "errno.h"
#include "execv.h"
#include "debug.h"
// ----------- DEBUGE -------------------
#include "sys.h"
// ----------- DEBUGE -------------------
union task_union init_task_union
	__attribute__((__section__(".data.init_task"))) = {INIT_TASK(init_task_union.task)};

struct task_struct *init_task[NR_CPUS] = {&init_task_union.task, 0};

struct mm_struct init_mm = {0};

struct thread_struct init_thread =
	{
		.rsp0 = (unsigned long)(init_task_union.stack + STACK_SIZE / sizeof(unsigned long)),
		.rsp = (unsigned long)(init_task_union.stack + STACK_SIZE / sizeof(unsigned long)),
		.fs = KERNEL_DS,
		.gs = KERNEL_DS,
		.cr2 = 0,
		.trap_nr = 0,
		.error_code = 0};

struct tss_struct init_tss[NR_CPUS] = {[0 ... NR_CPUS - 1] = INIT_TSS};
struct task_struct *my_cur;
extern void ret_system_call(void);	  // 进入特权级3
extern void kernel_thread_func(void); // 进入用户进程，在执行完用户进程后，会执行do_exit()程序
extern void system_call(void);

unsigned long shell_boot(unsigned long arg);
unsigned long do_execve(struct pt_regs *regs, char *name, char* argv[], char* envp[]);
long global_pid;

struct task_struct *get_task(long pid)
{
	struct task_struct *tsk = NULL;

	for (tsk = init_task_union.task.next; tsk != &init_task_union.task; tsk = tsk->next)
	{
		if (tsk->pid == pid)
			return tsk;
	}

	return NULL;
}

// 用户进程 执行一次系统调用, 已经过时了
void user_level_function()
{
	long errno = 0;
	// 这里会出现截断现象, 但如果我给string的内存配置大一点就不会 这是为甚么？
	char path[7] = "/a.txt";
	char buf[50];
	char str[] = "YSKM";
	int fd = -1;
	// call sys_open
	// int open(const char* path, int oflag)

	// sysenter 不会保护当前环境，依据sysexit的用法，
	// 我们需要保存需要恢复的rip到rdx,需要恢复的rsp到rcx
	// rax寄存器把执行结果返回到应用层并保存在变量ret中
	__asm__ __volatile__("sub $0x100, %%rsp \n\t"
						 "pushq %%r10 \n\t"
						 "pushq %%r11 \n\t"
						 "leaq sysexit_return_address0(%%rip), %%r10 \n\t" // 保存应用层的rip
						 "movq %%rsp, %%r11 \n\t"						   // 保存应用层的rsp
						 "sysenter \n\t"								   // 进入内核层，跳转到entry.S的system_call
						 "sysexit_return_address0:  \n\t"
						 "xchgq %%rdx, %%r10 \n\t"
						 "xchgq %%rcx, %%r11 \n\t"
						 "popq %%r11 \n\t"
						 "popq %%r10 \n\t"
						 : "=a"(errno)
						 : "0"(__NR_open), "D"(path), "S"(O_APPEND)
						 : "memory");
	fd = errno; // 文件描述符

	// call sys_write
	// long write(int fildes, void* buf, long nbyte);
	__asm__ __volatile__("pushq %%r10 \n\t"
						 "pushq %%r11 \n\t"
						 "leaq sysexit_return_address2(%%rip), %%r10 \n\t" // 再次执行的rip
						 "movq %%rsp, %%r11 \n\t"						   // 再次执行的rsp
						 "sysenter \n\t"								   // 进入内核层，跳转到entry.S的system_call
						 "sysexit_return_address2:  \n\t"
						 "xchgq %%rdx, %%r10 \n\t"
						 "xchgq %%rcx, %%r11 \n\t"
						 "popq %%r11 \n\t"
						 "popq %%r10 \n\t"
						 : "=a"(errno), "+D"(fd)
						 : "0"(__NR_write), "S"(str), "d"(4)
						 : "memory");

	// call sys_lseek
	// long lseek(int fildes, void* buf, long nbyte);
	__asm__ __volatile__("pushq %%r10 \n\t"
						 "pushq %%r11 \n\t"
						 "leaq sysexit_return_address22(%%rip), %%r10 \n\t" // 再次执行的rip
						 "movq %%rsp, %%r11 \n\t"							// 再次执行的rsp
						 "sysenter \n\t"									// 进入内核层，跳转到entry.S的system_call
						 "sysexit_return_address22:  \n\t"
						 "xchgq %%rdx, %%r10 \n\t"
						 "xchgq %%rcx, %%r11 \n\t"
						 "popq %%r11 \n\t"
						 "popq %%r10 \n\t"
						 : "=a"(errno), "+D"(fd)
						 : "0"(__NR_lseek), "S"(0), "d"(SEEK_SET)
						 : "memory");

	// call sys_read
	// long read(int fildes, void* buf, long nbyte);
	__asm__ __volatile__("pushq %%r10 \n\t"
						 "pushq %%r11 \n\t"
						 "leaq sysexit_return_address21(%%rip), %%r10 \n\t" // 再次执行的rip
						 "movq %%rsp, %%r11 \n\t"							// 再次执行的rsp
						 "sysenter \n\t"									// 进入内核层，跳转到entry.S的system_call
						 "sysexit_return_address21:  \n\t"
						 "xchgq %%rdx, %%r10 \n\t"
						 "xchgq %%rcx, %%r11 \n\t"
						 "popq %%r11 \n\t"
						 "popq %%r10 \n\t"
						 : "=a"(errno), "+D"(fd)
						 : "0"(__NR_read), "S"(buf), "d"(50)
						 : "memory");
	// call sys_close
	// int close(int fildes);
	__asm__ __volatile__("pushq %%r10 \n\t"
						 "pushq %%r11 \n\t"
						 "leaq sysexit_return_address1(%%rip), %%r10 \n\t" // 再次执行的rip
						 "movq %%rsp, %%r11 \n\t"						   // 再次执行的rsp
						 "sysenter \n\t"								   // 进入内核层，跳转到entry.S的system_call
						 "sysexit_return_address1:  \n\t"
						 "xchgq %%rdx, %%r10 \n\t"
						 "xchgq %%rcx, %%r11 \n\t"
						 "popq %%r11 \n\t"
						 "popq %%r10 \n\t"
						 : "=a"(errno), "+D"(fd)
						 : "0"(__NR_close)
						 : "memory");
	/////////////////////////////////////////////////////////////////////////////////

	// 不知道为什么，这里不能打印
	// color_printk(RED, BLACK, "user_level_function task called sysenter,errno:%d\n", errno);
	while (1)
		;
}

// 内核线程，该线程会进入特权级3
// 在init进程的执行过程中，init进程会放弃内核线程的身份，将自己修改为普通进程
// 尽管init进程此刻还没有实体程序，但伴随do_execve函数的执行结束，init进程将作为一个全新的个体运行与操作系统
// 新的程序位于文件系统根目录下，名为init.bin
unsigned long init(unsigned long arg)
{
	// struct pt_regs *regs; // 这里破坏了中断栈
	DISK1_FAT32_FS_init();
 	DEBUGK("init task is running, arg:%#018lx\n", arg);
	// color_printk(RED, BLACK, "init task is running, arg:%#018lx\n", arg);

	// sys_open("/The quick brown.fox", O_CREAT);

	current->thread->rip = (unsigned long)ret_system_call;
	current->thread->rsp = (unsigned long)current + STACK_SIZE - sizeof(struct pt_regs);
	current->thread->gs = USER_DS;
	current->thread->fs = USER_DS;
	current->flags &= ~PF_KTHREAD;

	//while(1);

	// 更换rsp到中断栈, PCB最上部的需要pop返回的位置
	// 压入了ret_system_call作为返回地址
	__asm__ __volatile("movq %1, %%rsp \n\t"
					   "pushq %2  \n\t"
					   "jmp do_execve \n\t" ::"D"(current->thread->rsp),
					   "m"(current->thread->rsp), "m"(current->thread->rip), 
					   "S"("/init.bin"),"d"(NULL),"c"(NULL)
					   : "memory");
	return 1;
}

// ------------------DEBUG----------------------
extern int usr_init();
// 被init调用,加载用户进程体，到用户空间800000
unsigned long shell_execve(struct pt_regs *regs, char *name)
{
	unsigned long retval = 0;

	// 这里没有初始化gs,fs段选择子
	regs->ds = USER_DS;
	regs->es = USER_DS;
	regs->ss = USER_DS;
	regs->cs = USER_CS;
	// regs->rip = new_rip;
	// regs->rsp = new_rsp;
	//  在中断栈中填入地址

	int (*fn)(void) = usr_init;
	regs->r10 = (unsigned long)fn;				// RIP
	void *tmp = kmalloc(1048576, 0);			// 申请1MB
	regs->r11 = ((unsigned long)tmp) + 1048576; // RSP
	regs->rax = 0;

	// 清理地址空间脏数据
	// go to ret_system_call
	return retval;
}

unsigned long shell_boot(unsigned long arg)
{
	current->thread->rip = (unsigned long)ret_system_call;
	current->thread->rsp = (unsigned long)current + STACK_SIZE - sizeof(struct pt_regs);
	current->thread->gs = USER_DS;
	current->thread->fs = USER_DS;
	current->flags &= ~PF_KTHREAD;

	// 更换rsp到中断栈, PCB最上部的需要pop返回的位置
	// 压入了ret_system_call作为返回地址
	__asm__ __volatile("movq %1, %%rsp \n\t"
					   "pushq %2  \n\t"
					   "jmp shell_execve \n\t" ::"D"(current->thread->rsp),
					   "m"(current->thread->rsp), "m"(current->thread->rip) : "memory");

	return 1;
}
//--------------------DEBUG----------------------

// 创建内核线程的PCB
unsigned long do_fork_old(struct pt_regs *regs, unsigned long clone_flags, unsigned long stack_start, unsigned long stack_size)
{
	struct task_struct *tsk = NULL;
	struct thread_struct *thd = NULL;
	struct Page *p = NULL;

	// 为创建的进程的PCB申请内存
	// 此处既然是为了创建PCB, 目前内存系统已经完善，为何不使用kmalloc()函数申请数据呢 ？
	p = alloc_pages(ZONE_NORMAL, 1, PG_PTable_Maped | PG_Active | PG_Kernel);

	tsk = (struct task_struct *)Phy_To_Virt(p->PHY_address);
	color_printk(WHITE, BLACK, "struct task_struct address:%#018lx\n", (unsigned long)tsk);

	memset(tsk, 0, sizeof(*tsk));
	*tsk = *current; // 复制了当前进程的结构体

	// 修改一定的参数
	list_init(&tsk->list);
	tsk->priority = 2;
	tsk->preempt_count = 0;
	tsk->pid++;
	tsk->state = TASK_UNINTERRUPTIBLE;
	// 设置线程栈，初始化线程栈
	thd = (struct thread_struct *)(tsk + 1);
	tsk->thread = thd;
	// 把准备好的中断栈拷贝到进程PCB内存的上部, 这里是中断栈实体
	memcpy(regs, (void *)((unsigned long)tsk + STACK_SIZE - sizeof(struct pt_regs)), sizeof(struct pt_regs));

	// 这里是描述中断栈的结构体
	thd->rsp0 = (unsigned long)tsk + STACK_SIZE;						 // 使得rsp0指向PCB内存的最高处
	thd->rip = regs->rip;												 // 使得中断栈和中断栈结构体中记录的rip相等，都是 = kernel_thread_func
	thd->rsp = (unsigned long)tsk + STACK_SIZE - sizeof(struct pt_regs); // 使得rsp0 指向PCB中最上端，中断栈的最顶部，可从此处退出中断栈
	thd->fs = KERNEL_DS;
	thd->gs = KERNEL_DS;

	// 不属于内核程序,则从系统中断返回
	if (!(tsk->flags & PF_KTHREAD))
		thd->rip = regs->rip = (unsigned long)ret_system_call;

	tsk->state = TASK_RUNNING;

	insert_task_queue(tsk);

	return 1;
}

void switch_mm(struct task_struct *prev, struct task_struct *next)
{
	__asm__ __volatile__("movq %0, %%cr3 \n\t" ::"r"(next->mm->pgd)
						 : "memory");
}

/**
 * @brief 把子进程插入到准备就绪队列中
 *
 * @param tsk CPCB = Child Process control block
 */
void wakeup_process(struct task_struct *tsk)
{
	tsk->state = TASK_RUNNING;
	insert_task_queue(tsk);
	current->flags |= NEED_SCHEDULE;
}

/**
 * @brief 复制进程标志
 *
 * @param clone_flags
 * @param tsk
 * @return unsigned long
 */
unsigned long copy_flags(unsigned long clone_flags, struct task_struct *tsk)
{
	// 如果子进程要与父进程共享内存空间，那么设置子进程标志PF_VFORK
	if (clone_flags & CLONE_VM)
		tsk->flags |= PF_VFORK;
	return 0;
}

/**
 * @brief 复制进程的文件描述符
 *
 * @param clone_flags
 * @param tsk CPCB
 * @return unsigned long
 */
unsigned long copy_files(unsigned long clone_flags, struct task_struct *tsk)
{
	int error = 0;
	int i = 0;
	if (clone_flags & CLONE_FS)
		goto out;
	for (; i < TASK_FILE_MAX; i++)
		if (current->file_struct[i] != NULL)
		{
			tsk->file_struct[i] = (struct file *)kmalloc(sizeof(struct file), 0);
			memcpy(current->file_struct[i], tsk->file_struct[i], sizeof(struct file));
		}
out:
	return error;
}

/**
 * @brief 回收进程已经打开的文件
 *		整个回收过程将包括文件描述符结构体的释放和文件描述符指针数组
 * @param tsk
 */
void exit_files(struct task_struct *tsk)
{
	int i = 0;
	if (tsk->flags & PF_VFORK)
		;
	else
		for (; i < TASK_FILE_MAX; i++)
			if (tsk->file_struct[i] != NULL)
				kfree(tsk->file_struct[i]);
	memset(tsk->file_struct, 0, sizeof(struct file *) * TASK_FILE_MAX);
	// clear current->file_struct
}

/**
 * @brief 给子进程创建出运行空间，创建出页表
 *
 * @param clone_flags
 * @param tsk 子进程PCB
 * @return unsigned long
 */
unsigned long copy_mm(unsigned long clone_flags, struct task_struct *tsk)
{
	int error = 0;
	struct mm_struct *newmm = NULL;
	unsigned long code_start_addr = 0x800000;
	unsigned long stack_start_addr = 0xa00000;
	unsigned long brk_start_addr = 0xc00000;
	unsigned long *tmp;
	unsigned long *virtual = NULL;
	struct Page *p = NULL;
	if (clone_flags & CLONE_VM)
	{
		newmm = current->mm;
		goto out;
	}

	newmm = (struct mm_struct *)kmalloc(sizeof(struct mm_struct), 0);
	memcpy(current->mm, newmm, sizeof(struct mm_struct));

	// copy kernel space, 创建了PML4页表
	newmm->pgd = (pml4t_t *)Virt_To_Phy(kmalloc(PAGE_4K_SIZE, 0));
	memcpy(Phy_To_Virt(init_task[0]->mm->pgd) + 256, Phy_To_Virt(newmm->pgd) + 256, PAGE_4K_SIZE / 2);
	memset(Phy_To_Virt(newmm->pgd), 0, PAGE_4K_SIZE / 2); // clear user memory space

	// copy user code / data / bss / space
	// 申请PDPT内存，填充PML4页表项
	tmp = Phy_To_Virt((unsigned long *)((unsigned long)newmm->pgd & (~0xfffUL)) + ((code_start_addr >> PAGE_GDT_SHIFT) & 0x1ff));
	virtual = kmalloc(PAGE_4K_SIZE, 0);
	memset(virtual, 0, PAGE_4K_SIZE);
	set_pdpt(tmp, mk_pdpt(Virt_To_Phy(virtual), PAGE_USER_Dir));

	// 申请PDT内存，填充PDPT页表项
	tmp = Phy_To_Virt((unsigned long *)(*tmp & (~0xfffUL)) + ((code_start_addr >> PAGE_1G_SHIFT) & 0x1ff));
	virtual = kmalloc(PAGE_4K_SIZE, 0);
	memset(virtual, 0, PAGE_4K_SIZE);
	set_pdpt(tmp, mk_pdpt(Virt_To_Phy(virtual), PAGE_USER_Dir));

	// 申请用户占用的内存,填充页表, 填充PDT内存
	tmp = Phy_To_Virt((unsigned long *)(*tmp & (~0xfffUL)) + ((code_start_addr >> PAGE_2M_SHIFT) & 0x1ff));
	p = alloc_pages(ZONE_NORMAL, 1, PG_PTable_Maped);
	set_pdt(tmp, mk_pdt(p->PHY_address, PAGE_USER_Page));

	// 拷贝代码、数据
	memcpy((void *)code_start_addr, Phy_To_Virt(p->PHY_address), stack_start_addr - code_start_addr);

	// copy user brk space 拷贝用户空间的堆内存
	if (current->mm->end_brk - current->mm->start_brk != 0)
	{
		tmp = Phy_To_Virt((unsigned long *)((unsigned long)newmm->pgd & (~0xfffUL)) + ((brk_start_addr >> PAGE_GDT_SHIFT) & 0x1ff));
		tmp = Phy_To_Virt((unsigned long *)(*tmp & (~0xfffUL)) + ((brk_start_addr >> PAGE_1G_SHIFT) & 0x1ff));
		tmp = Phy_To_Virt((unsigned long *)(*tmp & (~0xfffUL)) + ((brk_start_addr >> PAGE_2M_SHIFT) & 0x1ff));
		p = alloc_pages(ZONE_NORMAL, 1, PG_PTable_Maped);
		set_pdt(tmp, mk_pdt(p->PHY_address, PAGE_USER_Page));

		memcpy((void *)brk_start_addr, Phy_To_Virt(p->PHY_address), PAGE_2M_SIZE);
	}
out:
	tsk->mm = newmm;
	return error;
}

/**
 * @brief 释放内存空间结构体，
 * 		exit_mm()与copy_mm()的分配空间分布结构体初始化过程相逆
 * @param tsk
 */
void exit_mm(struct task_struct *tsk)
{
	unsigned long code_start_addr = 0x800000;
	unsigned long *tmp4, *tmp3, *tmp2;

	if (tsk->flags & PF_VFORK)
		return;

	if (tsk->mm->pgd != NULL)
	{ // PDPT内存
		tmp4 = Phy_To_Virt((unsigned long *)((unsigned long)tsk->mm->pgd & (~0xfffUL)) + ((code_start_addr >> PAGE_GDT_SHIFT) & 0x1ff));
		// PDT内存
		tmp3 = Phy_To_Virt((unsigned long *)(*tmp4 & (~0xfffUL)) + ((code_start_addr >> PAGE_1G_SHIFT) & 0x1ff));
		// 用户进程占用的内存
		tmp2 = Phy_To_Virt((unsigned long *)(*tmp3 & (~0xfffUL)) + ((code_start_addr >> PAGE_2M_SHIFT) & 0x1ff));

		// 这里明显写的太不标准，你怎么确定用户进程只占用2MB内存的
		free_pages(Phy_to_2M_Page(*tmp2), 1);
		kfree(Phy_To_Virt(*tmp3));
		kfree(Phy_To_Virt(*tmp4));
		kfree(Phy_To_Virt(tsk->mm->pgd));
	}

	if (tsk->mm != NULL)
		kfree(tsk->mm);
}

// 为子进程伪造应用层执行现场
unsigned long copy_thread(unsigned long clone_flags, unsigned long stack_start, unsigned long stack_size, struct task_struct *tsk, struct pt_regs *regs)
{
	struct thread_struct *thd = NULL;
	struct pt_regs *childregs = NULL; // 应用层执行现场结构体

	// 开辟中断栈，
	thd = (struct thread_struct *)(tsk + 1);
	memset(thd, 0, sizeof(*thd));
	tsk->thread = thd;

	// 给新进程复制上下文环境到中断栈内
	childregs = (struct pt_regs *)((unsigned long)tsk + STACK_SIZE) - 1;
	memcpy(regs, childregs, sizeof(struct pt_regs));

	childregs->rax = 0;			  // fork给子进程，返回值为0
	childregs->rsp = stack_start; // 设置子进程的应用层栈指针 ？为什么这里设置为0?

	// 中断栈中的rsp会在新进程从内核中启动运行的时候用到
	// rsp0用于新进程进入内核用到的栈
	thd->rsp0 = (unsigned long)tsk + STACK_SIZE;
	thd->rsp = (unsigned long)childregs;
	thd->fs = current->thread->fs;
	thd->gs = current->thread->gs;

	if (tsk->flags & PF_KTHREAD)
		thd->rip = (unsigned long)kernel_thread_func;
	else
		thd->rip = (unsigned long)ret_system_call;

	// color_printk(WHITE, BLACK, "current user ret addr:%#018lx, rsp:%#018lx\n", regs->r10, regs->r11);
	// color_printk(WHITE, BLACK, "new user ret addr:%#018lx, rsp:%#018lx\n", childregs->r10, childregs->r11);

	return 0;
}

void exit_thread(struct task_struct *tsk) {}
/**
 * @brief a.do_fork函数会先为PCB分配存储空间并对其进行初步赋值
 *		  b. 根据clone_flags参数进行克隆或共享工作
 * @param regs
 * @param clone_flags 克隆信息
 * @param stack_start
 * @param stack_size
 * @return unsigned long 为父进程返回子进程的ID号
 */
unsigned long do_fork(struct pt_regs *regs, unsigned long clone_flags, unsigned long stack_start, unsigned long stack_size)
{
	int retval = 0;
	struct task_struct *tsk = NULL;

	// alloc & copy task struct
	tsk = (struct task_struct *)kmalloc(STACK_SIZE, 0);
	// color_printk(WHITE, BLACK, "struct_task address:%#018lx\n", (unsigned long)tsk);
	if (tsk == NULL)
	{
		retval = -EAGAIN;
		goto alloc_copy_task_fail;
	}
	memset(tsk, 0, sizeof(*tsk));
	memcpy(current, tsk, sizeof(struct task_struct));
	list_init(&tsk->list);
	wait_queue_init(&tsk->wait_childexit, current);
	
	tsk->exit_code = 0;
	tsk->priority = 2;
	tsk->pid = global_pid++;
	tsk->preempt_count = 0; // 进程抢占计数值初始化
	
	// 拷贝信号
	tsk->sigaction = (sigaction_T*)kmalloc(sizeof(sigaction_T) * (NSIG + 1), 0);
	memcpy(current->sigaction, tsk->sigaction, sizeof(sigaction_T) * (NSIG + 1));

	tsk->state = TASK_UNINTERRUPTIBLE;
	//////头插法
	tsk->next = init_task_union.task.next;
	init_task_union.task.next = tsk;
	//////
	tsk->parent = current;
	retval = -ENOMEM;

	// copy flags
	if (copy_flags(clone_flags, tsk))
		goto copy_flags_fail;
	// copy mm struct
	if (copy_mm(clone_flags, tsk))
		goto copy_mm_fail;
	// copy file struct
	if (copy_files(clone_flags, tsk))
		goto copy_files_fail;
	// copy thread struct
	if (copy_thread(clone_flags, stack_start, stack_size, tsk, regs))
		goto copy_thread_fail;
	retval = tsk->pid;
	wakeup_process(tsk);

fork_ok:
	return retval;

copy_thread_fail:
	exit_thread(tsk);
copy_files_fail:
	exit_files(tsk);
copy_mm_fail:
	exit_mm(tsk);
copy_flags_fail:
alloc_copy_task_fail:
	kfree(tsk);
	return retval;
}

void exit_notify(void)
{
	wakeup(&current->parent->wait_childexit, TASK_INTERRUPTIBLE);
}

unsigned long do_exit(unsigned long exit_code)
{
	struct task_struct *tsk = current;
	// color_printk(RED, BLACK, "exit task is running,arg:%#018lx\n", exit_code);
	

	cli();
	tsk->state = TASK_ZOMBIE;
	tsk->exit_code = exit_code;
	exit_thread(tsk);
	exit_files(tsk);
	sti();

do_exit_again:
	
	exit_notify();
	schedule();
	goto do_exit_again;

	return 0;
}

// 线程承载的函数，参数，标志
// kernel_thread给进程创建了寄存器环境
int kernel_thread(unsigned long (*fn)(unsigned long), unsigned long arg, unsigned long flags)
{ // 设置中断栈
	struct pt_regs regs;
	memset(&regs, 0, sizeof(regs));

	regs.rbx = (unsigned long)fn;  // RBX保存着程序的入口地址
	regs.rdx = (unsigned long)arg; // RDX保存着进程创建者传入的参数

	regs.ds = KERNEL_DS;
	regs.es = KERNEL_DS;
	regs.cs = KERNEL_CS;
	regs.ss = KERNEL_DS;
	regs.rflags = (1 << 9); // 设置可中断标志

	// rip 保存着进程引导程序, 在内核栈中设置这个rip,的意义是什么
	regs.rip = (unsigned long)kernel_thread_func;

	return do_fork(&regs, flags | CLONE_VM, 0, 0);
}

// 被switch_to宏调用, 用来切换任务
void __switch_to(struct task_struct *prev, struct task_struct *next)
{
	// 切换进程的TSS 和 数据段选择子
	init_tss[0].rsp0 = next->thread->rsp0;

	// 这样下一次中断就会使用next进程中断栈
	set_tss64(init_tss[0].rsp0, init_tss[0].rsp1, init_tss[0].rsp2, init_tss[0].ist1, init_tss[0].ist2, init_tss[0].ist3, init_tss[0].ist4, init_tss[0].ist5, init_tss[0].ist6, init_tss[0].ist7);

	__asm__ __volatile__("movq	%%fs,	%0 \n\t"
						 : "=a"(prev->thread->fs));
	__asm__ __volatile__("movq	%%gs,	%0 \n\t"
						 : "=a"(prev->thread->gs));

	__asm__ __volatile__("movq	%0,	%%fs \n\t" ::"a"(next->thread->fs));
	__asm__ __volatile__("movq	%0,	%%gs \n\t" ::"a"(next->thread->gs));

	// 改变下一个进程要使用的系统调用栈
	wrmsr(0x175, next->thread->rsp0);

	// color_printk(WHITE, BLACK, "prev->thread->rsp0:%#018lx\n", prev->thread->rsp0);
	// color_printk(WHITE, BLACK, "next->thread->rsp0:%#018lx\n", next->thread->rsp0);

	// go to kernel_thread_func
}

// 任务初始化
void task_init()
{
	unsigned long *tmp = NULL;
	unsigned long *vaddr = NULL;
	int i = 0;

	vaddr = Phy_To_Virt((unsigned long)Get_gdt() & (~0xfffUL));

	// 内核层空间占用顶层页表的256个页表项
	for (i = 256; i < 512; i++)
	{
		tmp = vaddr + i;
		if (*tmp == 0)
		{
			unsigned long *virtual = kmalloc(PAGE_4K_SIZE, 0);
			memset(virtual, 0, PAGE_4K_SIZE);
			set_mpl4t(tmp, mk_mpl4t(Virt_To_Phy(virtual), PAGE_KERNEL_GDT));
			// set_mpl4t(tmp, mk_mpl4t(Virt_To_Phy(virtual), PAGE_USER_GDT));
		}
	}

	// 初始化内核进程的内存结构
	init_mm.pgd = (pml4t_t *)Get_gdt();

	init_mm.start_code = memory_management_struct.start_code;
	init_mm.end_code = memory_management_struct.end_code;

	init_mm.start_data = (unsigned long)&_data;
	init_mm.end_data = memory_management_struct.end_data;

	init_mm.start_rodata = (unsigned long)&_rodata;
	init_mm.end_rodata = (unsigned long)&_erodata;

	init_mm.start_brk = (unsigned long)&_bss;
	init_mm.end_brk = (unsigned long)&_ebss;

	init_mm.start_brk = memory_management_struct.start_brk;
	init_mm.end_brk = current->addr_limit;

	init_mm.start_stack = _stack_start;

	// sysenter指令：从特权级3跳转到特权级0，sysexit指令从0特权级跳转到3特权级
	// RDX中保存RIP, RCX中保存RSP, 这是sysexit需要的
	// 把IA32_SYSENTER_CS寄存器设置为段选择子, 这个参数是sysexit/sysenter指令需要的
	wrmsr(0x174, KERNEL_CS);

	// sysenter
	wrmsr(0x175, current->thread->rsp0);	  // sysenter需要使用的rsp
	wrmsr(0x176, (unsigned long)system_call); // sysenter进入系统调用后，会去执行system_call,sysenter要使用的rip

	//	init_thread,init_tss
	set_tss64(init_thread.rsp0, init_tss[0].rsp1, init_tss[0].rsp2, init_tss[0].ist1, init_tss[0].ist2, init_tss[0].ist3, init_tss[0].ist4, init_tss[0].ist5, init_tss[0].ist6, init_tss[0].ist7);
	init_tss[0].rsp0 = init_thread.rsp0;

	list_init(&init_task_union.task.list);
	list_init(&init_task_union.task.wait_childexit.wait_list);

	init_task_union.task.sigaction = (sigaction_T*)kmalloc(sizeof(sigaction_T) * (NSIG + 1), 0);

	// 创建内核线程
	kernel_thread(init, 13, CLONE_FS | CLONE_SIGNAL);

	init_task_union.task.state = TASK_RUNNING;
	init_task_union.task.preempt_count = 0;
}


