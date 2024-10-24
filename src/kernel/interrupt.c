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
#include "toolkit.h"
#include "kernelkit.h"
#include "mmkit.h"

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
                 str_t irq_name)
{
    irq_desc_t *p = &interrupt_desc[irq - 32];

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
    irq_desc_t *p = &interrupt_desc[irq - 32];

    p->controller->disable(irq);
    p->controller->uninstall(irq);

    p->controller = nullptr;
    p->irq_name = nullptr;
    p->parameter = 0;
    p->flags = 0;
    p->handler = nullptr;

    return 1;
}

/**
 * @brief 缺页处理
 *配合exec
 * @param address The address that cause the exception
 */
extern s64_t krluserspace_accessfailed(adr_t fairvadrs);

static void vma_load_filedata(vma_to_file_t* vtft, adr_t fault_vadrs, adr_t vma_start_vadr, kvmemcbox_t * kbox)
{
    if (vtft == nullptr) {
        return;
    }

    file_t* task_file = vtft->vtf_file;
    fault_vadrs = (fault_vadrs & PAGE_4K_MASK);
    u64_t cur_load_position = fault_vadrs - vma_start_vadr;

    s64_t cur_start_load_size = (vma_start_vadr + vtft->vtf_size - fault_vadrs) > PAGE_4K_SIZE ? PAGE_4K_SIZE : (vma_start_vadr + vtft->vtf_size - fault_vadrs);
    assert(cur_start_load_size > 0);

    vtft->vtf_alread_load_size += cur_start_load_size;

    task_file->f_ops->lseek(task_file, vtft->vtf_position + cur_load_position, SEEK_SET);
    task_file->f_ops->read(task_file, (buf_t)fault_vadrs, cur_start_load_size, &task_file->position);

    kbox->kmb_filenode = task_file; // 给页面盒子设置他所管理的文件描述符
    return;
}

sint_t vma_map_fairvadrs_core(mmdsc_t *mm, adr_t vadrs)
{
    sint_t rets = FALSE;
    adr_t phyadrs = NULL;
    virmemadrs_t *vma = &mm->msd_virmemadrs;
    kmvarsdsc_t *kmvd = nullptr;
    kvmemcbox_t *kmbox = nullptr;
    // knl_spinlock(&vma->vs_lock);

    //查找对应的kmvarsdsc_t结构, 没有找到. 说明虚拟地址不存在，直接返回
    kmvd = vma_map_find_kmvarsdsc(vma, vadrs);
    if (nullptr == kmvd) {
        rets = -EFAULT;
        goto out;
    }

    if (kmvd->kva_flgs == 1)
        vadrs = PAGE_4K_ALIGN(vadrs) - 0x1000;

    //返回kmvarsdsc_t结构下对应kvmemcbox_t结构
    kmbox = vma_map_retn_kvmemcbox(kmvd);
    if (nullptr == kmbox) {
        rets = -ENOMEM;
        goto out;
    }
    //分配物理内存页面并建立MMU页表
    phyadrs = vma_map_phyadrs(mm, kmvd, vadrs, (0 | PML4E_US | PML4E_RW | PML4E_P));
    if (NULL == phyadrs) {
        rets = -ENOMEM;
        goto out;
    }

    vma_load_filedata(kmvd->kva_vir2file, vadrs, kmvd->kva_start, kmbox);
    rets = EOK;

out:
    //   knl_spinunlock(&vma->vs_lock);
    return rets;
}

//缺页异常处理接口
sint_t vma_map_fairvadrs(mmdsc_t *mm, adr_t vadrs)
{//对参数进行检查
    if ((0x1000 > vadrs) || (USER_VIRTUAL_ADDRESS_END < vadrs) || (nullptr == mm))
    {
        return -EPARAM;
    }
    //进行缺页异常的核心处理
    return vma_map_fairvadrs_core(mm, vadrs);
}

//由异常分发器调用的接口
sint_t krluserspace_accessfailed(adr_t fairvadrs)
{//这里应该获取当前进程的mm，但是现在我们没有进程，才initmmadrsdsc代替
    mmdsc_t* mm = current->mm;
    //应用程序的虚拟地址不可能大于USER_VIRTUAL_ADDRESS_END
    if(USER_VIRTUAL_ADDRESS_END < fairvadrs)
    {
        return -EACCES;
    }
    return vma_map_fairvadrs(mm, fairvadrs);
}

s64_t do_no_page(u64_t virtual_address)
{
    color_printk(GREEN, BLACK,"  proc:%d, no page %#0lx", current->pid, virtual_address);
    return krluserspace_accessfailed(virtual_address);
}


/**
 * @brief 写保护页面处理
 *    配合fork()
 * 
 * @param virtual_address The addresss that caused the exception
 * @return u64_t 
 */
static u64_t do_wp_page_core(mmdsc_t *mm, adr_t vadrs) {
    u64_t rets = EOK;
    adr_t phyadrs = NULL;
    virmemadrs_t *vma_list = &mm->msd_virmemadrs;
    kmvarsdsc_t *kmvd = nullptr;
    kvmemcbox_t *kmbox = nullptr;
    msadsc_t* msa = nullptr;
    void* swap;
    // knl_spinlock(&vma->vs_lock);

    vadrs = (PAGE_4K_MASK & vadrs);

    //查找对应的kmvarsdsc_t结构, 没有找到. 说明虚拟地址不存在，直接返回
    kmvd = vma_map_find_kmvarsdsc(vma_list, vadrs);
    if (nullptr == kmvd) {
        rets = -EFAULT;
        goto out;
    }

    //返回kmvarsdsc_t结构下对应kvmemcbox_t结构
    kmbox = vma_map_retn_kvmemcbox(kmvd);
    if (nullptr == kmbox) {
        rets = -ENOMEM;
        goto out;
    }
    //分配物理内存页面并建立MMU页表
    phyadrs = hal_mmu_virtophy(&mm->msd_mmu, vadrs);
    
    msa = find_msa_from_pagebox(kmbox, phyadrs);
    
    if(msa->md_phyadrs.paf_shared == PAF_SHARED && msa->md_cntflgs.mf_refcnt > 1) //todo 交换动作
    {
        swap = knew(PAGE_4K_SIZE, 0);
        
        memcpy((void*)vadrs, swap, PAGE_4K_SIZE);

        vma_map_phyadrs(mm, kmvd, vadrs, (0 | PML4E_US | PML4E_RW | PML4E_P));
        
        memcpy(swap, (void*)vadrs, PAGE_4K_SIZE);
        
        msa->md_phyadrs.paf_shared = PAF_NO_SHARED;
        msa->md_cntflgs.mf_refcnt --;

        kdelete(swap, PAGE_4K_SIZE);
    }
    else
    {
        hal_mmu_transform(&mm->msd_mmu, vadrs, msadsc_ret_addr(msa), (0 | PML4E_US | PML4E_RW | PML4E_P));
    }
    
out:
    return rets;
}

u64_t do_wp_page(u64_t virtual_address) {
    
    color_printk(GREEN, BLACK,"  proc:%d, wp page  %#0lx", current->pid, virtual_address);
    do_wp_page_core(current->mm, virtual_address);

    // flush_tlb_one(virtual_address);
    flush_tlb();
    return 0;
}

