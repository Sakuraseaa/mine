#include "toolkit.h"
#include "kernelkit.h"
#include "mmkit.h"
#include "arch_x86kit.h"
s32_t lookup_kallsyms(u64_t address,s32_t level)
{
    s32_t index = 0;
    s32_t level_index = 0;
    str_t string = (str_t)&kallsyms_names;
    for(index = 0;index<kallsyms_syms_num;index++)
        if(address > kallsyms_addresses[index] && address <= kallsyms_addresses[index+1])
            break;
    if(index < kallsyms_syms_num)
    {
        for(level_index = 0;level_index < level;level_index++) 
        {
            color_printk(RED,BLACK,"  ");
        }
        color_printk(RED,BLACK,"+---> ");

        color_printk(RED,BLACK,"address:%#018lx \t(+) %04d function:%s\n",address,address - kallsyms_addresses[index],&string[kallsyms_index[index]]);
        ERRORK("address:%#018lx \t(+) %04d function:%s",address,address - kallsyms_addresses[index],&string[kallsyms_index[index]]);
        return 0;
    }
    else
        return 1;
}

void backtrace(pt_regs_t * regs)
{
    u64_t *rbp = (u64_t *)regs->rbp;
    u64_t ret_address = regs->rip;
    s32_t i = 0;

    color_printk(RED,BLACK,"====================== Kernel Stack Backtrace From Stack Top To Stack Bottom======================\n");
    ERRORK("====================== Kernel Stack Backtrace From Stack Top To Stack Bottom======================\n");

    for(i = 0;i<10;i++)
    {
        if(lookup_kallsyms(ret_address,i))
            break;
        if((u64_t)rbp < (u64_t)regs->rsp || (u64_t)rbp > current->thread->rsp0)
            break;
        
        /*这里不是特别理解，这与函数调用机制有关*/
        ret_address = *(rbp + 1);
        rbp = (u64_t *)*rbp; // 可以想象但 没有手操过
    }
}

void display_regs(pt_regs_t * regs)
{
    color_printk(RED,BLACK,"CS:%#010x,SS:%#010x\nDS:%#010x,ES:%#010x\nRFLAGS:%#018lx\n",regs->cs,regs->ss,regs->ds,regs->es,regs->rflags);
    color_printk(RED,BLACK,"RAX:%#018lx,RBX:%#018lx,RCX:%#018lx,RDX:%#018lx\nRSP:%#018lx,RBP:%#018lx,RIP:%#018lx\nRSI:%#018lx,RDI:%#018lx\n",regs->rax,regs->rbx,regs->rcx,regs->rdx,regs->rsp,regs->rbp,regs->rip,regs->rsi,regs->rdi);
    color_printk(RED,BLACK,"R8 :%#018lx,R9 :%#018lx\nR10:%#018lx,R11:%#018lx\nR12:%#018lx,R13:%#018lx\nR14:%#018lx,R15:%#018lx\n",regs->r8,regs->r9,regs->r10,regs->r11,regs->r12,regs->r13,regs->r14,regs->r15);
    ERRORK("CS:%#010x,SS:%#010x",regs->cs,regs->ss);
    ERRORK("DS:%#010x,ES:%#010x",regs->ds,regs->es);
    ERRORK("RFLAGS:%#018lx", regs->rflags);
    ERRORK("RAX:%#018lx,RBX:%#018lx,RCX:%#018lx,RDX:%#018lx", regs->rax,regs->rbx,regs->rcx,regs->rdx);
    ERRORK("RSP:%#018lx,RBP:%#018lx,RIP:%#018lx",regs->rsp,regs->rbp,regs->rip);
    ERRORK("RSI:%#018lx,RDI:%#018lx",regs->rsi,regs->rdi);
    ERRORK("R8 :%#018lx,R9 :%#018lx",regs->r8,regs->r9);
    ERRORK("R10:%#018lx,R11:%#018lx",regs->r10,regs->r11);
    ERRORK("R12:%#018lx,R13:%#018lx",regs->r12,regs->r13);
    ERRORK("R14:%#018lx,R15:%#018lx",regs->r14,regs->r15);
    backtrace(regs);
}

