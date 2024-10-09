#include "toolkit.h"
#include "mmkit.h"
#include "arch_x86kit.h"
#include "kernelkit.h"

// 读io apic的数据寄存器
u64_t ioapic_rte_read(u8_t index)
{
	u64_t ret;

	*ioapic_map.virtual_index_address = index + 1;
	io_mfence(); // 进行数值同步 防止乱序执行带来的麻烦
	ret = *ioapic_map.virtual_data_address;
	ret <<= 32;
	io_mfence();

	*ioapic_map.virtual_index_address = index;
	io_mfence();
	ret |= *ioapic_map.virtual_data_address;
	io_mfence();

	return ret;
}

// 写io apic的数据寄存器
void ioapic_rte_write(u8_t index, u64_t value)
{
	*ioapic_map.virtual_index_address = index;
	io_mfence();
	*ioapic_map.virtual_data_address = value & 0xffffffff;
	value >>= 32;
	io_mfence();

	*ioapic_map.virtual_index_address = index + 1;
	io_mfence();
	*ioapic_map.virtual_data_address = value & 0xffffffff;
	io_mfence();
}

#if 0
// 初始化 Struct IOAPIC_map 并且 把间接访问寄存器的物理基地址映射到线性空间
static void IOAPIC_pagetable_remap()
{
	u64_t *tmp;
	u8_t *IOAPIC_addr = (u8_t *)Phy_To_Virt(0xfec00000);

	ioapic_map.physical_address = 0xfec00000;
	ioapic_map.virtual_index_address = IOAPIC_addr;
	ioapic_map.virtual_data_address = (u32_t *)(IOAPIC_addr + 0x10);
	ioapic_map.virtual_EOI_address = (u32_t *)(IOAPIC_addr + 0x40);

	Global_CR3 = Get_gdt();

	tmp = Phy_To_Virt(Global_CR3 + (((u64_t)IOAPIC_addr >> PAGE_GDT_SHIFT) & 0x1ff));
	if (*tmp == 0)
	{
		u64_t *virtual = kmalloc(PAGE_4K_SIZE, 0);
		// color_printk(WHITE, RED, "IOAPIC_kmalloc_4K"); 检测本次映射中 是否申请了内存
		set_mpl4t(tmp, mk_mpl4t(Virt_To_Phy(virtual), PAGE_KERNEL_GDT));
	}

	// color_printk(YELLOW, BLACK, "1:%#018lx\t%#018lx\n", (u64_t)tmp, (u64_t)*tmp);

	tmp = Phy_To_Virt((u64_t *)(*tmp & (~0xfffUL)) + (((u64_t)IOAPIC_addr >> PAGE_1G_SHIFT) & 0x1ff));
	if (*tmp == 0)
	{
		u64_t *virtual = kmalloc(PAGE_4K_SIZE, 0);
		// color_printk(WHITE, RED, "IOAPIC_kmalloc_4K");
		set_pdpt(tmp, mk_pdpt(Virt_To_Phy(virtual), PAGE_KERNEL_Dir));
	}

	// color_printk(YELLOW, BLACK, "2:%#018lx\t%#018lx\n", (u64_t)tmp, (u64_t)*tmp);

	tmp = Phy_To_Virt((u64_t *)(*tmp & (~0xfffUL)) + (((u64_t)IOAPIC_addr >> PAGE_2M_SHIFT) & 0x1ff));
	set_pdt(tmp, mk_pdt(ioapic_map.physical_address, PAGE_KERNEL_Page | PAGE_PWT | PAGE_PCD));

	// color_printk(YELLOW, BLACK, "3:%#018lx\t%#018lx\n", (u64_t)tmp, (u64_t)*tmp);

	// color_printk(BLUE, BLACK, "ioapic_map.physical_address:%#010x\t\t\n", ioapic_map.physical_address);
	// color_printk(BLUE, BLACK, "ioapic_map.virtual_address:%#018lx\t\t\n", (u64_t)ioapic_map.virtual_index_address);

	flush_tlb();
}
#endif
// init local apic
void Local_APIC_init()
{
	u32_t x, y;
	u32_t a, b, c, d;

	// check APIC & x2APIC support
	get_cpuid(1, 0, &a, &b, &c, &d);
	// void get_cpuid(u32_t Mop,u32_t Sop,u32_t * a,u32_t * b,u32_t * c,u32_t * d)
	color_printk(WHITE, BLACK, "CPUID\t01,eax:%#010x,ebx:%#010x,ecx:%#010x,edx:%#010x\n", a, b, c, d);
	if ((1UL << 9) & d)
		color_printk(WHITE, BLACK, "HW support APIC&xAPIC\t");
	else
		color_printk(WHITE, BLACK, "HW NO support APIC&xAPIC\t");
	if ((1Ul << 21) & c)
		color_printk(WHITE, BLACK, "HW support x2APIC\n");
	else
		color_printk(WHITE, BLACK, "HW NO support x2APIC\n");

	// enable xAPIC & x2APIC, rcx:选择操作的msr寄存器组中的地址, 修改IA32_APIC_BASE
	// rdmsr 结果存放在 edx | ecx, bts修改标志enable APIC
	// 再次读，存放入 x, y中
	__asm__ __volatile__(
		"movq 	$0x1b,	%%rcx	\n\t"
		"rdmsr	\n\t"
		"bts	$10,	%%rax	\n\t"
		"bts	$11,	%%rax	\n\t"
		"wrmsr	\n\t"
		"movq 	$0x1b,	%%rcx	\n\t"
		"rdmsr	\n\t"
		: "=a"(x), "=d"(y)
		:
		: "memory");
	color_printk(WHITE, BLACK, "eax:%#010x,edx:%#010x\t", x, y);
	if (x & 0xc00)
		color_printk(WHITE, BLACK, "xAPIC & x2APIC enabled\n");

	// enable SVR[8],  SVR[12] =  开启 Local APIC 和 禁止广播EOI消息(模拟器不支持禁止这东西)
	__asm__ __volatile__(
		"movq 	$0x80f,	%%rcx	\n\t"
		"rdmsr	\n\t"
		"bts	$8,	%%rax	\n\t"
		"bts	$12,	%%rax\n\t"
		"wrmsr	\n\t"
		"movq 	$0x80f,	%%rcx	\n\t"
		"rdmsr	\n\t"
		: "=a"(x), "=d"(y)
		:
		: "memory");
	color_printk(WHITE, BLACK, "eax:%#010x,edx:%#010x\t", x, y);
	if (x & 0x100)
		color_printk(WHITE, BLACK, "SVR[8] enabled\n");
	if (x & 0x1000)
		color_printk(WHITE, BLACK, "SVR[12] enabled\n");

	// get local APIC ID
	__asm__ __volatile__("movq $0x802,	%%rcx	\n\t"
						 "rdmsr	\n\t"
						 : "=a"(x), "=d"(y)
						 :
						 : "memory");
	color_printk(WHITE, BLACK, "eax:%#010x,edx:%#010x\tx2APIC ID:%#010x\n", x, y, x);

	// get local APIC version
	__asm__ __volatile__("movq $0x803,	%%rcx	\n\t"
						 "rdmsr	\n\t"
						 : "=a"(x), "=d"(y)
						 :
						 : "memory");
	color_printk(WHITE, BLACK, "local APIC Version:%#010x,Max LVT Entry:%#010x,SVR(Suppress EOI Broadcast):%#04x\t", x & 0xff, (x >> 16 & 0xff) + 1, x >> 24 & 0x1);
	if ((x & 0xff) < 0x10)
		color_printk(WHITE, BLACK, "82489DX discrete APIC\n");
	else if (((x & 0xff) >= 0x10) && ((x & 0xff) <= 0x15))
		color_printk(WHITE, BLACK, "Integrated APIC\n");

	// mask all LVT,//the virtual machine of bochs, vbox and qemu does not support CMCI register
	__asm__ __volatile__(
		//"movq 	$0x82f,	%%rcx	\n\t" // CMCI
		//"wrmsr	\n\t"				  // 这条指令会引起QEMU异常
		"movq 	$0x832,	%%rcx	\n\t" // Timer
		"wrmsr	\n\t"
		"movq 	$0x833,	%%rcx	\n\t" // Thermal Monitor
		"wrmsr	\n\t"
		"movq 	$0x834,	%%rcx	\n\t" // Performance Counter
		"wrmsr	\n\t"
		"movq 	$0x835,	%%rcx	\n\t" // LINT0
		"wrmsr	\n\t"
		"movq 	$0x836,	%%rcx	\n\t" // LINT1
		"wrmsr	\n\t"
		"movq 	$0x837,	%%rcx	\n\t" // Error
		"wrmsr	\n\t"
		:
		: "a"(0x10000), "d"(0x00)
		: "memory");
	color_printk(GREEN, BLACK, "Mask ALL LVT\n");

	// TPR 任务优先级寄存器
	__asm__ __volatile__(
		"movq 	$0x808,	%%rcx	\n\t"
		"rdmsr	\n\t"
		: "=a"(x), "=d"(y)
		:
		: "memory");
	color_printk(GREEN, BLACK, "Set LVT TPR:%#010x\t", x);

	// PPR 处理器优先级寄存器
	__asm__ __volatile__(
		"movq 	$0x80a,	%%rcx	\n\t"
		"rdmsr	\n\t"
		: "=a"(x), "=d"(y)
		:
		: "memory");
	color_printk(GREEN, BLACK, "Set LVT PPR:%#010x\n", x);
}

