#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__
#include "linkage.h"
#include "ptrace.h"

typedef struct hw_int_type
{
    void (*enable)(unsigned long irq);                      // 使能中断操作接口
    void (*disable)(unsigned long irq);                     // 禁止中断操作接口
    unsigned long (*install)(unsigned long irq, void *arg); // 安装中断操作接口
    void (*uninstall)(unsigned long irq);                   // 卸载中断操作接口

    void (*ack)(unsigned long irq); // 应答中断操作接口
} hw_int_controller;

typedef struct
{
    hw_int_controller *controller; // 中断的使能，禁止，应答, 卸载操作结构体

    char *irq_name;                                                                   // 中断名
    unsigned long parameter;                                                          // 中断处理函数的参数
    void (*handler)(unsigned long nr, unsigned long parameter, pt_regs_t *regs); // 中断处理函数
    unsigned long flags;                                                              // 自定义标志位
} irq_desc_T;

#define NR_IRQS 24
irq_desc_T interrupt_desc[NR_IRQS] = {0};

extern void (*interrupt[24])(void);
extern void do_IRQ(pt_regs_t *regs, unsigned long nr);

// 中断注册函数，根据中断向量号把中断处理函数，参数以及相关结果和数据赋值到对应irq_desc_T
int register_irq(unsigned long irq, void *arg, void (*handler)(unsigned long nr, unsigned long parameter, pt_regs_t *regs),
                 unsigned long parameter, hw_int_controller *controller, char *irq_name);
int unregister_irq(unsigned long irq);
#endif