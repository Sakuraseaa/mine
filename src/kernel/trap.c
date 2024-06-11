#include "trap.h"
#include "gate.h"
#include "lib.h"
#include "ptrace.h"

int lookup_kallsyms(unsigned long address, int level)
{
	int index = 0;
	int level_index = 0;

	char* string = (char*)&kallsyms_names;
	for(index = 0; index < kallsyms_syms_num; index++)
		if(address > kallsyms_addresses[index] && address <= kallsyms_addresses[index + 1])
			break;
	
	if(index < kallsyms_syms_num) {
		for(level_index = 0; level_index < level; level_index++)
			color_printk(RED, BLACK, " ");
		color_printk(RED, BLACK, "+-->");

		color_printk(RED, BLACK, "address:%#018lx \t(+) %04d function:%s\n", address,
		 address - kallsyms_addresses[index], &string[kallsyms_index[index]]);
		return 0;
	}
	else
		return 1;

}


void backtrace(struct pt_regs* regs)
{
	unsigned long *rbp = (unsigned long*)regs->rbp;
	unsigned long ret_address = regs->rip;
	int i = 0;

	// color_printk(RED,BLACK,"&kallsyms_addresses:%#018lx,kallsyms_addresses:%#018lx\n",&kallsyms_addresses,kallsyms_addresses);
	// color_printk(RED,BLACK,"&kallsyms_syms_num:%#018lx,kallsyms_syms_num:%d\n",&kallsyms_syms_num,kallsyms_syms_num);
	// color_printk(RED,BLACK,"&kallsyms_index:%#018lx\n",&kallsyms_index);
	// color_printk(RED,BLACK,"&kallsyms_names:%#018lx,kallsyms_names:%s\n",&kallsyms_names,&kallsyms_names);
	color_printk(RED,BLACK,"====================== Kernel Stack Backtrace ======================\n");

	for(i = 0; i < 10; i++)
	{
		if(lookup_kallsyms(ret_address, i))
			break;
		
		if((unsigned long)rbp < (unsigned long)regs->rsp || (unsigned long)rbp > current->thread->rsp0)
					break;
		ret_address = *(rbp + 1);
		rbp = (unsigned long *)*rbp;
		// color_printk(RED, BLACK, "rbp:%#018lx, *rbp:%#018lx\n", rbp, *rbp);
	}
}

void display_regs(struct pt_regs * regs)
{
	color_printk(RED,BLACK,"CS:%#010x,SS:%#010x\nDS:%#010x,ES:%#010x\nRFLAGS:%#018lx\n",regs->cs,regs->ss,regs->ds,regs->es,regs->rflags);
	color_printk(RED,BLACK,"RAX:%#018lx,RBX:%#018lx,RCX:%#018lx,RDX:%#018lx\nRSP:%#018lx,RBP:%#018lx,RIP:%#018lx\nRSI:%#018lx,RDI:%#018lx\n",regs->rax,regs->rbx,regs->rcx,regs->rdx,regs->rsp,regs->rbp,regs->rip,regs->rsi,regs->rdi);
	color_printk(RED,BLACK,"R8 :%#018lx,R9 :%#018lx\nR10:%#018lx,R11:%#018lx\nR12:%#018lx,R13:%#018lx\nR14:%#018lx,R15:%#018lx\n",regs->r8,regs->r9,regs->r10,regs->r11,regs->r12,regs->r13,regs->r14,regs->r15);
	backtrace(regs);
}

void do_divide_error(struct pt_regs* regs, unsigned long error_code)
{

	color_printk(RED, BLACK, "do_divide_error(0),ERROR_CODE:%#018lx\n", error_code);
	backtrace(regs);
	while (1)
		hlt();
}