// init IOAPIC
void IOAPIC_init()
{
	s32_t i;

	// get I/O APIC ID
	*ioapic_map.virtual_index_address = 0x00;
	io_mfence();
	*ioapic_map.virtual_data_address = 0x0f000000; // Set IO_APIC ID
	io_mfence();
	color_printk(GREEN, BLACK, "Get IOAPIC ID REG:%#010x,ID:%#010x\n", *ioapic_map.virtual_data_address, *ioapic_map.virtual_data_address >> 24 & 0xf);
	io_mfence();

	//	I/O APIC	Version
	*ioapic_map.virtual_index_address = 0x01;
	io_mfence();
	color_printk(GREEN, BLACK, "Get IOAPIC Version REG:%#010x,MAX redirection enties:%#08d\n", *ioapic_map.virtual_data_address, ((*ioapic_map.virtual_data_address >> 16) & 0xff) + 1);

	// 初始化RTE表项
	for (i = 0x10; i < 0x40; i += 2)
		ioapic_rte_write(i, 0x10020 + ((i - 0x10) >> 1));

	color_printk(GREEN, BLACK, "I/O APIC Redirection Table Entries Set Finished.\n");
}

void APIC_IOAPIC_init()
{
	//	init trap abort fault
	s32_t i;
	u32_t x;
	u32_t *p;

	memset(interrupt_desc, 0, sizeof(irq_desc_T) * NR_IRQS);

	// 为操作IO_APIC寄存器作铺垫
	IOAPIC_pagetable_remap();

	for (i = 32; i < 56; i++)
	{
		set_intr_gate(i, 2, interrupt[i - 32]); // 启动了ist机制
	}

	// mask 8259A
	color_printk(GREEN, BLACK, "MASK 8259A\n");
	io_out8(0x21, 0xff);
	io_out8(0xa1, 0xff);

	// enable IMCR, 使得处理器只接受APIC的中断请求信号
	io_out8(0x22, 0x70);
	io_out8(0x23, 0x01);

	// init local apic
	Local_APIC_init();

	// init ioapic
	IOAPIC_init();

	// get RCBA address(RCBA = Root Complex Address Register)
	// io_out32(0xcf8, 0x8000f8f0); // RCBA位于PCI总线0的31号设备0号功能的F0h偏移处。这一句详情看书
	// x = io_in32(0xcfc);
	// color_printk(RED, BLACK, "Root Complex Address Register:%#010x\n", x);
	// x = x & 0xffffc000;
	// color_printk(RED, BLACK, "Chipset registers Address:%#010x\n", x); // 芯片组寄存器的物理基地址

	// // get OIC address
	// if (x > 0xfec00000 && x < 0xfee00000)
	// {
	// 	// OIC 寄存器位于芯片组配置寄存器组的31FEh地址便宜处
	// 	p = (u32_t *)Phy_To_Virt(x + 0x31feUL);
	// }

	// // enable IOAPIC
	// x = (*p & 0xffffff00) | 0x100;
	// io_mfence();
	// *p = x;
	// io_mfence();

	// enable IF eflages
	sti();
}

