org 10000h
    jmp Label_Start
; intel 格式
%include "fat12.inc"

BaseOfKernelFile     equ 0x00
OffsetOfKernelFile   equ 0x400000   ; 1MB
StartOfKernelFile equ 0xffff800000100000
BaseTmpOfKernelAddr  equ 0x00
OffsetTmpKernelFile   equ 0x7E00     ;内核文件被加载地址
MemoryStructBufferAddr  equ 0x7e00


[SECTION gdt]
LABEL_GDT:		dd	0,0
;基址：0x0, 段界限：0xFFFFF, 代码段：A, DPL:00, S = 1(非系统段), P = 1(段有效),G=1(4kb), D/B= 1, L = 0, AVL = 0
LABEL_DESC_CODE32:	dd	0x0000FFFF,0x00CF9A00
;基址: 0x0, 段界限：0xFFFFF, 代码段：2, DPL:00, S = 1(非系统段), P = 1(段有效),G=1(4kb), D/B= 1, L = 0, AVL = 0
LABEL_DESC_DATA32:	dd	0x0000FFFF,0x00CF9200

GdtLen	equ	$ - LABEL_GDT
GdtPtr	dw	GdtLen - 1
	dd	LABEL_GDT

SelectorCode32	equ	LABEL_DESC_CODE32 - LABEL_GDT
SelectorData32	equ	LABEL_DESC_DATA32 - LABEL_GDT

[SECTION gdt64]

LABEL_GDT64:		dq	0x0000000000000000
LABEL_DESC_CODE64:	dq	0x0020980000000000
LABEL_DESC_DATA64:	dq	0x0000920000000000

GdtLen64	equ	$ - LABEL_GDT64
GdtPtr64	dw	GdtLen64 - 1
		dd	LABEL_GDT64

SelectorCode64	equ	LABEL_DESC_CODE64 - LABEL_GDT64
SelectorData64	equ	LABEL_DESC_DATA64 - LABEL_GDT64


[SECTION .s16]
[BITS 16]
Label_Start:
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ax, 0x00
    mov ss, ax
    mov sp, 0x7c00

;============ display on screen: start loader ....
    mov ax, 0x1301    
    mov cx, 12      ;字符串长度
    mov bx, 00fh    ; bh = 页码, bl = 属性
    mov dx, 0200h   ; dh = 行号, dl = 列号
    push ax
    mov  ax, ds
    mov  es, ax
    pop ax          ; es:bp ->要显示的字符串地址
    mov bp, StartLoaderMessage
    int 10h

;======== open address A20
; 下面这段代码打开了 保护16模式，又关闭了，给fs获得了访问超越1MB地址的能力
    push ax
    in al, 92h
    or al, 00000010b
    out 92h, al
    pop ax

    cli
;在实模式下使用lgdt(32位数据指令)，需要加前缀db0x66
    db 0x66
    lgdt [GdtPtr]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    mov ax, SelectorData32
    mov fs, ax
    mov eax, cr0
    
    ; 关闭地址线
    and al, 11111110b
    mov cr0, eax

    sti

;=======	reset floppy

	xor	ah,	ah
	xor	dl,	dl
	int	13h

;======= 下面的代码实现在软盘根目录下找到fat12, boot中的代码类似, 不做注释了
;======= 注意执行这段代码后， di执行的是内核代码在 / 目录中的目录项
;=======	search kernel.bin
	mov	word	[SectorNo],	SectorNumOfRootDirStart

Lable_Search_In_Root_Dir_Begin:

	cmp	word	[RootDirSizeForLoop],	0
	jz	Label_No_LoaderBin
	dec	word	[RootDirSizeForLoop]	
	mov	ax,	00h
	mov	es,	ax
	mov	bx,	8000h
	mov	ax,	[SectorNo]
	mov	cl,	1
	call	Func_ReadOneSector
	mov	si,	KernelFileName
	mov	di,	8000h
	cld
	mov	dx,	10h
	
Label_Search_For_LoaderBin:

	cmp	dx,	0
	jz	Label_Goto_Next_Sector_In_Root_Dir
	dec	dx
	mov	cx,	11

Label_Cmp_FileName:

	cmp	cx,	0
	jz	Label_FileName_Found
	dec	cx
	lodsb	
	cmp	al,	byte	[es:di]
	jz	Label_Go_On
	jmp	Label_Different

Label_Go_On:
	
	inc	di
	jmp	Label_Cmp_FileName

