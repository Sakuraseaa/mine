// CRT =  C RunTime library
// 用户进程的入口，进行初始化函数，为进程准备条件，传递参数等工作
.section .text
.globl _start
_start:
    call main

Proc_End:
    jmp Proc_End



// 用于信号处理函数执行完成后,恢复进程环境
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