void do_IRQ(pt_regs_t *regs, u64_t nr) // regs:rsp,nr
{
	irq_desc_T *irq = &interrupt_desc[nr - 32];

	// color_printk(BLUE, WHITE, "rip:%#018lx,  rsp:%#018lx\n", regs->rip, regs->rsp);

	if (irq->handler != NULL)
		irq->handler(nr, irq->parameter, regs); // 执行中断上半部处理程序

	if (irq->controller != NULL && irq->controller->ack != NULL)
		irq->controller->ack(nr); // 向中断控制器发生应答消息,   向Local APIC的EOI寄存器写入数值00, 以通知控制器中断处理过程结束
}

// 修改向量号为irq对应I/O中断定向投递寄存器组的屏蔽标志,使其激活
void IOAPIC_enable(u64_t irq)
{
	u64_t value = 0;
	value = ioapic_rte_read((irq - 32) * 2 + 0x10);
	value = value & (~0x10000UL);
	ioapic_rte_write((irq - 32) * 2 + 0x10, value);
}

// 修改向量号为irq对应I/O中断定向投递寄存器组的屏蔽标志,使其失效
void IOAPIC_disable(u64_t irq)
{
	u64_t value = 0;
	value = ioapic_rte_read((irq - 32) * 2 + 0x10);
	value = value | 0x10000UL;
	ioapic_rte_write((irq - 32) * 2 + 0x10, value);
}