Label_Different:

	and	di,	0FFE0h
	add	di,	20h
	mov	si,	KernelFileName
	jmp	Label_Search_For_LoaderBin

Label_Goto_Next_Sector_In_Root_Dir:
	
	add	word	[SectorNo],	1
	jmp	Lable_Search_In_Root_Dir_Begin
	
;=======	display on screen : ERROR:No KERNEL Found

Label_No_LoaderBin:

	mov	ax,	1301h
	mov	bx,	008Ch
	mov	dx,	0300h		;row 3
	mov	cx,	21
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	mov	bp,	NoLoaderMessage
	int	10h
	jmp	$

;=======	found loader.bin name in root director struct
;====== 从硬盘加载内核镜像到 0x7c00, 再把内核移动到高地址4MB处
Label_FileName_Found:
	mov	ax,	RootDirSectors
	and	di,	0FFE0h
	add	di,	01Ah
	mov	cx,	word	[es:di]
	push	cx
	add	cx,	ax
	add	cx,	SectorBalance
	mov	eax,	BaseTmpOfKernelAddr	;BaseOfKernelFile
	mov	es,	eax
	mov	bx,	OffsetTmpKernelFile	;OffsetOfKernelFile
	mov	ax,	cx

Label_Go_On_Loading_File:
	push	ax
	push	bx
	mov	ah,	0Eh
	mov	al,	'.'
	mov	bl,	0Fh
	int	10h
	pop	bx
	pop	ax

	mov	cl,	1
	call	Func_ReadOneSector
	pop	ax      ; 加载内核文件的一个扇区进入了缓冲区

;;;;;;;;;;;;;;;;;;;;;;;	
	push	cx
	push	eax
	push	fs
	push	edi
	push	ds
	push	esi

	mov	cx,	200h                     ;循环次数
	
    mov	ax,	BaseOfKernelFile        ;目的地址
	mov	fs,	ax
	mov	edi, dword [OffsetOfKernelFileCount]

	mov	ax,	BaseTmpOfKernelAddr     ;源地址
	mov	ds,	ax
	mov	esi,OffsetTmpKernelFile

Label_Mov_Kernel:	;------------------
	
	mov	al,	byte	[ds:esi]
	mov	byte	[fs:edi],	al

	inc	esi
	inc	edi

	loop	Label_Mov_Kernel
	;=================== 下面这两句是必要的， 可我知道必要再哪里 ===============
	mov	eax, 0x1000
	mov	ds,	eax
	;======================================================================

	mov	dword	[OffsetOfKernelFileCount],	edi

	pop	esi
	pop	ds
	pop	edi
	pop	fs
	pop	eax
	pop	cx
;;;;;;;;;;;;;;;;;;;;;;;	

	call	Func_GetFATEntry
	cmp	ax,	0FFFh
	jz	Label_File_Loaded
	push	ax
	mov	dx,	RootDirSectors
	add	ax,	dx
	add	ax,	SectorBalance

	jmp	Label_Go_On_Loading_File

Label_File_Loaded:
    mov ax, 0B800h
    mov gs, ax
    mov ah, 0Fh                     ; 0000 黑底， 1111白字
    mov al, 'G'
    mov [gs:((80*0 + 39) * 2)], ax  ;屏幕第0行, 第39列

;====== 关闭软驱马达
KillMotor:
	push dx
	mov dx, 03F2h
	mov al, 0
	out dx, al
	pop dx

;=======	get memory address size type
	mov	ax,	1301h		; 01设置光标跟随
	mov	bx,	000Fh		; 黑白高亮
	mov	dx,	0400h		;row 4, 第4行
	mov	cx,	24			;cx = 字符串长度
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax				; es:bp ->指向字符串
	mov	bp,	StartGetMemStructMessage
	int	10h				; 打印字符串

	mov	ebx,	0; ebx = 后续值, 开始时需为 0
	mov	ax,	0x00
	mov	es,	ax
	mov	di,	MemoryStructBufferAddr	
; 通过15号中断,得到多个ARDS(Address Range Descriptor Structure 地址范围描述符)
; es:di = 0x7c00是ARDS存储缓冲区
Label_Get_Mem_Struct:

	mov	eax,	0x0E820		; eax = 0000E820h
	mov	ecx,	20			; ecx = 地址范围描述符结构的大小
	mov	edx,	0x534D4150	; edx = 'SMAP'
	int	15h
	jc	Label_Get_Mem_Fail
	add	di,	20

	cmp	ebx,	0		;是否已经读完了ARDS ?
	jne	Label_Get_Mem_Struct
	jmp	Label_Get_Mem_OK