void do_divide_error(pt_regs_t * regs,u64_t error_code)
{
	color_printk(RED,BLACK,"do_divide_error(0),ERROR_CODE:%#018lx,CPU:%#010x,PID:%#010x\n",error_code,SMP_cpu_id(),current->pid);
	display_regs(regs);
	while(1)
		hlt();
}

void do_debug(pt_regs_t * regs,u64_t error_code)
{
	color_printk(RED,BLACK,"do_debug(1),ERROR_CODE:%#018lx,CPU:%#010x,PID:%#010x\n",error_code,SMP_cpu_id(),current->pid);
	display_regs(regs);
	while(1)
		hlt();
}

void do_nmi(pt_regs_t * regs,u64_t error_code)
{
	color_printk(RED,BLACK,"do_nmi(2),ERROR_CODE:%#018lx,CPU:%#010x,PID:%#010x\n",error_code,SMP_cpu_id(),current->pid);
	display_regs(regs);
	while(1)
		hlt();
}

void do_int3(pt_regs_t * regs,u64_t error_code)
{
	color_printk(RED,BLACK,"do_int3(3),ERROR_CODE:%#018lx,CPU:%#010x,PID:%#010x\n",error_code,SMP_cpu_id(),current->pid);
	display_regs(regs);
	while(1)
		hlt();
}

void do_overflow(pt_regs_t * regs,u64_t error_code)
{
	color_printk(RED,BLACK,"do_overflow(4),ERROR_CODE:%#018lx,CPU:%#010x,PID:%#010x\n",error_code,SMP_cpu_id(),current->pid);
	display_regs(regs);
	while(1)
		hlt();
}


void do_bounds(pt_regs_t * regs,u64_t error_code)
{
	color_printk(RED,BLACK,"do_bounds(5),ERROR_CODE:%#018lx,CPU:%#010x,PID:%#010x\n",error_code ,SMP_cpu_id(),current->pid);
	display_regs(regs);
	while(1)
		hlt();
}

void do_undefined_opcode(pt_regs_t * regs,u64_t error_code)
{
	color_printk(RED,BLACK,"do_undefined_opcode(6),ERROR_CODE:%#018lx,CPU:%#010x,PID:%#010x\n",error_code,SMP_cpu_id(),current->pid);
	display_regs(regs);
	while(1)
		hlt();
}


void do_dev_not_available(pt_regs_t * regs,u64_t error_code)
{
	color_printk(RED,BLACK,"do_dev_not_available(7),ERROR_CODE:%#018lx,CPU:%#010x,PID:%#010x\n",error_code,SMP_cpu_id(),current->pid);
	display_regs(regs);
	while(1)
		hlt();
}


void do_double_fault(pt_regs_t * regs,u64_t error_code)
{
	color_printk(RED,BLACK,"do_double_fault(8),ERROR_CODE:%#018lx,CPU:%#010x,PID:%#010x\n",error_code,SMP_cpu_id(),current->pid);
	display_regs(regs);
	while(1)
		hlt();
}


void do_coprocessor_segment_overrun(pt_regs_t * regs,u64_t error_code)
{
	color_printk(RED,BLACK,"do_coprocessor_segment_overrun(9),ERROR_CODE:%#018lx,CPU:%#010x,PID:%#010x\n",error_code,SMP_cpu_id(),current->pid);
	display_regs(regs);
	while(1)
		hlt();
}



void do_invalid_TSS(pt_regs_t * regs,u64_t error_code)
{
	color_printk(RED,BLACK,"do_invalid_TSS(10),ERROR_CODE:%#018lx,CPU:%#010x,PID:%#010x\n",error_code,SMP_cpu_id(),current->pid);

	if(error_code & 0x01)
		color_printk(RED,BLACK,"The exception occurred during delivery of an event external to the program,such as an interrupt or an earlier exception.\n");

	if(error_code & 0x02)
		color_printk(RED,BLACK,"Refers to a gate descriptor in the IDT;\n");
	else
		color_printk(RED,BLACK,"Refers to a descriptor in the GDT or the current LDT;\n");

	if ((error_code & 0x02) == 0)
	{
		if(error_code & 0x04) {
			color_printk(RED,BLACK,"Refers to a segment or gate descriptor in the LDT;\n");
		} else {
			color_printk(RED,BLACK,"Refers to a descriptor in the current GDT;\n");
		}
	}

	color_printk(RED,BLACK,"Segment Selector Index:%#010x\n",error_code & 0xfff8);

	display_regs(regs);
	while(1)
		hlt();
}