// 给向量号为irq的中断创建I/O中断向量投递寄存器
u64_t IOAPIC_install(u64_t irq, void *arg)
{
	struct IO_APIC_RET_entry *entry = (struct IO_APIC_RET_entry *)arg;
	ioapic_rte_write((irq - 32) * 2 + 0x10, *(u64_t *)entry);

	return 1;
}

// 回收分配给向量号irq中断创建的I/O中断向量投递寄存器
void IOAPIC_uninstall(u64_t irq)
{
	ioapic_rte_write((irq - 32) * 2 + 0x10, 0x10000UL);
}

// 电平触发后给中断控制器的ack(EOI)
void IOAPIC_level_ack(u64_t irq)
{
	__asm__ __volatile__("movq	$0x00,	%%rdx	\n\t"
						 "movq	$0x00,	%%rax	\n\t"
						 "movq 	$0x80b,	%%rcx	\n\t"
						 "wrmsr	\n\t" ::: "memory");

	*ioapic_map.virtual_EOI_address = irq;
}

// 边沿触发后给中断触发器的ack(EOI)
void IOAPIC_edge_ack(u64_t irq)
{
	__asm__ __volatile__("movq	$0x00,	%%rdx	\n\t"
						 "movq	$0x00,	%%rax	\n\t"
						 "movq 	$0x80b,	%%rcx	\n\t"
						 "wrmsr	\n\t" ::: "memory");
}

void Local_APIC_edge_level_ack(u64_t irq)
{
	__asm__ __volatile__("movq	$0x00,	%%rdx	\n\t"
						 "movq	$0x00,	%%rax	\n\t"
						 "movq 	$0x80b,	%%rcx	\n\t"
						 "wrmsr	\n\t" ::: "memory");
}