Label_Get_Mem_Fail:
;获取内存地址失败, ->在第五行打印失败信息
	mov	ax,	1301h
	mov	bx,	008Ch
	mov	dx,	0500h		;row 5
	mov	cx,	23
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	mov	bp,	GetMemStructErrMessage
	int	10h
	jmp	$

Label_Get_Mem_OK:
;获取内存地址成功, ->在第六行打印成功信息
	mov	ax,	1301h
	mov	bx,	000Fh
	mov	dx,	0600h		;row 6
	mov	cx,	29
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	mov	bp,	GetMemStructOKMessage
	int	10h	

; 不懂 321 ~ 467行代码的来龙去脉，不知道svga是什么
; 下面代码的主要目的是设置合理vbe号, 使得图像能够显示
;=======	get SVGA information

	mov	ax,	1301h
	mov	bx,	000Fh
	mov	dx,	0800h		;row 8
	mov	cx,	23
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	mov	bp,	StartGetSVGAVBEInfoMessage
	int	10h

	mov	ax,	0x00
	mov	es,	ax
	mov	di,	0x8000
	
; 根据4f00的得到vbe控制信息
	mov	ax,	4F00h
	int	10h
	cmp	ax,	004Fh ; 判定操作是否成功
	; 得到vbeInfor信息结构块，512字节
	jz	.KO
	
;=======	get SVGA information Fail=======

	mov	ax,	1301h
	mov	bx,	008Ch
	mov	dx,	0900h		;row 9
	mov	cx,	23
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	mov	bp,	GetSVGAVBEInfoErrMessage
	int	10h
	jmp	$

.KO:

	mov	ax,	1301h
	mov	bx,	000Fh
	mov	dx,	0A00h		;row 10
	mov	cx,	29
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	mov	bp,	GetSVGAVBEInfoOKMessage
	int	10h

	;打印vbeInforBlock信息块前22个字节
	; mov ax, 0x00
	; mov es, ax
	; mov si, 0x8000
	; mov cx, 22
;LOOP_Disp_VBE_INFO:
;	mov ax, 00h
;	mov al, byte[es:si]
;	call Label_DispAL
;	add si, 1
;	loop LOOP_Disp_VBE_INFO
;	jmp $

;=======	Get SVGA Mode Info

	mov	ax,	1301h
	mov	bx,	000Fh
	mov	dx,	0C00h		;row 12
	mov	cx,	24
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	mov	bp,	StartGetSVGAModeInfoMessage
	int	10h


	mov	ax,	0x00
	mov	es,	ax
	mov	si,	0x800e

	mov	esi,	dword	[es:si] ;videoModeList指针, 模式号列表的原指针, VBE芯片能够支持的模式号
	mov	edi,	0x8200

Label_SVGA_Mode_Info_Get:

	mov	cx,	word	[es:esi]	
; 打印vbe芯片能打印的模式号, 获得每个模式的信息
;=======	display SVGA mode information
	push	ax
	
	mov	ax,	00h
	mov	al,	ch
	call	Label_DispAL

	mov	ax,	00h
	mov	al,	cl	
	call	Label_DispAL
	
	pop	ax

;=======
	
	cmp	cx,	0FFFFh
	jz	Label_SVGA_Mode_Info_Finish
	cmp cx, 0180h
	jnz Sk
	
	mov	ax,	4F01h
	int	10h

	cmp	ax,	004Fh

	jnz	Label_SVGA_Mode_Info_FAIL	
; 如果是合理的VBE结构, 那么就输出55aa /地址标志
	xor eax, eax
	mov al, byte [es:edi + 25]
	cmp al, 0x20
	jne Sk
	mov al, byte [es:edi + 27]
	cmp al, 6h
	jne Sk
	; mov ax, word [es:edi + 42] 写的地址
	;mov ax, word [es:edi + 18] ; 水平分辨率
	;mov ax, word [es:edi + 20]	;垂直分辨率
; 输出55aa标志=== 
	mov cx, ax
	push ax
	mov ax, 00h
	mov al, ch
	call Label_DispAL
	mov ax, 00h
	mov al, cl
	call Label_DispAL
	pop ax
; ===
	add edi, 0x100
