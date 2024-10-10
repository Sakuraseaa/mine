#include "syskit.h"
#include "kernelkit.h"

typedef struct signal_frame {
    u64_t restorer;  //恢复函数
    s64_t signum;
    s64_t blocked; // 屏蔽位图

    // 依据ABI保存调用时的寄存器，用于恢复执行信号之前的代码
    u64_t rdi;
    
    u64_t rax;  // 按照系统调用退出要求，保存栈帧
    u64_t r10;
    u64_t r11;
    
    u64_t rbx; // ABI 规范要求保存
    u64_t r12;
    u64_t r15;

    u64_t rflags; // 返回地址
    u64_t rip;
}signal_frame_T;

// 设置信号
sighadler_t sys_signal(s64_t signum, sighadler_t  hander, void (*restorer)(void))
{
    sigaction_t* sa = (current->sigaction + signum);
    sa->sa_handler = hander;
    sa->sa_restorer = restorer;

    return nullptr;
}

// 发送信号
s32_t sys_kill(s64_t pid, s32_t signum)
{
    task_t *tsk = nullptr;

	for (tsk = init_task_union.task.next; tsk != &init_task_union.task; tsk = tsk->next)
	{
		if (tsk->pid == pid)
			break;
	}

    if(tsk == nullptr)
        return -1;

    tsk->signal |= (1UL << signum); // 设置信号位图-表示接收信号

   //  current->signal |= (1 << signum); // 设置信号位图-表示接收信号
    return 0;
}

void do_signal(pt_regs_t* regs)
{
    if(current->signal == 0)
        return;
    
    sigaction_t* sa;
    signal_frame_T sf;
    s64_t signal = current->signal;
    s64_t signr = 1;

    for(; signr < NSIG; signr++)  
        if((signal >> signr) & 1) {
            signal = signal & (~(1UL << signr));
            break;
        }
    
    current->signal = signal;
    
    if((current->blocked >> signr) & 1) // 屏蔽该信号
        return;
    
    sa = (current->sigaction + signr);

    if(sa->sa_handler == SIG_IGN) // 忽略该信号
        return;

    if(sa->sa_handler == SIG_DFL) {
        do_exit(signr);
    }

    // 为信号的执行准备栈帧
    sf.blocked = current->blocked;
    sf.restorer = (u64_t)sa->sa_restorer;
    sf.signum = signr;
    sf.rflags = regs->rflags;
    sf.r12 = regs->r12;
    sf.r15 = regs->r15;
    sf.rbx = regs->rbx;
    sf.rdi = regs->rdi;
    sf.r10 = regs->r10;
    sf.r11 = regs->r11;
    sf.rax = regs->rax;

    regs->rdi = signr;
    if(regs->rip) { // 从中断异常退出
        regs->rsp = (regs->rsp - sizeof(signal_frame_T));
        sf.rip = regs->rip;
        copy_to_user(&sf, (void*)(regs->rsp), sizeof(signal_frame_T));
        regs->rip = (u64_t)sa->sa_handler;
    } else { // 从系统调用退出
        regs->r11 = (regs->r11 - sizeof(signal_frame_T));
        sf.rip = regs->r10;
        copy_to_user(&sf, (void*)(regs->r11), sizeof(signal_frame_T));
        regs->r10 = (u64_t)sa->sa_handler;
    }

}