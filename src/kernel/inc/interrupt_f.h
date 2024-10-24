#ifndef _INTERRUPT_F_H_
#define _INTERRUPT_F_H_

// 中断注册函数，根据中断向量号把中断处理函数，参数以及相关结果和数据赋值到对应irq_desc_T
s32_t register_irq(u64_t irq, void *arg, void (*handler)(u64_t nr, u64_t parameter, pt_regs_t *regs),
                 u64_t parameter, hw_int_controller *controller, str_t irq_name);
s32_t unregister_irq(u64_t irq);
u64_t do_wp_page(u64_t virtual_address);
s64_t do_no_page(u64_t virtual_address);
#endif // _s32_tERRUPT_F_H_