Sk:
	add	esi,	2

	jmp	Label_SVGA_Mode_Info_Get
		
Label_SVGA_Mode_Info_FAIL:

	mov	ax,	1301h
	mov	bx,	008Ch
	mov	dx,	0D00h		;row 13
	mov	cx,	24
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	mov	bp,	GetSVGAModeInfoErrMessage
	int	10h

Label_SET_SVGA_Mode_VESA_VBE_FAIL:

	jmp	$

Label_SVGA_Mode_Info_Finish:

	mov	ax,	1301h
	mov	bx,	000Fh
	mov	dx,	0E00h		;row 14
	mov	cx,	30
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	mov	bp,	GetSVGAModeInfoOKMessage
	int	10h

	;jmp $
; vesa bios extension 设置vbe号
; ============= set the SVGA mode(VESA VBE) ====
	mov ax, 4F02h
	mov bx, 4180h		;mode: 0x180

	int 10h

	cmp ax, 004Fh
	jnz Label_SET_SVGA_Mode_VESA_VBE_FAIL
;========= init IDT GDT goto protect mode ==========
	
	cli 			;===== close interrupt
	
	db 0x66
	lgdt [GdtPtr]

	db 0x66
	lidt [IDT_POINTER]

	mov eax, cr0
	or eax, 1
	mov cr0, eax

	jmp dword SelectorCode32:GO_TO_TMP_Proctect

; IA-32保护模式
[SECTION .32]
[BITS 32]
GO_TO_TMP_Proctect:
	
	mov ax, 0x10	;初始化段寄存器
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov ss, ax
	mov esp, 7E00h	;栈顶指向0x7e00

	call support_long_mode
	test eax, eax

	jz no_support	;不支持64位模式则跳转

;======= init temporary page table 0x90000 ===
	
	mov dword [0x90000], 0x91007
	mov dword [0x90800], 0x91007

	mov dword [0x91000], 0x92007

	mov	dword	[0x92000],	0x000087
	mov	dword	[0x92008],	0x200087	;8-2MB的物理页，3=有效的，可读写的，系统的
	mov	dword	[0x92010],	0x400087
	mov	dword	[0x92018],	0x600087
	mov	dword	[0x92020],	0x800087
	mov	dword	[0x92028],	0xa00087

; ======= load GDTR
	db 0x66
	lgdt [GdtPtr64]
	
	mov ax, 0x10
	mov	ds,	ax
	mov	es,	ax
	mov	fs,	ax
	mov	gs,	ax
	mov	ss,	ax

	mov	esp,	7E00h

; ======= open PAE eflags
	
	mov eax, cr4
	bts eax, 5
	mov cr4, eax

; ======= open load cr3, 加载页表目录基址基址寄存器
	mov eax, 0x90000
	mov cr3, eax

; ======= enable  long-mode  开启MSR寄存器中的LME标志位====
	mov ecx, 0C0000080h		;IS32_EFER
	rdmsr					; 读64位的msr寄存器, edx:eax

	bts eax, 8				; 启动LME标志
	wrmsr					; 写msr寄存器

; ======== open PE and paging
	mov eax, cr0
	bts eax, 0
	bts eax, 31
	mov cr0, eax

	jmp SelectorCode64:InitKernel

	
; ======== test support long mode or not =====
support_long_mode:
	mov eax, 0x80000000
	cpuid
	cmp eax, 0x80000001		;查出最大主功能号
	setnb al				;如果eax >= 0x80000001, 则设置al = 1
	jb support_long_mode_done	;低于则跳转
	mov  eax, 0x80000001	;查处最大扩展功能号
	cpuid
	bt edx, 29				 ;这里不太懂
	setc al
support_long_mode_done:
	movzx eax, al
	ret
no_support:
	jmp $

; 64保护模式
[SECTION .64]
[BITS 64]
InitKernel:
	
	xor rsi, rsi
	mov cx, word [OffsetOfKernelFile + 38h]
	movzx rcx, cx

	mov rsi, [OffsetOfKernelFile + 20h]
	add rsi, OffsetOfKernelFile
.Begin:
	mov eax, [rsi + 0]
	cmp eax, 0
	je .NoAction
;======================================
	push rsi
	push rcx

	mov rcx, [rsi + 20h]	; count
	mov rdi, [rsi + 10h]	; destination
	mov rsi, [rsi + 8h]		; source
	add rsi, OffsetOfKernelFile

	cmp rcx, 0
	jz .InitKernel_end