void do_segment_not_present(pt_regs_t * regs,u64_t error_code)
{
	color_printk(RED,BLACK,"do_segment_not_present(11),ERROR_CODE:%#018lx,CPU:%#010x,PID:%#010x\n",error_code,SMP_cpu_id(),current->pid);

	if(error_code & 0x01)
		color_printk(RED,BLACK,"The exception occurred during delivery of an event external to the program,such as an interrupt or an earlier exception.\n");

	if(error_code & 0x02)
		color_printk(RED,BLACK,"Refers to a gate descriptor in the IDT;\n");
	else
		color_printk(RED,BLACK,"Refers to a descriptor in the GDT or the current LDT;\n");

	if ((error_code & 0x02) == 0)
	{
		if (error_code & 0x04) {
			color_printk(RED,BLACK,"Refers to a segment or gate descriptor in the LDT;\n");
		} else {
			color_printk(RED,BLACK,"Refers to a descriptor in the current GDT;\n");
		}
	}
	color_printk(RED,BLACK,"Segment Selector Index:%#010x\n",error_code & 0xfff8);

	display_regs(regs);
	while(1)
		hlt();
}


void do_stack_segment_fault(pt_regs_t * regs,u64_t error_code)
{
	color_printk(RED,BLACK,"do_stack_segment_fault(12),ERROR_CODE:%#018lx,CPU:%#010x,PID:%#010x\n",error_code,SMP_cpu_id(),current->pid);

	if(error_code & 0x01)
		color_printk(RED,BLACK,"The exception occurred during delivery of an event external to the program,such as an interrupt or an earlier exception.\n");

	if(error_code & 0x02)
		color_printk(RED,BLACK,"Refers to a gate descriptor in the IDT;\n");
	else
		color_printk(RED,BLACK,"Refers to a descriptor in the GDT or the current LDT;\n");

	if((error_code & 0x02) == 0) {
		if(error_code & 0x04)
			color_printk(RED,BLACK,"Refers to a segment or gate descriptor in the LDT;\n");
		else
			color_printk(RED,BLACK,"Refers to a descriptor in the current GDT;\n");
	}

	color_printk(RED,BLACK,"Segment Selector Index:%#010x\n",error_code & 0xfff8);

	display_regs(regs);
	while(1)
		hlt();
}



void do_general_protection(pt_regs_t * regs,u64_t error_code)
{
	color_printk(RED,BLACK,"do_general_protection(13),ERROR_CODE:%#018lx,CPU:%#010x,PID:%#010x\n",error_code,SMP_cpu_id(),current->pid);

	if(error_code & 0x01)
		color_printk(RED,BLACK,"The exception occurred during delivery of an event external to the program,such as an interrupt or an earlier exception.\n");

	if(error_code & 0x02)
		color_printk(RED,BLACK,"Refers to a gate descriptor in the IDT;\n");
	else
		color_printk(RED,BLACK,"Refers to a descriptor in the GDT or the current LDT;\n");

	if((error_code & 0x02) == 0){
		if(error_code & 0x04)
			color_printk(RED,BLACK,"Refers to a segment or gate descriptor in the LDT;\n");
		else
			color_printk(RED,BLACK,"Refers to a descriptor in the current GDT;\n");
	}

	color_printk(RED,BLACK,"Segment Selector Index:%#010x\n",error_code & 0xfff8);

	display_regs(regs);
	while(1)
		hlt();
}


