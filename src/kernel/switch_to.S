.section .text
.globl switch_to

thread = 0x38 // PCB结构体中 thread(线程栈) 的偏移
//===================thread中各成员变量偏移=======================
rsp0 =	0x00
RIP	=	0x08
RSP	=	0x10
FS	=	0x18
GS	=	0x20
CR2	=	0x28
trap_nr = 0x30
error_code = 0x38
// ==============================================================

switch_to: 
    pushq   %rbp
    pushq   %rax
    pushq   %rcx
    pushq   %rdx
    pushq   %rbx

    movq    thread(%rdi), %rcx // prev
    movq    thread(%rsi), %rdx // next

    movq %rsp, RSP(%rcx)    // 保存前一个进程的栈指针，栈中又函数调用逻辑和栈变量
    movq RSP(%rdx), %rsp    // 加载新进程的栈
    
    leaq back(%rip), %rax   // 记录前一个进程再次运行的地址
    movq %rax, RIP(%rcx)

    pushq  RIP(%rdx)    // 新进程要执行程序的地址，入栈
    jmp __switch_to
    
back:
    popq %rbx
    popq %rdx
    popq %rcx
    popq %rax
    popq %rbp
    retq


.section .text
.globl sig_restore
sig_restore:
    addq $8, %rsp
    addq $8, %rsp
    popq %rdi
    
    popq %rax
    popq %r10
    popq %r11

    popq %rbx
    popq %r12
    popq %r15
    popfq
    retq