.mov_Kernel:
	
	mov al, [rsi]
	mov byte [rdi], al
	inc rdi
	inc rsi
	loop .mov_Kernel
	
	pop rcx
	pop rsi
;======================================
.NoAction
	add rsi, 038h
	dec rcx
	jnz .Begin
.InitKernel_end:

	jmp StartOfKernelFile
	

;=======	read one sector from floppy, boot中实现过
[SECTION .s16lib]
[BITS 16]
Func_ReadOneSector:
	push	bp
	mov	bp,	sp
	sub	esp,	2
	mov	byte	[bp - 2],	cl
	push	bx
	mov	bl,	[BPB_SecPerTrk]
	div	bl
	inc	ah
	mov	cl,	ah
	mov	dh,	al
	shr	al,	1
	mov	ch,	al
	and	dh,	1
	pop	bx
	mov	dl,	[BS_DrvNum]
Label_Go_On_Reading:
	mov	ah,	2
	mov	al,	byte	[bp - 2]
	int	13h
	jc	Label_Go_On_Reading
	add	esp,	2
	pop	bp
	ret
;=======	get FAT Entry, boot中实现过
Func_GetFATEntry:    
	push	es
	push	bx
	push	ax
	mov	ax,	00
	mov	es,	ax
	pop	ax
	mov	byte	[Odd],	0
	mov	bx,	3
	mul	bx
	mov	bx,	2
	div	bx
	cmp	dx,	0
	jz	Label_Even
	mov	byte	[Odd],	1

Label_Even:

	xor	dx,	dx
	mov	bx,	[BPB_BytesPerSec]
	div	bx
	push	dx
	mov	bx,	8000h
	add	ax,	SectorNumOfFAT1Start
	mov	cl,	2
	call	Func_ReadOneSector
	
	pop	dx
	add	bx,	dx
	mov	ax,	[es:bx]
	cmp	byte	[Odd],	1
	jnz	Label_Even_2
	shr	ax,	4

Label_Even_2:
	and	ax,	0FFFh
	pop	bx
	pop	es
	ret

;=======	display num in al
;AL = 要显示的16进制数
Label_DispAL:

	push	ecx
	push	edx
	push	edi
	
	mov	edi, [DisplayPosition]
	mov	ah,	0Fh	;给ah存入字体颜色属性值
	mov	dl,	al
	shr	al,	4
	mov	ecx,	2
.begin:

	and	al,	0Fh
	cmp	al,	9
	ja	.1			; al > 9, 则跳转
	add	al,	'0'
	jmp	.2
.1:

	sub	al,	0Ah
	add	al,	'A'
.2:

	mov	[gs:edi],	ax	;gs:edi指向显存
	add	edi,	2
	
	mov	al,	dl
	loop	.begin

	mov	[DisplayPosition],	edi

	pop	edi
	pop	edx
	pop	ecx
	
	ret
;=======	tmp IDT 定义了空的中断描述符表
IDT:
	times	0x50	dq	0	; 0x50 = 80
IDT_END:

IDT_POINTER:
		dw	IDT_END - IDT - 1
		dd	IDT


;=======	tmp variable

RootDirSizeForLoop	dw	RootDirSectors
SectorNo		dw	0
Odd			db	0
OffsetOfKernelFileCount	dd	OffsetOfKernelFile

DisplayPosition		dd	0

;=======	display messages

StartLoaderMessage:	db	"Start Loader"
NoLoaderMessage:	db	"ERROR:No KERNEL Found"
KernelFileName:		db	"KERNEL  BIN",0
StartGetMemStructMessage:	db	"Start Get Memory Struct."
GetMemStructErrMessage:	db	"Get Memory Struct ERROR"
GetMemStructOKMessage:	db	"Get Memory Struct SUCCESSFUL!"

StartGetSVGAVBEInfoMessage:	db	"Start Get SVGA VBE Info"
GetSVGAVBEInfoErrMessage:	db	"Get SVGA VBE Info ERROR"
GetSVGAVBEInfoOKMessage:	db	"Get SVGA VBE Info SUCCESSFUL!"

StartGetSVGAModeInfoMessage:	db	"Start Get SVGA Mode Info"
GetSVGAModeInfoErrMessage:	db	"Get SVGA Mode Info ERROR"
GetSVGAModeInfoOKMessage:	db	"Get SVGA Mode Info SUCCESSFUL!"