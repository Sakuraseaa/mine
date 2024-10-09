#ifndef __8259A_H__
#define __8259A_H__

// // 修改向量号为irq对应I/O中断定向投递寄存器组的屏蔽标志,使其激活
// void IOAPIC_enable(u64_t irq);

// // 修改向量号为irq对应I/O中断定向投递寄存器组的屏蔽标志,使其失效
// void IOAPIC_disable(u64_t irq);

// // 给向量号为irq的中断创建I/O中断向量投递寄存器
// u64_t IOAPIC_install(u64_t irq, void *arg);

// // 回收分配给向量号irq中断创建的I/O中断向量投递寄存器
// void IOAPIC_uninstall(u64_t irq);

// // 电平触发后给中断控制器的ack(EOI)
// void IOAPIC_level_ack(u64_t irq);
// // 边沿触发后给中断触发器的ack(EOI)
// void IOAPIC_edge_ack(u64_t irq);
void init_8259A();
#endif
