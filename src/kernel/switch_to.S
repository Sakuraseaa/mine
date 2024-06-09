.section .text
.globl switch_to

thread = 0x28
rsp0 =	0x00
RIP	=	0x08
RSP	=	0x10
FS	=	0x18
GS	=	0x20
CR2	=	0x28
trap_nr = 0x30
error_code = 0x38

switch_to: 
    pushq   %rbp
    pushq   %rax
    pushq   %rcx
    pushq   %rdx

    movq    thread(%rdi), %rcx // prev
    movq    thread(%rsi), %rdx // next

    movq %rsp, RSP(%rcx)
    movq RSP(%rdx), %rsp
    
    leaq back(%rip), %rax
    movq %rax, RIP(%rcx)

    pushq  RIP(%rdx)
    jmp __switch_to
    
back:
    popq %rdx
    popq %rcx
    popq %rax
    popq %rbp
    ret
