#include "linkage.h"

.section .text
.globl _start
_start:

	mov	$0x10,	%ax
	mov	%ax,	%ds
	mov	%ax,	%es
	mov	%ax,	%fs
	mov	%ax,	%ss
	mov	$0x7E00,	%esp

//=======	load GDTR

	lgdt	GDT_POINTER(%rip)

//=======	load	IDTR

	lidt	IDT_POINTER(%rip)

	mov	$0x10,	%ax
	mov	%ax,	%ds
	mov	%ax,	%es
	mov	%ax,	%fs
	mov	%ax,	%gs
	mov	%ax,	%ss

	movq	$0x7E00,	%rsp

//=======	load	cr3

	movq	$0x101000,	%rax
	movq	%rax,		%cr3
	movq	switch_seg(%rip),	%rax
	pushq	$0x08
	pushq	%rax
	lretq

//=======	64-bit mode code

switch_seg:
	.quad	entry64

entry64:
	movq	$0x10,	%rax
	movq	%rax,	%ds
	movq	%rax,	%es
	movq	%rax,	%gs
	movq	%rax,	%ss
	movq	_stack_start(%rip),	%rsp		/* rsp address */

setup_IDT:							
	leaq	ignore_int(%rip),	%rdx
	movq	$(0x08 << 16),	%rax
	movw	%dx,	%ax
	movq	$(0x8E00 << 32),	%rcx		
	addq	%rcx,	%rax
	movl	%edx,	%ecx
	shrl	$16,	%ecx
	shlq	$48,	%rcx
	addq	%rcx,	%rax
	shrq	$32,	%rdx
	leaq	IDT_Table(%rip),	%rdi
	mov	$256,	%rcx
rp_sidt:
	movq	%rax,	(%rdi)
	movq	%rdx,	8(%rdi)
	addq	$0x10,	%rdi
	dec	%rcx
	jne	rp_sidt

// 设置TSS描述符
setup_TSS64:
	leaq	TSS64_Table(%rip),	%rdx
	xorq	%rax,	%rax
	xorq	%rcx,	%rcx
	movq	$0x89,	%rax
	shlq	$40,	%rax
	movl	%edx,	%ecx
	shrl	$24,	%ecx
	shlq	$56,	%rcx
	addq	%rcx,	%rax
	xorq	%rcx,	%rcx
	movl	%edx,	%ecx
	andl	$0xffffff,	%ecx
	shlq	$16,	%rcx
	addq	%rcx,	%rax
	addq	$103,	%rax
	leaq	GDT_Table(%rip),	%rdi
	movq	%rax,	80(%rdi)	// tss segment offset
	shrq	$32,	%rdx
	movq	%rdx,	88(%rdi)	// tss + 1 segment offset

//	mov	$0x50,	%ax
//	ltr	%ax

	movq	go_to_kernel(%rip),	%rax		/* movq address */
	pushq	$0x08
	pushq	%rax
	lretq

go_to_kernel:
	.quad	Start_Kernel


//=======	ignore_int

ignore_int:
	cld
	pushq	%rax
	pushq	%rbx
	pushq	%rcx
	pushq	%rdx
	pushq	%rbp
	pushq	%rdi
	pushq	%rsi

	pushq	%r8
	pushq	%r9
	pushq	%r10
	pushq	%r11
	pushq	%r12
	pushq	%r13
	pushq	%r14
	pushq	%r15

	movq	%es,	%rax
	pushq	%rax
	movq	%ds,	%rax
	pushq	%rax

	movq	$0x10,	%rax
	movq	%rax,	%ds
	movq	%rax,	%es

	leaq	int_msg(%rip),	%rax			/* leaq get address */
	pushq	%rax
	movq	%rax,	%rdx
	movq	$0x00000000,	%rsi
	movq	$0x00ff0000,	%rdi
	movq	$0,	%rax
	callq	color_printk
	addq	$0x8,	%rsp

Loop:
	jmp	Loop	

	popq	%rax
	movq	%rax,	%ds
	popq	%rax
	movq	%rax,	%es

	popq	%r15
	popq	%r14
	popq	%r13
	popq	%r12
	popq	%r11
	popq	%r10
	popq	%r9
	popq	%r8

	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%rdx
	popq	%rcx
	popq	%rbx
	popq	%rax
	iretq

int_msg:
	.asciz "Unknown interrupt or fault at RIP\n"

// 这里定义的是一个地址，内存地址
ENTRY(_stack_start)
	.quad	init_task_union + 32768

