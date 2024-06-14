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