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
    pushq   %rbx
    pushq   %rdx

    movq    thread(%rdi), %rbx // 0x28 = thread在PCB中的偏移
    movq    thread(%rsi), %rdx

    movq %rsp, RSP(%rbx)
    movq RSP(%rdx), %rsp
    
    leaq 1f(%rip), %rax
    movq %rax, RIP(%rbx)

    pushq  RIP(%rdx)
    jmp __switch_to
    
    1:
    popq %rdx
    popq %rbx
    popq %rax
    popq %rbp