//=======	init page
/*.align 8
.org	0x1000
__PML4E:
	.quad	0x102003
	.fill	255,8,0
	.quad	0x102003
	.fill	255,8,0

.org	0x2000
__PDPTE:
	.quad	0x103003
	.fill	511,8,0

.org	0x3000
__PDE:
	.quad	0x000083	
	.quad	0x200083
	.quad	0x400083
	.quad	0x600083
	.quad	0x800083		// 0X800083 
	.quad   0xa00083
	.quad   0xc00083
	.quad   0xe00083
	.quad	0x1000083
	.quad	0x1200083
	.quad	0x1400083
	.quad	0x1600083
	.quad	0x1800083
	.quad	0x1a00083
	.quad	0x1c00083
	.quad	0x1e00083
	.quad	0x2000083
	.quad	0x2200083
	.quad	0x2400083
	.quad	0x2600083
	.quad	0x2800083
	.quad	0x2a00083
	.quad	0x2c00083
	.quad	0x2e00083		// 0 ~ 48MB 都被内核映射到了
	
	.quad	0xfd000083		// 0x300_0000
	.quad	0xfd200083
	.quad	0xfd400083
	.quad	0xfd600083		
	.quad	0xfd800083
	.quad	0xfda00083
	.quad	0xfdc00083
	.quad	0xfde00083
	.fill	499,8,0
*/
//=======	init page

.align 8

.org	0x1000

__PML4E:

	.quad	0x102007
	.fill	255,8,0
	.quad	0x102007
	.fill	255,8,0

.org	0x2000

__PDPTE:
	
	.quad	0x103007
	.fill	511,8,0
/*
.org	0x3000

__PDE:

	.quad	0x000087	
	.quad	0x200087
	.quad	0x400087
	.quad	0x600087
	.quad	0x800087		// 0X800083 
	.quad   0xa00087
	.quad   0xc00087
	.quad   0xe00087
	.quad	0x1000087
	.quad	0x1200087
	.quad	0x1400087
	.quad	0x1600087
	.quad	0x1800087
	.quad	0x1a00087
	.quad	0x1c00087
	.quad	0x1e00087
	.quad	0x2000087
	.quad	0x2200087
	.quad	0x2400087
	.quad	0x2600087
	.quad	0x2800087
	.quad	0x2a00087
	.quad	0x2c00087
	.quad	0x2e00087		// 0 ~ 48MB 都被内核映射到了
	

	.quad	0xfd000087		// 0x300_0000
	.quad	0xfd200087
	.quad	0xfd400087
	.quad	0xfd600087		
	.quad	0xfd800087
	.quad	0xfda00087
	.quad	0xfdc00087
	.quad	0xfde00087
	.fill	480,8,0
*/

.org 0x3000

__PDE:
    .quad 0x104007
    .quad 0x105007
    .quad 0x106007
    .quad 0x107007
    .quad 0x108007
    .quad 0x109007
    .quad 0x10A007
    .quad 0x10B007
    .quad 0x10C007
    .quad 0x10D007
    .quad 0x10E007
    .quad 0x10F007
    .quad 0x110007
    .quad 0x111007
    .quad 0x112007
    .quad 0x113007
    .quad 0x114007
    .quad 0x115007
    .quad 0x116007
    .quad 0x117007
    .quad 0x118007
    .quad 0x119007
    .quad 0x11A007
    .quad 0x11B007
	
	.quad	0x11C007		// 0x300_0000
	.quad	0x11D007
	.quad	0x11E007
	.quad	0x11F007		
	.quad	0x120007
	.quad	0x121007
	.quad	0x122007
	.quad	0x123007

	.fill	480,8,0


// 页表0
.org 0x4000
page_table_0:
    .set base, 0x00000000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表1
.org 0x5000
page_table_1:
    .set base, 0x00200000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表2
.org 0x6000
page_table_2:
    .set base, 0x00400000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表3
.org 0x7000
page_table_3:
    .set base, 0x00600000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表4
.org 0x8000
page_table_4:
    .set base, 0x00800000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表5
.org 0x9000
page_table_5:
    .set base, 0x00a00000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表6
.org 0xA000
page_table_6:
    .set base, 0x00c00000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表7
.org 0xB000
page_table_7:
    .set base, 0x00e00000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表8
.org 0xC000
page_table_8:
    .set base, 0x01000000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表9
.org 0xD000
page_table_9:
    .set base, 0x01200000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表10
.org 0xE000
page_table_10:
    .set base, 0x01400000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表11
.org 0xF000
page_table_11:
    .set base, 0x01600000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表12
.org 0x10000
page_table_12:
    .set base, 0x01800000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表13
.org 0x11000
page_table_13:
    .set base, 0x01a00000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表14
.org 0x12000
page_table_14:
    .set base, 0x01c00000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表15
.org 0x13000
page_table_15:
    .set base, 0x01e00000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表16
.org 0x14000
page_table_16:
    .set base, 0x02000000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表17
