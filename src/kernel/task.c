#include "toolkit.h"
#include "mmkit.h"
#include "syskit.h"
#include "kernelkit.h"

union task_union init_task_union
	__attribute__((__section__(".data.init_task"))) = {INIT_TASK(init_task_union.task)};

task_t *init_task[NR_CPUS] = {&init_task_union.task, 0};

struct mm_struct init_mm = {0};

struct thread_struct init_thread =
	{
		.rsp0 = (u64_t)(init_task_union.stack + STACK_SIZE / sizeof(u64_t)),
		.rsp = (u64_t)(init_task_union.stack + STACK_SIZE / sizeof(u64_t)),
		.fs = KERNEL_DS,
		.gs = KERNEL_DS,
		.cr2 = 0,
		.trap_nr = 0,
		.error_code = 0};

struct tss_struct init_tss[NR_CPUS] = {[0 ... NR_CPUS - 1] = INIT_TSS};
task_t *my_cur;
extern void ret_system_call(void);	  // 进入特权级3
extern void kernel_thread_func(void); // 进入用户进程，在执行完用户进程后，会执行do_exit()程序
extern void system_call(void);

u64_t shell_boot(u64_t arg);
s64_t global_pid;

task_t *get_task(s64_t pid)
{
	task_t *tsk = nullptr;

	for (tsk = init_task_union.task.next; tsk != &init_task_union.task; tsk = tsk->next)
	{
		if (tsk->pid == pid)
			return tsk;
	}

	return nullptr;
}

// 内核线程，该线程会进入特权级3
// 在init进程的执行过程中，init进程会放弃内核线程的身份，将自己修改为普通进程
// 尽管init进程此刻还没有实体程序，但伴随do_execve函数的执行结束，init进程将作为一个全新的个体运行与操作系统
// 新的程序位于文件系统根目录下，名为init.bin
u64_t init(u64_t arg)
{
	// pt_regs_t *regs; // 这里破坏了中断栈
	DISK1_FAT32_FS_init();
 	DEBUGK("init task is running, arg:%#018lx\n", arg);
	// color_printk(RED, BLACK, "init task is running, arg:%#018lx\n", arg);

	// sys_open("/The quick brown.fox", O_CREAT);

	current->thread->rip = (u64_t)ret_system_call;
	current->thread->rsp = (u64_t)current + STACK_SIZE - sizeof(pt_regs_t);
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
					   "S"("/init.bin"),"d"(nullptr),"c"(nullptr)
					   : "memory");
	return 1;
}

// ------------------DEBUG----------------------
extern s32_t usr_init();
// 被init调用,加载用户进程体，到用户空间800000
u64_t shell_execve(pt_regs_t *regs, str_t name)
{
	u64_t retval = 0;

	// 这里没有初始化gs,fs段选择子
	regs->ds = USER_DS;
	regs->es = USER_DS;
	regs->ss = USER_DS;
	regs->cs = USER_CS;
	// regs->rip = new_rip;
	// regs->rsp = new_rsp;
	//  在中断栈中填入地址

	s32_t (*fn)(void) = usr_init;
	regs->r10 = (u64_t)fn;				// RIP
	void *tmp = knew(1048576, 0);			// 申请1MB
	regs->r11 = ((u64_t)tmp) + 1048576; // RSP
	regs->rax = 0;

	// 清理地址空间脏数据
	// go to ret_system_call
	return retval;
}

