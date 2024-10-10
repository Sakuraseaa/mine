#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

typedef struct hw_int_type
{
    void (*enable)(u64_t irq);                      // 使能中断操作接口
    void (*disable)(u64_t irq);                     // 禁止中断操作接口
    u64_t (*install)(u64_t irq, void *arg); // 安装中断操作接口
    void (*uninstall)(u64_t irq);                   // 卸载中断操作接口

    void (*ack)(u64_t irq); // 应答中断操作接口
} hw_int_controller;

typedef struct
{
    hw_int_controller *controller; // 中断的使能，禁止，应答, 卸载操作结构体

    str_t irq_name;                                                                   // 中断名
    u64_t parameter;                                                          // 中断处理函数的参数
    void (*handler)(u64_t nr, u64_t parameter, pt_regs_t *regs); // 中断处理函数
    u64_t flags;                                                              // 自定义标志位
} irq_desc_T;

#define NR_IRQS 24
irq_desc_T interrupt_desc[NR_IRQS] = {0};

extern void (*interrupt[24])(void);
extern void do_IRQ(pt_regs_t *regs, u64_t nr);

#endif