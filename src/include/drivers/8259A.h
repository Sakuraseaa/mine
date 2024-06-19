#ifndef __8259A_H__
#define __8259A_H__

#include "linkage.h"
#include "ptrace.h"
// // 修改向量号为irq对应I/O中断定向投递寄存器组的屏蔽标志,使其激活
// void IOAPIC_enable(unsigned long irq);

// // 修改向量号为irq对应I/O中断定向投递寄存器组的屏蔽标志,使其失效
// void IOAPIC_disable(unsigned long irq);

// // 给向量号为irq的中断创建I/O中断向量投递寄存器
// unsigned long IOAPIC_install(unsigned long irq, void *arg);

// // 回收分配给向量号irq中断创建的I/O中断向量投递寄存器
// void IOAPIC_uninstall(unsigned long irq);

// // 电平触发后给中断控制器的ack(EOI)
// void IOAPIC_level_ack(unsigned long irq);
// // 边沿触发后给中断触发器的ack(EOI)
// void IOAPIC_edge_ack(unsigned long irq);
void init_8259A();
#endif