.org 0x15000
page_table_17:
    .set base, 0x02200000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表18
.org 0x16000
page_table_18:
    .set base, 0x02400000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表19
.org 0x17000
page_table_19:
    .set base, 0x02600000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表20
.org 0x18000
page_table_20:
    .set base, 0x02800000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表21
.org 0x19000
page_table_21:
    .set base, 0x02a00000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表22
.org 0x1A000
page_table_22:
    .set base, 0x02c00000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

// 页表23
.org 0x1B000
page_table_23:
    .set base, 0x02e00000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

.org 0x1C000
page_table_2g:
    .set base, 0xfd000000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr

.org 0x1D000
page_table_2t:
    .set base, 0xfd200000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr
.org 0x1E000
page_table_29:
    .set base, 0xfd400000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr
.org 0x1F000
page_table_2r:
    .set base, 0xfd600000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr
.org 0x20000
page_table_28:
    .set base, 0xfd800000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr
.org 0x21000
page_table_25:
    .set base, 0xfda00000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr
.org 0x22000
page_table_26:
    .set base, 0xfdc00000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr
.org 0x23000
page_table_27:
    .set base, 0xfde00000
    .set index, base + 0x7
    .rept 512
        .quad index
        .set index, index + 0x1000
    .endr
//=======	init page
/*
.align 8

.org	0x1000

__PML4E:

	.quad	0x102007
	.fill	255,8,0
	.quad	0x102007
	.fill	255,8,0

.org	0x2000

__PDPTE:
	
	.quad	0x103007
	.fill	511,8,0

.org	0x3000

__PDE:

	.quad	0x104007	
	.quad	0x200086
	.quad	0x400086
	.quad	0x600086
	.quad	0x800086		//  0X800083 
	.quad   0xa00086
	.quad   0xc00086
	.quad   0xe00086
	.quad	0x1000086
	.quad	0x1200086
	.quad	0x1400086
	.quad	0x1600086
	.quad	0x1800086
	.quad	0x1a00086
	.quad	0x1c00086
	.quad	0x1e00086
	.quad	0x2000086
	.quad	0x2200086
	.quad	0x2400086
	.quad	0x2600086
	.quad	0x2800086
	.quad	0x2a00086
	.quad	0x2c00086
	.quad	0x2e00086		// 0 ~ 48MB 都被内核映射到了
	

	.quad	0xfd000087		// 0x300_0000
	.quad	0xfd200087
	.quad	0xfd400087
	.quad	0xfd600087		
	.quad	0xfd800087
	.quad	0xfda00087
	.quad	0xfdc00087
	.quad	0xfde00087
	.fill	480,8,0
.org 0x4000
_PE:
    .quad   0x0007    // 第一个表项
    // 填充剩下的508个表项
    .set index, 0x1007
    .rept 511
        .quad index
        .set index, index + 0x1000
    .endr */
//=======	GDT_Table

.section .data

.globl GDT_Table

GDT_Table:
	.quad	0x0000000000000000			/*0	NULL descriptor		       	00*/
	.quad	0x0020980000000000			/*1	KERNEL	Code	64-bit	Segment	08*/
	.quad	0x0000920000000000			/*2	KERNEL	Data	64-bit	Segment	10*/
	.quad	0x0000000000000000			/*3	USER	Code	32-bit	Segment 18*/
	.quad	0x0000000000000000			/*4	USER	Data	32-bit	Segment 20*/
	.quad	0x0020f80000000000			/*5	USER	Code	64-bit	Segment	28*/
	.quad	0x0000f20000000000			/*6	USER	Data	64-bit	Segment	30*/
	.quad	0x00cf9a000000ffff			/*7	KERNEL	Code	32-bit	Segment	38*/
	.quad	0x00cf92000000ffff			/*8	KERNEL	Data	32-bit	Segment	40*/
	.fill	100,8,0					/*10 ~ 11 TSS (jmp one segment <9>) in long-mode 128-bit 50*/
GDT_END:
GDT_POINTER:
GDT_LIMIT:	.word	GDT_END - GDT_Table - 1
GDT_BASE:	.quad	GDT_Table

//=======	IDT_Table

.globl IDT_Table

IDT_Table:
	.fill  512,8,0
IDT_END:

IDT_POINTER:
IDT_LIMIT:	.word	IDT_END - IDT_Table - 1
IDT_BASE:	.quad	IDT_Table

//=======	TSS64_Table

.globl	TSS64_Table

TSS64_Table:
	.fill  13,8,0
TSS64_END:


TSS64_POINTER:
TSS64_LIMIT:	.word	TSS64_END - TSS64_Table - 1
TSS64_BASE:	.quad	TSS64_Table

