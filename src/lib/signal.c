#include "signal.h"
#include "ptrace.h"
#include "task.h"

typedef struct signal_frame {
    unsigned long restorer;  //恢复函数
    long signum;
    long blocked; // 屏蔽位图

    // 依据ABI保存调用时的寄存器，用于恢复执行信号之前的代码
    unsigned long rdi;
    
    unsigned long rax;  // 按照系统调用退出要求，保存栈帧
    unsigned long r10;
    unsigned long r11;
    
    unsigned long rbx; // ABI 规范要求保存
    unsigned long r12;
    unsigned long r15;

    unsigned long rflags; // 返回地址
    unsigned long rip;
}signal_frame_T;

// 设置信号
sighadler_t sys_signal(long signum, sighadler_t  hander, void (*restorer)(void))
{
    sigaction_T* sa = (current->sigaction + signum);
    sa->sa_handler = hander;
    sa->sa_restorer = restorer;

    return NULL;
}

// 发送信号
int sys_kill(long pid, int signum)
{
    struct task_struct *tsk = NULL;

	for (tsk = init_task_union.task.next; tsk != &init_task_union.task; tsk = tsk->next)
	{
		if (tsk->pid == pid)
			break;
	}

    if(tsk == NULL)
        return;

    tsk->signal |= (1 << signum); // 设置信号位图-表示接收信号

   //  current->signal |= (1 << signum); // 设置信号位图-表示接收信号
    return 0;
}

void do_signal(struct pt_regs* regs)
{
    if(current->signal == 0)
        return;
    
    sigaction_T* sa;
    signal_frame_T sf;
    long signal = current->signal;
    long signr = 1;

    for(; signr < NSIG; signr++)  
        if((signal >> signr) & 1) {
            signal = signal & (~(1 << signr));
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
    sf.restorer = sa->sa_restorer;
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
        regs->rip = (unsigned long)sa->sa_handler;
    } else { // 从系统调用退出
        regs->r11 = (regs->r11 - sizeof(signal_frame_T));
        sf.rip = regs->r10;
        copy_to_user(&sf, (void*)(regs->r11), sizeof(signal_frame_T));
        regs->r10 = (unsigned long)sa->sa_handler;
    }

}


