/**
 * @file interrupt.c 重写这段程序，使得中断入口函数 使用汇编完成。
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-06-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "types.h"
#include "interrupt.h"
#include "linkage.h"
#include "lib.h"
#include "printk.h"
#include "memory.h"
#include "gate.h"

// 这里多余的push了2下，为什么
// 依照regs结构体，对于中断补充压入rax充当:错误码, 中断处理函数地址，.....rax
#define SAVE_ALL             \
    "cld;			\n\t"            \
    "pushq	%rax;		\n\t"      \
    "pushq	%rax;		\n\t"      \
    "movq	%es,	%rax;	\n\t"   \
    "pushq	%rax;		\n\t"      \
    "movq	%ds,	%rax;	\n\t"   \
    "pushq	%rax;		\n\t"      \
    "xorq	%rax,	%rax;	\n\t"  \
    "pushq	%rbp;		\n\t"      \
    "pushq	%rdi;		\n\t"      \
    "pushq	%rsi;		\n\t"      \
    "pushq	%rdx;		\n\t"      \
    "pushq	%rcx;		\n\t"      \
    "pushq	%rbx;		\n\t"      \
    "pushq	%r8;		\n\t"       \
    "pushq	%r9;		\n\t"       \
    "pushq	%r10;		\n\t"      \
    "pushq	%r11;		\n\t"      \
    "pushq	%r12;		\n\t"      \
    "pushq	%r13;		\n\t"      \
    "pushq	%r14;		\n\t"      \
    "pushq	%r15;		\n\t"      \
    "movq	$0x10,	%rdx;	\n\t" \
    "movq	%rdx,	%ds;	\n\t"   \
    "movq	%rdx,	%es;	\n\t"

// ## 用于连接两个宏值， # 将其后的内容强制转换为字符串
#define IRQ_NAME2(nr) nr##_interrupt(void)
#define IRQ_NAME(nr) IRQ_NAME2(IRQ##nr)

// ss,rsp, rflags, cs, rip已经被CPU自动压入, 多余的2次push是为了适应do_IRQ函数的strct pt_regs结构体

// leaq 的目的是主动压入了返回地址, 因为使用的是jmp指令指向函数
// rsp栈作为第一参数进入rdi寄存器，中断号作为第二参数给rsi寄存器
// 这段宏代码定义中断处理程序的入口部分
#define Build_IRQ(nr)                                                      \
    void IRQ_NAME(nr);                                                     \
    __asm__( ".section .text	\n\t"              \
            SYMBOL_NAME_STR(IRQ) #nr "_interrupt:    \n\t"                 \
                                     "pushq $0x00 \n\t" SAVE_ALL           \
                                     "movq %rsp, %rdi \n\t"                \
                                     "leaq ret_from_intr(%rip), %rax \n\t" \
                                     "pushq %rax \n\t"                     \
                                     "movq $" #nr ", %rsi \n\t"            \
                                     "jmp do_IRQ \n\t")
// 这里定义了处理中断的函数
Build_IRQ(0x20); // IRQ0x20_interrupt
Build_IRQ(0x21);
Build_IRQ(0x22);
Build_IRQ(0x23);
Build_IRQ(0x24);
Build_IRQ(0x25);
Build_IRQ(0x26);
Build_IRQ(0x27);
Build_IRQ(0x28);
Build_IRQ(0x29);
Build_IRQ(0x2a);
Build_IRQ(0x2b);
Build_IRQ(0x2c);
Build_IRQ(0x2d);
Build_IRQ(0x2e);
Build_IRQ(0x2f);
Build_IRQ(0x30);
Build_IRQ(0x31);
Build_IRQ(0x32);
Build_IRQ(0x33);
Build_IRQ(0x34);
Build_IRQ(0x35);
Build_IRQ(0x36);
Build_IRQ(0x37);


/*函数指针数组, 每个元素都指向由宏函数Build_IRQ定义的一个中断处理函数入口*/
void (*interrupt[24])(void) =
    {
        IRQ0x20_interrupt,
        IRQ0x21_interrupt,
        IRQ0x22_interrupt,
        IRQ0x23_interrupt,
        IRQ0x24_interrupt,
        IRQ0x25_interrupt,
        IRQ0x26_interrupt,
        IRQ0x27_interrupt,
        IRQ0x28_interrupt,
        IRQ0x29_interrupt,
        IRQ0x2a_interrupt,
        IRQ0x2b_interrupt,
        IRQ0x2c_interrupt,
        IRQ0x2d_interrupt,
        IRQ0x2e_interrupt,
        IRQ0x2f_interrupt,
        IRQ0x30_interrupt,
        IRQ0x31_interrupt,
        IRQ0x32_interrupt,
        IRQ0x33_interrupt,
        IRQ0x34_interrupt,
        IRQ0x35_interrupt,
        IRQ0x36_interrupt,
        IRQ0x37_interrupt,
};

/**
 * @brief Init Struct Irq_desc_T, 中断注册函数
 *
 * @param irq 中断号
 * @param arg IO_APIC的中断向量表项
 * @param handler 中断处理函数
 * @param parameter 中断处理函数的参数
 * @param controller 中断控制器
 * @param irq_name 中断名
 * @return int 运行成功, 返回1
 */
s32_t register_irq(u64_t irq,
                 void *arg,
                 void (*handler)(u64_t nr, u64_t parameter, pt_regs_t *regs),
                 u64_t parameter,
                 hw_int_controller *controller,
                 char *irq_name)
{
    irq_desc_T *p = &interrupt_desc[irq - 32];

    p->controller = controller;
    p->irq_name = irq_name;
    p->parameter = parameter;
    p->flags = 0;
    p->handler = handler;

    p->controller->install(irq, arg);
    p->controller->enable(irq);

    return 1;
}

// 中断销毁函数
s32_t unregister_irq(u64_t irq)
{
    irq_desc_T *p = &interrupt_desc[irq - 32];

    p->controller->disable(irq);
    p->controller->uninstall(irq);

    p->controller = NULL;
    p->irq_name = NULL;
    p->parameter = 0;
    p->flags = 0;
    p->handler = NULL;

    return 1;
}