void do_debug(unsigned long rsp, unsigned long error_code)
{
	unsigned long *p = NULL;
	p = (unsigned long *)(rsp + 0x98);
	color_printk(RED, BLACK, "do_debug(1),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, *p);
	while (1)
		hlt();
}

void do_nmi(unsigned long rsp, unsigned long error_code)
{
	unsigned long *p = NULL;
	p = (unsigned long *)(rsp + 0x98);
	color_printk(RED, BLACK, "do_nmi(2),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, *p);
	while (1)
		hlt();
}

void do_int3(unsigned long rsp, unsigned long error_code)
{
	unsigned long *p = NULL;
	p = (unsigned long *)(rsp + 0x98);
	color_printk(RED, BLACK, "do_int3(3),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, *p);
	while (1)
		hlt();
}

void do_overflow(unsigned long rsp, unsigned long error_code)
{
	unsigned long *p = NULL;
	p = (unsigned long *)(rsp + 0x98);
	color_printk(RED, BLACK, "do_overflow(4),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, *p);
	while (1)
		hlt();
}

void do_bounds(unsigned long rsp, unsigned long error_code)
{
	unsigned long *p = NULL;
	p = (unsigned long *)(rsp + 0x98);
	color_printk(RED, BLACK, "do_bounds(5),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, *p);
	while (1)
		hlt();
}

void do_undefined_opcode(unsigned long rsp, unsigned long error_code)
{
	unsigned long *p = NULL;
	p = (unsigned long *)(rsp + 0x98);
	color_printk(RED, BLACK, "do_undefined_opcode(6),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, *p);
	while (1)
		hlt();
}

void do_dev_not_available(unsigned long rsp, unsigned long error_code)
{
	unsigned long *p = NULL;
	p = (unsigned long *)(rsp + 0x98);
	color_printk(RED, BLACK, "do_dev_not_available(7),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, *p);
	while (1)
		hlt();
}

void do_double_fault(unsigned long rsp, unsigned long error_code)
{
	unsigned long *p = NULL;
	p = (unsigned long *)(rsp + 0x98);
	color_printk(RED, BLACK, "do_double_fault(8),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, *p);
	while (1)
		hlt();
}

void do_coprocessor_segment_overrun(unsigned long rsp, unsigned long error_code)
{
	unsigned long *p = NULL;
	p = (unsigned long *)(rsp + 0x98);
	color_printk(RED, BLACK, "do_coprocessor_segment_overrun(9),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, *p);
	while (1)
		hlt();
}

void do_invalid_TSS(unsigned long rsp, unsigned long error_code)
{
	unsigned long *p = NULL;
	p = (unsigned long *)(rsp + 0x98);
	color_printk(RED, BLACK, "do_invalid_TSS(10),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, *p);

	if (error_code & 0x01)
		color_printk(RED, BLACK, "The exception occurred during delivery of an event external to the program,such as an interrupt or an earlier exception.\n");

	if (error_code & 0x02)
		color_printk(RED, BLACK, "Refers to a gate descriptor in the IDT;\n");
	else
		color_printk(RED, BLACK, "Refers to a descriptor in the GDT or the current LDT;\n");

	if ((error_code & 0x02) == 0)
		if (error_code & 0x04)
			color_printk(RED, BLACK, "Refers to a segment or gate descriptor in the LDT;\n");
		else
			color_printk(RED, BLACK, "Refers to a descriptor in the current GDT;\n");

	color_printk(RED, BLACK, "Segment Selector Index:%#010x\n", error_code & 0xfff8);

	while (1)
		hlt();
}

void do_segment_not_present(unsigned long rsp, unsigned long error_code)
{
	unsigned long *p = NULL;
	p = (unsigned long *)(rsp + 0x98);
	color_printk(RED, BLACK, "do_segment_not_present(11),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, *p);

	if (error_code & 0x01)
		color_printk(RED, BLACK, "The exception occurred during delivery of an event external to the program,such as an interrupt or an earlier exception.\n");

	if (error_code & 0x02)
		color_printk(RED, BLACK, "Refers to a gate descriptor in the IDT;\n");
	else
		color_printk(RED, BLACK, "Refers to a descriptor in the GDT or the current LDT;\n");

	if ((error_code & 0x02) == 0)
		if (error_code & 0x04)
			color_printk(RED, BLACK, "Refers to a segment or gate descriptor in the LDT;\n");
		else
			color_printk(RED, BLACK, "Refers to a descriptor in the current GDT;\n");

	color_printk(RED, BLACK, "Segment Selector Index:%#010x\n", error_code & 0xfff8);

	while (1)
		hlt();
}

void do_stack_segment_fault(unsigned long rsp, unsigned long error_code)
{
	unsigned long *p = NULL;
	p = (unsigned long *)(rsp + 0x98);
	color_printk(RED, BLACK, "do_stack_segment_fault(12),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, *p);

	if (error_code & 0x01)
		color_printk(RED, BLACK, "The exception occurred during delivery of an event external to the program,such as an interrupt or an earlier exception.\n");

	if (error_code & 0x02)
		color_printk(RED, BLACK, "Refers to a gate descriptor in the IDT;\n");
	else
		color_printk(RED, BLACK, "Refers to a descriptor in the GDT or the current LDT;\n");

	if ((error_code & 0x02) == 0)
		if (error_code & 0x04)
			color_printk(RED, BLACK, "Refers to a segment or gate descriptor in the LDT;\n");
		else
			color_printk(RED, BLACK, "Refers to a descriptor in the current GDT;\n");

	color_printk(RED, BLACK, "Segment Selector Index:%#010x\n", error_code & 0xfff8);

	while (1)
		hlt();
}

void do_general_protection(unsigned long rsp, unsigned long error_code)
{
	unsigned long *p = NULL;
	p = (unsigned long *)(rsp + 0x98);
	color_printk(RED, BLACK, "do_general_protection(13),ERROR_CODE:%#018lx,OLD_RSP:%#018lx,RIP:%#018lx\n",
				 error_code, *(unsigned long *)(rsp + 0xb0), *p);

	if (error_code & 0x01)
		color_printk(RED, BLACK, "The exception occurred during delivery of an event external to the program,such as an interrupt or an earlier exception.\n");

	if (error_code & 0x02)
		color_printk(RED, BLACK, "Refers to a gate descriptor in the IDT;\n");
	else
		color_printk(RED, BLACK, "Refers to a descriptor in the GDT or the current LDT;\n");

	if ((error_code & 0x02) == 0)
		if (error_code & 0x04)
			color_printk(RED, BLACK, "Refers to a segment or gate descriptor in the LDT;\n");
		else
			color_printk(RED, BLACK, "Refers to a descriptor in the current GDT;\n");

	color_printk(RED, BLACK, "Segment Selector Index:%#010x\n", error_code & 0xfff8);

	while (1)
		hlt();
}

void do_page_fault(unsigned long rsp, unsigned long error_code)
{
	unsigned long *p = NULL;
	unsigned long cr2 = 0;

	__asm__ __volatile__("movq	%%cr2,	%0"
						 : "=r"(cr2)::"memory");

	p = (unsigned long *)(rsp + 0x98);
	color_printk(RED, BLACK, "do_page_fault(14),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, *(unsigned long *)(rsp + 0xb0), *p);

	if (!(error_code & 0x01))
		color_printk(RED, BLACK, "Page Not-Present,\t");

	if (error_code & 0x02)
		color_printk(RED, BLACK, "Write Cause Fault,\t");
	else
		color_printk(RED, BLACK, "Read Cause Fault,\t");

	if (error_code & 0x04)
		color_printk(RED, BLACK, "Fault in user(3)\t");
	else
		color_printk(RED, BLACK, "Fault in supervisor(0,1,2)\t");

	if (error_code & 0x08)
		color_printk(RED, BLACK, ",Reserved Bit Cause Fault\t");

	if (error_code & 0x10)
		color_printk(RED, BLACK, ",Instruction fetch Cause Fault");

	color_printk(RED, BLACK, "\n");

	color_printk(RED, BLACK, "CR2:%#018lx\n", cr2);

	while (1)
		hlt();
}

void do_x87_FPU_error(unsigned long rsp, unsigned long error_code)
{
	unsigned long *p = NULL;
	p = (unsigned long *)(rsp + 0x98);
	color_printk(RED, BLACK, "do_x87_FPU_error(16),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, *p);
	while (1)
		hlt();
}

void do_alignment_check(unsigned long rsp, unsigned long error_code)
{
	unsigned long *p = NULL;
	p = (unsigned long *)(rsp + 0x98);
	color_printk(RED, BLACK, "do_alignment_check(17),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, *p);
	while (1)
		hlt();
}

void do_machine_check(unsigned long rsp, unsigned long error_code)
{
	unsigned long *p = NULL;
	p = (unsigned long *)(rsp + 0x98);
	color_printk(RED, BLACK, "do_machine_check(18),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, *p);
	while (1)
		hlt();
}

void do_SIMD_exception(unsigned long rsp, unsigned long error_code)
{
	unsigned long *p = NULL;
	p = (unsigned long *)(rsp + 0x98);
	color_printk(RED, BLACK, "do_SIMD_exception(19),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, *p);
	while (1)
		hlt();
}

void do_virtualization_exception(unsigned long rsp, unsigned long error_code)
{
	unsigned long *p = NULL;
	p = (unsigned long *)(rsp + 0x98);
	color_printk(RED, BLACK, "do_virtualization_exception(20),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, *p);
	while (1)
		hlt();
}

void sys_vector_init()
{
	set_trap_gate(0, 1, divide_error);
	set_trap_gate(1, 1, debug);
	set_intr_gate(2, 1, nmi);
	set_system_gate(3, 1, int3);
	set_system_gate(4, 1, overflow);
	set_system_gate(5, 1, bounds);
	set_trap_gate(6, 1, undefined_opcode);
	set_trap_gate(7, 1, dev_not_available);
	set_trap_gate(8, 1, double_fault);
	set_trap_gate(9, 1, coprocessor_segment_overrun);
	set_trap_gate(10, 1, invalid_TSS);
	set_trap_gate(11, 1, segment_not_present);
	set_trap_gate(12, 1, stack_segment_fault);
	set_trap_gate(13, 1, general_protection);
	set_trap_gate(14, 1, page_fault);
	// 15 Intel reserved. Do not use.
	set_trap_gate(16, 1, x87_FPU_error);
	set_trap_gate(17, 1, alignment_check);
	set_trap_gate(18, 1, machine_check);
	set_trap_gate(19, 1, SIMD_exception);
	set_trap_gate(20, 1, virtualization_exception);

	// set_system_gate(SYSTEM_CALL_VECTOR,7,system_call);
}