void do_page_fault(pt_regs_t * regs,u64_t error_code)
{
    u64_t cr2 = 0;
    u64_t cr3 = 0;

    __asm__	__volatile__("movq	%%cr2,	%0":"=r"(cr2)::"memory");
    __asm__	__volatile__("movq	%%cr3,	%0":"=r"(cr3)::"memory");

    if(!(error_code & 0x01)) {
        ERRORK("rip:%#lx proc:%d, error address %#0lx cr3:%#0lx", regs->rip, current->pid, cr2, cr3);
        if(do_no_page(cr2) == EOK)	// 处理缺页成功则返回
            return;
        
        color_printk(RED,BLACK,"(%d)Page Not-Present! address: %#x\t", current->pid, cr2);
    }
    if(error_code & 0x02) {
        if(do_wp_page(cr2) == EOK)
            return;
        
        color_printk(RED,BLACK,"Write Cause Fault,\t");
    }
    else
        color_printk(RED,BLACK,"Read Cause Fault,\t");

    if(error_code & 0x04)
        color_printk(RED,BLACK,"Fault in user(3)\t");
    else
        color_printk(RED,BLACK,"Fault in supervisor(0,1,2)\t");

    if(error_code & 0x08)
        color_printk(RED,BLACK,",Reserved Bit Cause Fault\t");

    if(error_code & 0x10)
        color_printk(RED,BLACK,",Instruction fetch Cause Fault");

    color_printk(RED,BLACK,"\n");

    color_printk(RED,BLACK,"CR2:%#018lx\n",cr2);
    ERRORK("CR2:%#018lx",cr2);
    display_regs(regs);
    while(1)
        hlt();
}


void do_x87_FPU_error(pt_regs_t * regs,u64_t error_code)
{
	color_printk(RED,BLACK,"do_x87_FPU_error(16),ERROR_CODE:%#018lx,CPU:%#010x,PID:%#010x\n",error_code,SMP_cpu_id(),current->pid);
	display_regs(regs);
	while(1)
		hlt();
}



void do_alignment_check(pt_regs_t * regs,u64_t error_code)
{
	color_printk(RED,BLACK,"do_alignment_check(17),ERROR_CODE:%#018lx,CPU:%#010x,PID:%#010x\n",error_code,SMP_cpu_id(),current->pid);
	display_regs(regs);
	while(1)
		hlt();
}


void do_machine_check(pt_regs_t * regs,u64_t error_code)
{
	color_printk(RED,BLACK,"do_machine_check(18),ERROR_CODE:%#018lx,CPU:%#010x,PID:%#010x\n",error_code,SMP_cpu_id(),current->pid);
	display_regs(regs);
	while(1)
		hlt();
}


void do_SIMD_exception(pt_regs_t * regs,u64_t error_code)
{
	color_printk(RED,BLACK,"do_SIMD_exception(19),ERROR_CODE:%#018lx,CPU:%#010x,PID:%#010x\n",error_code,SMP_cpu_id(),current->pid);
	display_regs(regs);
	while(1)
		hlt();
}

void do_virtualization_exception(pt_regs_t * regs,u64_t error_code)
{
	color_printk(RED,BLACK,"do_virtualization_exception(20),ERROR_CODE:%#018lx,CPU:%#010x,PID:%#010x\n",error_code,SMP_cpu_id(),current->pid);
	display_regs(regs);
	while(1)
		hlt();
}


void sys_vector_init()
{
	set_trap_gate(0,0,divide_error);
	set_trap_gate(1,0,debug);
	set_intr_gate(2,0,nmi);
	set_system_gate(3,0,int3);
	set_system_gate(4,0,overflow);
	set_system_gate(5,0,bounds);
	set_trap_gate(6,0,undefined_opcode);
	set_trap_gate(7,0,dev_not_available);
	set_trap_gate(8,0,double_fault);
	set_trap_gate(9,0,coprocessor_segment_overrun);
	set_trap_gate(10,0,invalid_TSS);
	set_trap_gate(11,0,segment_not_present);
	set_trap_gate(12,0,stack_segment_fault);
	set_trap_gate(13,0,general_protection);
	// set_trap_gate(14,0,page_fault); // 陷阱门的中断处理函数可重入
	set_intr_gate(14,0,page_fault); // 中断门的中断处理函数不可重入，寄存器会自动
	//15 Intel reserved. Do not use.
	set_trap_gate(16,0,x87_FPU_error);
	set_trap_gate(17,0,alignment_check);
	set_trap_gate(18,0,machine_check);
	set_trap_gate(19,0,SIMD_exception);
	set_trap_gate(20,0,virtualization_exception);

	//set_system_gate(SYSTEM_CALL_VECTOR,7,system_call);

}