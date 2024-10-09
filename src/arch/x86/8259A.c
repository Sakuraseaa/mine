#include "toolkit.h"
#include "mmkit.h"
#include "kernelkit.h"

extern irq_desc_T interrupt_desc[NR_IRQS];
// 修改向量号为irq对应I/O中断定向投递寄存器组的屏蔽标志,使其激活
void IOAPIC_enable(unsigned long irq)
{
}

// 修改向量号为irq对应I/O中断定向投递寄存器组的屏蔽标志,使其失效
void IOAPIC_disable(unsigned long irq)
{
}

// 给向量号为irq的中断创建I/O中断向量投递寄存器
unsigned long IOAPIC_install(unsigned long irq, void *arg)
{
	return 0;
}

// 回收分配给向量号irq中断创建的I/O中断向量投递寄存器
void IOAPIC_uninstall(unsigned long irq)
{
}

// 电平触发后给中断控制器的ack(EOI)
void IOAPIC_level_ack(unsigned long irq)
{
	io_out8(0x20, 0x20);
	io_out8(0xA0, 0x20);
}

// 边沿触发后给中断触发器的ack(EOI)
void IOAPIC_edge_ack(unsigned long irq)
{
	io_out8(0x20, 0x20);
	io_out8(0xA0, 0x20);
}
void init_8259A()
{
	int i;
	for (i = 32; i < 56; i++)					// 64位模式中 中断发送时候，ss-rsp是需要强制保存的
		set_intr_gate(i, 0, interrupt[i - 32]); // 这里设置ist为0，在中断切换栈的时候，使用原有的栈切换机制
	// 32位是只有特权级改变的时候，才会栈切换
	//  color_printk(RED, BLACK, "8259A init \n");

	// 8259A-master	ICW1-4
	io_out8(0x20, 0x11);
	io_out8(0x21, 0x20);
	io_out8(0x21, 0x04);
	io_out8(0x21, 0x01);

	// 8259A-slave	ICW1-4
	io_out8(0xa0, 0x11);
	io_out8(0xa1, 0x28);
	io_out8(0xa1, 0x02);
	io_out8(0xa1, 0x01);

	// 8259A-M/S	OCW1 0xf9, 会关闭了时钟中断, Old::0xf8
	io_out8(0x21, 0xe0); // 开启了两个串口的中断
	io_out8(0xa1, 0x2f);

	// enable IF eflages
	// sti();
}

/**
 * @brief
 *
 * @param regs 栈基址
 * @param nr 中断号
 */
void do_IRQ(pt_regs_t *regs, unsigned long nr) // regs,nr
{
	irq_desc_T *irq = &interrupt_desc[nr - 32];
	// color_printk(BLUE, WHITE, "rip:%#018lx,  rsp:%#018lx\n", regs->rip, regs->rsp);
	//  执行中断进程上半部
	if (irq->handler != NULL)
		irq->handler(nr, irq->parameter, regs);

	// 发送EOI
	if (irq->controller != NULL && irq->controller->ack != NULL)
		irq->controller->ack(nr);
}