u64_t shell_boot(u64_t arg)
{
	current->thread->rip = (u64_t)ret_system_call;
	current->thread->rsp = (u64_t)current + STACK_SIZE - sizeof(pt_regs_t);
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

void switch_mm(task_t *prev, task_t *next)
{
	__asm__ __volatile__("movq %0, %%cr3 \n\t" ::"r"(next->mm->pgd)
						 : "memory");
}

/**
 * @brief 把子进程插入到准备就绪队列中
 *
 * @param tsk CPCB = Child Process control block
 */
void wakeup_process(task_t *tsk)
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
 * @return u64_t
 */
u64_t copy_flags(u64_t clone_flags, task_t *tsk)
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
 * @return u64_t
 */
u64_t copy_files(u64_t clone_flags, task_t *tsk)
{
	s32_t error = 0;
	s32_t i = 0;
	if (clone_flags & CLONE_FS)
		goto out;
	for (; i < TASK_FILE_MAX; i++)
		if (current->file_struct[i] != nullptr)
		{
			tsk->file_struct[i] = (file_t *)knew(sizeof(file_t), 0);
			memcpy(current->file_struct[i], tsk->file_struct[i], sizeof(file_t));
		}
out:
	return error;
}

/**
 * @brief 回收进程已经打开的文件
 *		整个回收过程将包括文件描述符结构体的释放和文件描述符指针数组
 * @param tsk
 */
void exit_files(task_t *tsk)
{
	s32_t i = 0;
	if (tsk->flags & PF_VFORK)
		;
	else
		for (; i < TASK_FILE_MAX; i++)
			if (tsk->file_struct[i] != nullptr)
				kdelete(tsk->file_struct[i], sizeof(file_t));
	memset(tsk->file_struct, 0, sizeof(file_t *) * TASK_FILE_MAX);
	// clear current->file_struct
}

/**
 * @brief 给子进程创建出运行空间，创建出页表
 *
 * @param clone_flags
 * @param tsk 子进程PCB
 * @return u64_t
 */
/*
u64_t copy_mm(u64_t clone_flags, task_t *tsk)
{
	s32_t error = 0;
	struct mm_struct *newmm = nullptr;
	u64_t code_start_addr = 0x800000;
	u64_t stack_start_addr = 0xa00000;
	u64_t brk_start_addr = 0xc00000;
	u64_t *tmp;
	u64_t *virtual = nullptr;
	struct Page *p = nullptr;
	if (clone_flags & CLONE_VM)
	{
		newmm = current->mm;
		goto out;
	}

	newmm = (struct mm_struct *)knew(sizeof(struct mm_struct), 0);
	memcpy(current->mm, newmm, sizeof(struct mm_struct));

	// copy kernel space, 创建了PML4页表
	newmm->pgd = (pml4t_t *)Virt_To_Phy(knew(PAGE_4K_SIZE, 0));
	memcpy(Phy_To_Virt(init_task[0]->mm->pgd) + 256, Phy_To_Virt(newmm->pgd) + 256, PAGE_4K_SIZE / 2);
	memset(Phy_To_Virt(newmm->pgd), 0, PAGE_4K_SIZE / 2); // clear user memory space

	// copy user code / data / bss / space
	// 申请PDPT内存，填充PML4页表项
	tmp = Phy_To_Virt((u64_t *)((u64_t)newmm->pgd & (~0xfffUL)) + ((code_start_addr >> PAGE_GDT_SHIFT) & 0x1ff));
	virtual = knew(PAGE_4K_SIZE, 0);
	memset(virtual, 0, PAGE_4K_SIZE);
	set_pdpt(tmp, mk_pdpt(Virt_To_Phy(virtual), PAGE_USER_Dir));

	// 申请PDT内存，填充PDPT页表项
	tmp = Phy_To_Virt((u64_t *)(*tmp & (~0xfffUL)) + ((code_start_addr >> PAGE_1G_SHIFT) & 0x1ff));
	virtual = knew(PAGE_4K_SIZE, 0);
	memset(virtual, 0, PAGE_4K_SIZE);
	set_pdpt(tmp, mk_pdpt(Virt_To_Phy(virtual), PAGE_USER_Dir));

	// 申请用户占用的内存,填充页表, 填充PDT内存
	tmp = Phy_To_Virt((u64_t *)(*tmp & (~0xfffUL)) + ((code_start_addr >> PAGE_2M_SHIFT) & 0x1ff));
	p = alloc_pages(ZONE_NORMAL, 1, PG_PTable_Maped);
	set_pdt(tmp, mk_pdt(p->PHY_address, PAGE_USER_Page));

	// 拷贝代码、数据
	memcpy((void *)code_start_addr, Phy_To_Virt(p->PHY_address), stack_start_addr - code_start_addr);

	// copy user brk space 拷贝用户空间的堆内存
	if (current->mm->end_brk - current->mm->start_brk != 0)
	{
		tmp = Phy_To_Virt((u64_t *)((u64_t)newmm->pgd & (~0xfffUL)) + ((brk_start_addr >> PAGE_GDT_SHIFT) & 0x1ff));
		tmp = Phy_To_Virt((u64_t *)(*tmp & (~0xfffUL)) + ((brk_start_addr >> PAGE_1G_SHIFT) & 0x1ff));
		tmp = Phy_To_Virt((u64_t *)(*tmp & (~0xfffUL)) + ((brk_start_addr >> PAGE_2M_SHIFT) & 0x1ff));
		p = alloc_pages(ZONE_NORMAL, 1, PG_PTable_Maped);
		set_pdt(tmp, mk_pdt(p->PHY_address, PAGE_USER_Page));

		memcpy((void *)brk_start_addr, Phy_To_Virt(p->PHY_address), PAGE_2M_SIZE);
	}
out:
	tsk->mm = newmm;
	return error;
} */


static void copy_pageTables(struct mm_struct* newmm, u64_t addr){
	
	// here is only copy Page Table. 
	// when page_fault occur, allcot memory and copy user data
	u64_t *tmp = nullptr, *virtual = nullptr, *parent_tmp;
	u64_t attr = 0;
	struct Page *p = nullptr;

	// alter page directory entry of parent_process(current_process)
	// here requires atomic execution
	parent_tmp = pde_ptr(addr);
	p = (memory_management_struct.pages_struct + (*parent_tmp  >> PAGE_2M_SHIFT));
	assert(p->PHY_address == (*parent_tmp & PAGE_2M_MASK))
	if(p->PHY_address == 0)
		return;


	// 申请PDPT内存，填充PML4页表项 for child_process
	tmp = Phy_To_Virt((u64_t *)((u64_t)newmm->pgd & (~0xfffUL)) + ((addr >> PAGE_GDT_SHIFT) & 0x1ff));
	if(!(*tmp & PAGE_Present)) {
		virtual = knew(PAGE_4K_SIZE, 0);
		memset(virtual, 0, PAGE_4K_SIZE);
		set_pdpt(tmp, mk_pdpt(Virt_To_Phy(virtual), PAGE_USER_Dir));
	}

	// 申请PDT内存，填充PDPT页表项 for child_process
	tmp = Phy_To_Virt((u64_t *)(*tmp & (~0xfffUL)) + ((addr >> PAGE_1G_SHIFT) & 0x1ff));
	if(!(*tmp & PAGE_Present)) {
		virtual = knew(PAGE_4K_SIZE, 0);
		memset(virtual, 0, PAGE_4K_SIZE);
		set_pdpt(tmp, mk_pdpt(Virt_To_Phy(virtual), PAGE_USER_Dir));
	}

	// 填充 PDT 页表项 for child_process
	tmp = Phy_To_Virt((u64_t *)(*tmp & (~0xfffUL)) + ((addr >> PAGE_2M_SHIFT) & 0x1ff));
	if(!(*tmp & PAGE_Present)) {

		attr = (*parent_tmp & (0xfffUL)); // get parent privilege
		
		attr = (attr & (~PAGE_R_W)); // delet PW right
		// set parent and child's Page privilege, parent and child share the Page
		set_pdt(tmp, mk_pdt(p->PHY_address, attr));
		set_pdt(parent_tmp, mk_pdt(p->PHY_address, attr));
		// add Page_struct count
		p->reference_count++;
	}
	return;
}

u64_t copy_mm_fork(u64_t clone_flags, task_t *tsk)
{
	s32_t error = 0;
	struct mm_struct *newmm = nullptr;
	if (clone_flags & CLONE_VM) {
		newmm = current->mm;
		goto out;
	}

	newmm = (struct mm_struct *)knew(sizeof(struct mm_struct), 0);
	memcpy(current->mm, newmm, sizeof(struct mm_struct));

	// copy kernel space, 创建了PML4页表
	newmm->pgd = (pml4t_t *)Virt_To_Phy(knew(PAGE_4K_SIZE, 0));
	memcpy(Phy_To_Virt(init_task[0]->mm->pgd) + 256, Phy_To_Virt(newmm->pgd) + 256, PAGE_4K_SIZE / 2);
	memset(Phy_To_Virt(newmm->pgd), 0, PAGE_4K_SIZE / 2); // clear user memory space

	u64_t vaddr = 0;
	
	for(vaddr = newmm->start_code; vaddr < newmm->end_code; vaddr += PAGE_2M_SIZE) // 代码段
		copy_pageTables(newmm, vaddr);
	for(vaddr = newmm->start_data; vaddr < newmm->end_data; vaddr += PAGE_2M_SIZE) // 数据
		copy_pageTables(newmm, vaddr);
	for(vaddr = newmm->start_rodata; vaddr < newmm->end_rodata; vaddr += PAGE_2M_SIZE) // 只读数据
		copy_pageTables(newmm, vaddr);
	for(vaddr = newmm->start_bss; vaddr < newmm->end_bss; vaddr += PAGE_2M_SIZE) // bss段
		copy_pageTables(newmm, vaddr);
	for(vaddr = newmm->start_brk; vaddr < newmm->end_brk; vaddr += PAGE_2M_SIZE) // brk 段
		copy_pageTables(newmm, vaddr);
	
	// 栈段, 这里的栈段共享不成。
	// 父进程返回用户态后，会破坏用户栈，但竟没有触发保护异常。
	// 没有触发异常，导致子进程运行失败.
	// 为什么会出现这种情况 ？ 

	// 24-7-17-10:16: 因为你没有刷新快表
	for(vaddr = newmm->start_stack; vaddr < (newmm->start_stack + newmm->stack_length); vaddr += PAGE_2M_SIZE)
		copy_pageTables(newmm, vaddr);

	// 刷新快表，很多错误发生在快表没有及时刷新
	flush_tlb();
out:
	tsk->mm = newmm;
	return error;
}

void exit_mm_fork(task_t *tsk)
{
	u64_t *tmp4 = nullptr, *tmp3 = nullptr, *tmp2 = nullptr;
	u64_t tmp1 = 0;
	size_t i = 0, j = 0, k = 0;
	struct Page* p = nullptr;
	if (tsk->flags & PF_VFORK)
		return;

	struct mm_struct *newmm = tsk->mm;
	tmp4 = (u64_t*)newmm->pgd;

	for(i = 0; i < 256; i++) {	// 遍历 PML4 页表
		if((*(tmp4 + i)) & PAGE_Present) {
			tmp3 = Phy_To_Virt(*(tmp4 + i) & ~(0xfffUL)); // 屏蔽目录项标志位，获取PDPT页表地址
			
			for (j = 0; j < 512; j++) { // 遍历 PDPT 页表
				if((*(tmp3 + j)) & PAGE_Present) {
					
					tmp2 = Phy_To_Virt(*(tmp3 + j) & ~(0xfffUL)) ; //遍历 PDT 页表项
					for(k = 0; k < 512; k++) {
							tmp1 = (*(tmp2 + k));
							p = (memory_management_struct.pages_struct + (tmp1  >> PAGE_2M_SHIFT));
							assert(p->reference_count > 1);
							p->PHY_address--;
							// for parent_process's page_table privilege, give page_fault solve. 
						}
					kdelete(Phy_To_Virt(*tmp2), PAGE_4K_SIZE);
				}
			}
			kdelete(Phy_To_Virt(*tmp3), PAGE_4K_SIZE);
		}
	}
	kdelete(Phy_To_Virt(tsk->mm->pgd), PAGE_4K_SIZE); // release PMl4's memory

	if (tsk->mm != nullptr)
		kdelete(tsk->mm, sizeof(struct mm_struct));
}


// 为子进程伪造应用层执行现场
u64_t copy_thread(u64_t clone_flags, u64_t stack_start, u64_t stack_size, task_t *tsk, pt_regs_t *regs)
{
	thread_t *thd = nullptr;
	pt_regs_t *childregs = nullptr; // 应用层执行现场结构体

	// 开辟中断栈，
	thd = (thread_t *)(tsk + 1);
	memset(thd, 0, sizeof(*thd));
	tsk->thread = thd;

	// 给新进程复制上下文环境到中断栈内
	childregs = (pt_regs_t *)((u64_t)tsk + STACK_SIZE) - 1;
	memcpy(regs, childregs, sizeof(pt_regs_t));

	childregs->rax = 0;			  // fork给子进程，返回值为0
	childregs->rsp = stack_start; // 设置子进程的应用层栈指针 ？为什么这里设置为0?

	// 中断栈中的rsp会在新进程从内核中启动运行的时候用到
	// rsp0用于新进程进入内核用到的栈
	thd->rsp0 = (u64_t)tsk + STACK_SIZE;
	thd->rsp = (u64_t)childregs;
	thd->fs = current->thread->fs;
	thd->gs = current->thread->gs;

	if (tsk->flags & PF_KTHREAD)
		thd->rip = (u64_t)kernel_thread_func;
	else
		thd->rip = (u64_t)ret_system_call;

	// color_printk(WHITE, BLACK, "current user ret addr:%#018lx, rsp:%#018lx\n", regs->r10, regs->r11);
	// color_printk(WHITE, BLACK, "new user ret addr:%#018lx, rsp:%#018lx\n", childregs->r10, childregs->r11);

	return 0;
}

void exit_thread(task_t *tsk) {}

/**
 * @brief a.do_fork函数会先为PCB分配存储空间并对其进行初步赋值
 *		  b. 根据clone_flags参数进行克隆或共享工作
 * @param regs
 * @param clone_flags 克隆信息
 * @param stack_start
 * @param stack_size
 * @return u64_t 为父进程返回子进程的ID号
 */
u64_t do_fork(pt_regs_t *regs, u64_t clone_flags, u64_t stack_start, u64_t stack_size)
{
	s32_t retval = 0;
	task_t *tsk = nullptr;

	// alloc & copy task struct
	tsk = (task_t *)knew(STACK_SIZE, 0);
	// color_printk(WHITE, BLACK, "struct_task address:%#018lx\n", (u64_t)tsk);
	if (tsk == nullptr)
	{
		retval = -EAGAIN;
		goto alloc_copy_task_fail;
	}
	memset(tsk, 0, sizeof(*tsk));
	memcpy(current, tsk, sizeof(task_t));
	list_init(&tsk->list);
	wait_queue_init(&tsk->wait_childexit, current);
	
	tsk->exit_code = 0;
	tsk->priority = 2;
	tsk->pid = global_pid++;
	tsk->preempt_count = 0; // 进程抢占计数值初始化
	
	tsk->gid = 64 + tsk->pid;
	tsk->uid = 64 + tsk->pid;
	tsk->umask = 0002;

	// 拷贝信号
	tsk->sigaction = (sigaction_t*)knew(sizeof(sigaction_t) * (NSIG + 1), 0);
	memcpy(current->sigaction, tsk->sigaction, sizeof(sigaction_t) * (NSIG + 1));

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
	if (copy_mm_fork(clone_flags, tsk))
		goto copy_mm_fail;
	// copy file struct
	if (copy_files(clone_flags, tsk))
		goto copy_files_fail;
	// copy thread struct
	if (copy_thread(clone_flags, stack_start, tsk->mm->stack_length, tsk, regs))
		goto copy_thread_fail;
	retval = tsk->pid;
	wakeup_process(tsk);

// fork_ok:
	return retval;

copy_thread_fail:
	exit_thread(tsk);
copy_files_fail:
	exit_files(tsk);
copy_mm_fail:
	exit_mm_fork(tsk);
copy_flags_fail:
alloc_copy_task_fail:
	kdelete(tsk, sizeof(task_t));
	return retval;
}

void exit_notify(void)
{
	wakeup(&current->parent->wait_childexit, TASK_INTERRUPTIBLE);
}

u64_t do_exit(u64_t exit_code)
{
	task_t *tsk = current;
	DEBUGK("exit task is running,arg:%#018lx\n", exit_code);
	

	cli();
	tsk->state = TASK_ZOMBIE;
	tsk->exit_code = exit_code;
	exit_thread(tsk);
	exit_files(tsk);
	// 回收信号，回收pid, recycle all resource
	if(tsk->sigaction)
		kdelete(tsk->sigaction, sizeof(sigaction_t) * (NSIG + 1));
	exit_mm(tsk);

	sti();

do_exit_again:
	
	exit_notify();
	schedule();
	goto do_exit_again;
	// 不会执行到这里的
	return 0;
}

// 线程承载的函数，参数，标志
// kernel_thread给进程创建了寄存器环境
s32_t kernel_thread(u64_t (*fn)(u64_t), u64_t arg, u64_t flags)
{ // 设置中断栈
	pt_regs_t regs;
	memset(&regs, 0, sizeof(regs));

	regs.rbx = (u64_t)fn;  // RBX保存着程序的入口地址
	regs.rdx = (u64_t)arg; // RDX保存着进程创建者传入的参数

	regs.ds = KERNEL_DS;
	regs.es = KERNEL_DS;
	regs.cs = KERNEL_CS;
	regs.ss = KERNEL_DS;
	regs.rflags = (1 << 9); // 设置可中断标志

	// rip 保存着进程引导程序, 在内核栈中设置这个rip,的意义是什么
	regs.rip = (u64_t)kernel_thread_func;

	return do_fork(&regs, flags | CLONE_VM, 0, 0);
}

// 被switch_to宏调用, 用来切换任务
void __switch_to(task_t *prev, task_t *next)
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
	u64_t *tmp = nullptr;
	u64_t *vaddr = nullptr;
	s32_t i = 0;

	vaddr = Phy_To_Virt((u64_t)Get_gdt() & (~0xfffUL));

	// 内核层空间占用顶层页表的256个页表项
	for (i = 256; i < 512; i++)
	{
		tmp = vaddr + i;
		if (*tmp == 0)
		{	
			u64_t *virtual = knew(PAGE_4K_SIZE, 0);
			memset(virtual, 0, PAGE_4K_SIZE);
			set_mpl4t(tmp, mk_mpl4t(Virt_To_Phy(virtual), PAGE_KERNEL_GDT));
			// set_mpl4t(tmp, mk_mpl4t(Virt_To_Phy(virtual), PAGE_USER_GDT));
		}
	}

	// 初始化内核进程的内存结构
	init_mm.pgd = (pml4t_t *)Get_gdt();

	init_mm.start_code = memory_management_struct.start_code;
	init_mm.end_code = memory_management_struct.end_code;

	init_mm.start_data = (u64_t)&_data;
	init_mm.end_data = memory_management_struct.end_data;

	init_mm.start_rodata = (u64_t)&_rodata;
	init_mm.end_rodata = (u64_t)&_erodata;

	init_mm.start_brk = (u64_t)&_bss;
	init_mm.end_brk = (u64_t)&_ebss;

	init_mm.start_brk = memory_management_struct.start_brk;
	init_mm.end_brk = current->addr_limit;

	init_mm.start_stack = _stack_start;

	// sysenter指令：从特权级3跳转到特权级0，sysexit指令从0特权级跳转到3特权级
	// RDX中保存RIP, RCX中保存RSP, 这是sysexit需要的
	// 把IA32_SYSENTER_CS寄存器设置为段选择子, 这个参数是sysexit/sysenter指令需要的
	wrmsr(0x174, KERNEL_CS);

	// sysenter
	wrmsr(0x175, current->thread->rsp0);	  // sysenter需要使用的rsp
	wrmsr(0x176, (u64_t)system_call); // sysenter进入系统调用后，会去执行system_call,sysenter要使用的rip

	//	init_thread,init_tss
	set_tss64(init_thread.rsp0, init_tss[0].rsp1, init_tss[0].rsp2, init_tss[0].ist1, init_tss[0].ist2, init_tss[0].ist3, init_tss[0].ist4, init_tss[0].ist5, init_tss[0].ist6, init_tss[0].ist7);
	init_tss[0].rsp0 = init_thread.rsp0;

	list_init(&init_task_union.task.list);
	list_init(&init_task_union.task.wait_childexit.wait_list);

	init_task_union.task.sigaction = (sigaction_t*)knew(sizeof(sigaction_t) * (NSIG + 1), 0);

	// 创建内核线程
	kernel_thread(init, 13, CLONE_FS | CLONE_SIGNAL);

	init_task_union.task.state = TASK_RUNNING;
	init_task_union.task.preempt_count = 0;
}


