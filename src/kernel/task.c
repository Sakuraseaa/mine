#include "toolkit.h"
#include "mmkit.h"
#include "syskit.h"
#include "kernelkit.h"

union task_union init_task_union
	__attribute__((__section__(".data.init_task"))) = {INIT_TASK(init_task_union.task)};

task_t *init_task[NR_CPUS] = {&init_task_union.task, 0};

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
s64_t global_pid = 2;

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
	DISK1_FAT32_FS_init();
 	DEBUGK("init task is running, arg:%#018lx\n", arg);

	// sys_open("/The quick brown.fox", O_CREAT);

	current->thread->rip = (u64_t)ret_system_call;
	current->thread->rsp = (u64_t)current + STACK_SIZE - sizeof(pt_regs_t);
	current->thread->gs = USER_DS;
	current->thread->fs = USER_DS;
	current->flags &= ~PF_KTHREAD;

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
	__asm__ __volatile__("movq %0, %%cr3 \n\t" ::"r"(next->mm->msd_mmu.mud_cr3)
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
u64_t copy_mm_fork(u64_t clone_flags, task_t *tsk)
{
	s32_t error = 0;
	mmdsc_t *newmm = nullptr;
	list_n_t *vma_entry = nullptr;
	kmvarsdsc_t *vma = nullptr;
	adr_t  vma_start = 0, vma_end = 0, vma_phy = 0;           /* 虚拟地址的开始 虚拟地址的结束 */ 
	msadsc_t *tmpmsa = nullptr;
	
	if (clone_flags & CLONE_VM) {
		newmm = current->mm;
		goto out;
	}

	// 2. 拷贝所有的虚拟地址区间, 为虚拟空间内已经映射了的虚拟地址，进行重新映射，
	// 3. 在halmm中添加修改页表属性的函数
	// 1. 拷贝4级页表，修改初始化函数为只拷贝后256项
	newmm = tsk->mm = (mmdsc_t *)knew(sizeof(mmdsc_t), 0);
	// 进程相关内存初始化三部曲
	mmadrsdsc_t_init(tsk->mm);
	kvma_inituserspace_virmemadrs(&tsk->mm->msd_virmemadrs);
	hal_mmu_init(&tsk->mm->msd_mmu);

	// tsk->mm->msd_virmemadrs.vs_startkmvdsc->kva_kvmbox = current->mm->msd_virmemadrs.vs_startkmvdsc->kva_kvmbox;
	// tsk->mm->msd_virmemadrs.vs_endkmvdsc->kva_kvmbox = current->mm->msd_virmemadrs.vs_endkmvdsc->kva_kvmbox;
	// 扫描所有的虚拟区间, 为子进程创建
    file_t* new_task_fp = nullptr;
    list_for_each(vma_entry, &current->mm->msd_virmemadrs.vs_list)
    {
        vma = list_entry(vma_entry, kmvarsdsc_t, kva_list);

		new_task_fp = copy_one_vma(newmm, vma, new_task_fp);

		vma_start = vma->kva_start;
		vma_end = vma->kva_end;
		while (vma_start < vma_end)
		{
			vma_phy = hal_mmu_virtophy(&current->mm->msd_mmu, vma_start);
			if (vma_phy != NULL)
			{
				tmpmsa = find_msa_from_pagebox(vma->kva_kvmbox, vma_phy);
				char* buf = knew(PAGE_4K_SIZE, 0);
				if (tmpmsa == nullptr)
				{
					color_printk(RED, BLACK, "This is a heavy error!\n");
				}
				memcpy((void*)vma_start, buf, PAGE_4K_SIZE);
				// 给父,子进程修改权限，物理页面重新映射
				hal_mmu_transform(&tsk->mm->msd_mmu, vma_start, hal_mmu_virtophy(&current->mm->msd_mmu, (adr_t)buf), (PML4E_RW | 0 | PML4E_US | PML4E_P));
				DEBUGK("direct copy to %#lx form %#lx \n", vma_start, hal_mmu_virtophy(&current->mm->msd_mmu, (adr_t)buf));
				// hal_mmu_transform(&tsk->mm->msd_mmu, vma_start, vma_phy, (0 | PML4E_US | PML4E_P));
				// hal_mmu_transform(&current->mm->msd_mmu, vma_start, vma_phy, (0 | PML4E_US | PML4E_P));
				// tmpmsa->md_phyadrs.paf_shared = PAF_SHARED;
				// tmpmsa->md_cntflgs.mf_refcnt++;
			}
			vma_start += PAGE_4K_SIZE;
		}
    }
	
	// if (vma == current->mm->msd_virmemadrs.vs_endkmvdsc)
	// {
	// 	vma_start = vma->kva_start;
	// 	vma_end = vma->kva_end;
	// 	while (vma_start < vma_end)
	// 	{
	// 		vma_phy = hal_mmu_virtophy(&current->mm->msd_mmu, vma_start);
	// 		if (vma_phy != NULL)
	// 		{
	// 			char* buf = knew(PAGE_4K_SIZE, 0);
	// 			memcpy((void*)vma_start, buf, PAGE_4K_SIZE);
	// 			// 给父,子进程修改权限，物理页面重新映射
	// 			hal_mmu_transform(&tsk->mm->msd_mmu, vma_start, hal_mmu_virtophy(&current->mm->msd_mmu, (adr_t)buf), (PML4E_RW | 0 | PML4E_US | PML4E_P));
	// 			DEBUGK("direct copy from %#lx to %#lx \n", vma_start, hal_mmu_virtophy(&current->mm->msd_mmu, (adr_t)buf));
	// 		}
	// 		vma_start += PAGE_4K_SIZE;
	// 	}
	// }


    list_for_each(vma_entry, &tsk->mm->msd_virmemadrs.vs_list)
    {
        vma = list_entry(vma_entry, kmvarsdsc_t, kva_list);

		vma_start = vma->kva_start;
		vma_end = vma->kva_end;
		DEBUGK("start:%#lx end:%#lx file:%#lx boxpage:%#lx\n", vma_start, vma_end, vma->kva_vir2file ? vma->kva_vir2file->vtf_file : 0x20021112, vma->kva_kvmbox);
    }
	flush_tlb();

	tsk->flags &= ~PF_VFORK;
out:
	tsk->mm = newmm;
	return error;
}

void exit_mm_fork(task_t *tsk)
{
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
task_t *procs[32] = {0};
u8_t procs_item = 0;
u64_t do_fork(pt_regs_t *regs, u64_t clone_flags, u64_t stack_start, u64_t stack_size)
{
	s32_t retval = 0;
	task_t *tsk = nullptr;
	// alloc & copy task struct
	tsk = (task_t *)knew(STACK_SIZE, 0);

    procs[procs_item++] = tsk;

	// color_printk(WHITE, BLACK, "struct_task address:%#018lx\n", (u64_t)tsk);
	if (tsk == nullptr)
	{
		retval = -EAGAIN;
		goto alloc_copy_task_fail;
	}
	memset(tsk, 0, sizeof(task_t));
	memcpy(current, tsk, sizeof(task_t));
	list_init(&tsk->list);
	wait_queue_init(&tsk->wait_childexit, current);
	
	tsk->exit_code = 0;
	tsk->priority = 2;
	tsk->pid = (global_pid++);
	tsk->preempt_count = 0; // 进程抢占计数值初始化
	
	tsk->gid = 0;
	tsk->uid = 0;
	tsk->umask = 0002;

	// 拷贝信号, 这里有错误
	tsk->sigaction = (sigaction_t*)knew(sizeof(sigaction_t) * (NSIG + 1), 0);
	memcpy(current->sigaction, tsk->sigaction, sizeof(sigaction_t) * (NSIG + 1));

	tsk->state = TASK_UNINTERRUPTIBLE;
	//////头插法
	tsk->next = init_task_union.task.next;
	init_task_union.task.next = tsk;
	//////
	tsk->parent = current;
	retval = -ENOMEM;

	if (copy_flags(clone_flags, tsk))
		goto copy_flags_fail;
	if (copy_mm_fork(clone_flags, tsk))
		goto copy_mm_fail;
	if (copy_files(clone_flags, tsk))
		goto copy_files_fail;
	if (copy_thread(clone_flags, stack_start, tsk->mm->stack_length, tsk, regs))
		goto copy_thread_fail;
	retval = tsk->pid;
	wakeup_process(tsk);

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
	// 初始化内核进程的内存结构
	initmm.start_code = memory_management_struct.start_code;
	initmm.end_code = memory_management_struct.end_code;

	initmm.start_data = (u64_t)&_data;
	initmm.end_data = memory_management_struct.end_data;

	initmm.start_rodata = (u64_t)&_rodata;
	initmm.end_rodata = (u64_t)&_erodata;

	initmm.start_brk = (u64_t)&_bss;
	initmm.end_brk = (u64_t)&_ebss;
	// 内核线程是不用堆的，他不用扩展堆内存目前，4GB一下皆可访问
	initmm.start_brk = 0;
	initmm.end_brk = current->addr_limit;

	initmm.start_bss = initmm.end_rodata;
	initmm.end_bss = memory_management_struct.end_of_struct;

	initmm.start_stack = _stack_start;

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
	
	init_task_union.task.pid = 0;
	init_task_union.task.state = TASK_RUNNING;
	init_task_union.task.preempt_count = 0;
	
	// 内核初始化完毕，该进程不需要再被执行
	// 给内核主程序赋值拥有虚拟时间的较大值, 依次降低他的运行优先级
	init_task_union.task.vrun_time = 200211121813;
	// init_task_union.task.vrun_time = 0;
}
