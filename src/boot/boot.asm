org 0x7c00
; intel 格式
BaseOfStack equ 0x7c00
BaseOfLoader equ 0x1000
OffsetOfLoader equ 0x00
RootDirSectors equ 14           ; /目录 占用的扇区数
SectorNumOfRootDirStart equ 19  ; / 目录的起始扇区数
SectorNumOfFAT1Start equ 1      ; fat1表起始扇区号
SectorBalance equ 17    ; SectorBalance = BPB_RsvdSecCnt + (BPB_NumFATs * FATSz) - 2，-2是因为簇号是从2开始计数的，
					; 但前两个簇号没有对应的扇区
					; 文件的开始Sector号 = DirEntry中的开始Sector号 + 根目录占用Sector数目 + DeltaSectorNo

; 这里定义的是fat12系统的元数据， fat = File Allocation Table = 文件分配表
    jmp short Label_Start       ; 跳转指令, 3字节
    nop 

    BS_OEMName db 'MINEboot'    ; 记录制造商的名字
    BPB_BytesPerSec dw 512      ; 每扇区字节数

    ; 每簇扇区数, 簇(Cluster),簇是Fat类文件系统的最小数据存储单位
    BPB_SecPerClus db 1        
    
    BPB_RsvdSecCnt dw 1         ; 保留扇区数, 引导扇区在保留扇区中
    BPB_NumFATs    db 2         ; FAT表的份数, FAT1 和 FAT2表是一样的
    BPB_RootEntCnt dw 224       ; 根目录可容纳的目录项数
    BPB_TotSec16   dw 2880      ; 总扇区数
    BPB_Media      db 0xf0      ; 介质描述符, 描述存储介质类型
    BPF_FATSz16    dw 9         ; 每FAT扇区数
    BPB_SecPerTrk  dw 18        ; 每磁道扇区数
    BPB_NumHeads   dw 2         ; 磁头数
    BPB_hiddSec    dd 0         ; 隐藏扇区数
    BPB_TotSec32   dd 0         ; 如果BPB_TotSec16值为0，则由该值记录扇区数
    BS_DrvNum      db 0         ; int 13h的驱动器号
    BS_Reserved1   db 0         ; 未使用
    BS_BootSig     db 29h       ; 扩展引导标记
    BS_VolID       dd 0         ; 卷序列号
    BS_VolLab      db 'boot loader' ;卷标
    BS_FileSysType db 'FAT12'   ;文件系统类型

Label_Start: 
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, BaseOfStack

;============ clear screen  
    mov ax, 0600h   ; ah 功能号=06h, al上卷行数，为0代表全部
    mov bx, 0700h   ; bh 上卷行的属性
    mov cx, 0       ;(0, 0)左上角位置
    mov dx, 0184fh  ;右下角(80, 25),80个字符25行, 下标从0开始，0x18=24, 0x4f=79
    int 10h

;========= set focus
    mov ax, 0200h
    mov dx, 000fh
    mov bx, 0000h
    int 10h

;========= display on screen: Start Booting...
    mov ax, 1301h
    mov bx, 000fh   ;黑白, 字体高两
    mov cx, 0ah
    mov dx, 0h
    mov bp, StartBootMessage
    int 010h

;========= reset floppy
    xor ah, ah
    xor dl, dl
    int 13h

;===================== 下面在 A 盘的根目录寻找 LOADER.BIN
	mov	word [SectorNo], SectorNumOfRootDirStart
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
	cmp	word [RootDirSizeForLoop], 0	; ┓
	jz	LABEL_NO_LOADEBIN		; ┣ 判断根目录区是不是已经读完
	dec	word [RootDirSizeForLoop]	; ┛ 如果读完表示没有找到 LOADER.BIN
	mov	ax, 00h
	mov	es, ax			; es <- BaseOfLoader es:bx = 缓冲区地址
	mov	bx, 8000h	    ; bx <- OffsetOfLoader	于是, es:bx = BaseOfLoader:OffsetOfLoader
	mov	ax, [SectorNo]	; ax <- Root Directory 中的某 Sector 号, 要读取的扇区地址, LBA号
	mov	cl, 1           ; 要读取的扇区数
	call	Func_ReadOneSector

	mov	si, LoaderFileName	; ds:si -> "LOADER  BIN"
	mov	di, 8000h	; es:di -> BaseOfLoader:0100 = BaseOfLoader*10h+100
	cld             ; 清DF标志
	mov	dx, 10h     ; 每个扇区容纳的目录项的数量 512 / 32 = 16
LABEL_SEARCH_FOR_LOADERBIN:
	cmp	dx, 0										; ┓循环次数控制,
	jz	LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR	; ┣如果已经读完了一个 Sector,
	dec	dx											; ┛就跳到下一个 Sector
	mov	cx, 11      ; 目录项文件名的长度
LABEL_CMP_FILENAME:
	cmp	cx, 0
	jz	LABEL_FILENAME_FOUND	; 如果比较了 11 个字符都相等, 表示找到
	dec	cx
	lodsb				; ds:si -> al
	cmp	al, byte [es:di]
	jz	LABEL_GO_ON
	jmp	LABEL_DIFFERENT		; 只要发现不一样的字符就表明本 DirectoryEntry 不是
; 我们要找的 LOADER.BIN
LABEL_GO_ON:
	inc	di
	jmp	LABEL_CMP_FILENAME	;	继续循环

LABEL_DIFFERENT:
	and	di, 0FFE0h						; di &= E0 为了让它指向本条目开头
	add	di, 20h							; di += 20h  下一个目录条目
	mov	si, LoaderFileName				;
	jmp	LABEL_SEARCH_FOR_LOADERBIN;    ┛

LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
	add	word [SectorNo], 1
	jmp	LABEL_SEARCH_IN_ROOT_DIR_BEGIN

;=============== 没有找到loader.bin 输出错误信息
LABEL_NO_LOADEBIN:
    mov ax, 1301h
    mov bx, 008ch
    mov dx, 0100h
    mov cx, 21
    push ax
    mov ax, ds
    mov es, ax
    pop ax
    mov bp, NoLoaderMessage
    int 10h

LABEL_FILENAME_FOUND:			; 找到 LOADER.BIN 后便来到这里继续
	mov	ax, RootDirSectors
	and	di, 0FFE0h		; di -> 当前条目的开始
	add	di, 01Ah		; di -> 首 Sector,FAT表中的索引号
	mov	cx, word [es:di]
	push	cx			; 保存此 Sector (文件) 在 FAT 中的序号
	add	cx, ax
	add	cx, SectorBalance	; cx = 根目录起始地址 + 根目录占用扇区数 + (fat[2] - 2)
	;-2是因为簇1簇0没有对应的扇区

    mov	ax, BaseOfLoader
	mov	es, ax			; es <- BaseOfLoader
	mov	bx, OffsetOfLoader	; bx <- OffsetOfLoader
	mov	ax, cx			; ax <- Sector 号

LABEL_GOON_LOADING_FILE:
	push	ax			; `.
	push	bx			;  |
	mov	ah, 0Eh			;  | 每读一个扇区就在 "Booting  " 后面
	mov	al, '.'			;  | 打一个点, 形成这样的效果:
	mov	bl, 0Fh			;  | Booting ......
	int	10h				;  |
	pop	bx				;  |
	pop	ax				; /

	mov	cl, 1			; 读出loader.bin中的一共扇区
	call	Func_ReadOneSector
	pop	ax				; 取出此 Sector 在 FAT 中的序号
	call	Func_GetFATEntry
	cmp	ax, 0FFFh
	jz	LABEL_FILE_LOADED
	push	ax			; 保存 Sector 在 FAT 中的序号
	mov	dx, RootDirSectors
	add	ax, dx
	add	ax, SectorBalance		; ax = 下次要读的扇区号 lba
	add	bx, [BPB_BytesPerSec] 	;bx += 512
	jmp	LABEL_GOON_LOADING_FILE


LABEL_FILE_LOADED:		; ============                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          进入loader ==========
    jmp BaseOfLoader:OffsetOfLoader


;========= read one sector form floppy =====
;======= Func_ReadOneSector =======
; ax = 带读取的磁盘起始扇区号(LBA logical block Address),
; cl = 读入的扇区数量, es:bx => 目标缓冲区起始地址
Func_ReadOneSector:
	; -----------------------------------------------------------------------
	; 怎样由扇区号求扇区在磁盘中的位置 (扇区号 -> 柱面号, 起始扇区, 磁头号)
	; -----------------------------------------------------------------------
	; 设扇区号为 x
	;                          ┌ 柱面号 = y >> 1
	;       x           ┌ 商 y ┤
	; -------------- => ┤      └ 磁头号 = y & 1
	;  每磁道扇区数      │
	;                   └ 余 z => 起始扇区号 = z + 1
	push	bp
	mov	bp, sp
	sub	esp, 2			; 辟出两个字节的堆栈区域保存要读的扇区数: byte [bp-2]

	mov	byte [bp-2], cl
	push	bx			; 保存 bx
	mov	bl, [BPB_SecPerTrk]	; bl: 除数
	div	bl			    ; y 在 al 中, z 在 ah 中, y = 磁道号, z = 起始扇区号
	inc	ah			    ; z ++
	mov	cl, ah			; cl <- 起始扇区号
	mov	dh, al			; dh <- y
	shr	al, 1			; y >> 1 (其实是 y/BPB_NumHeads, 这里BPB_NumHeads=2)
	mov	ch, al			; ch <- 柱面号
	and	dh, 1			; dh & 1 = 磁头号
	pop	bx			    ; 恢复 bx
	; 至此, "柱面号, 起始扇区, 磁头号" 全部得到 ^^^^^^^^^^^^^^^^^^^^^^^^
	mov	dl, [BS_DrvNum]		; 驱动器号 (0 表示 A 盘)
.GoOnReading:
	mov	ah, 2			; 读
	mov	al, byte [bp-2]		; 读 al 个扇区
	int	13h
	jc	.GoOnReading		; 如果读取错误 CF 会被置为 1, 这时就不停地读, 直到正确为止

	add	esp, 2
	pop	bp

	ret

;----------------------------------------------------------------------------
; 函数名: GetFATEntry(根据当前FAT表项索引处下一个FAT表项)
;----------------------------------------------------------------------------
; 作用:
;	找到序号为 ax 的 Sector 在 FAT 中的条目, 结果放在 ax 中
;	需要注意的是, 中间需要读 FAT 的扇区到 es:bx 处, 所以函数一开始保存了 es 和 bx
Func_GetFATEntry:
	push	es
	push	bx
	push	ax
    mov ax, 00h
	mov	es, ax		; es = 00h
	pop	ax          ; 恢复ax
	mov	byte [Odd], 0
	mov	bx, 3
	mul	bx			; dx:ax = ax * 3
	mov	bx, 2
	div	bx			; dx:ax / 2  ==>  ax <- 商, dx <- 余数
	cmp	dx, 0       
    ; ax里面保存的是Fat[ax]相对于fat1表的字节偏移量, 每个fat[ax]占3个字节
	jz	LABEL_EVEN
	mov	byte [Odd], 1 
LABEL_EVEN:;偶数
	; 现在 ax 中是 FATEntry 在 FAT 中的偏移量,下面来
	; 计算 FATEntry 在哪个扇区中(FAT占用不止一个扇区)
	xor	dx, dx			
	mov	bx, [BPB_BytesPerSec]        ; 确定fat[ax]在哪个扇区
	div	bx ; dx:ax / BPB_BytsPerSec
		   ;  ax <- 商 (FATEntry 所在的扇区相对于 FAT 的扇区号)
		   ;  dx <- 余数 (FATEntry 在扇区内的偏移)。
	push	dx
	mov	bx, 8000h ; bx <- 0 于是, es:bx
	
    add	ax, SectorNumOfFAT1Start ; ax = FATEntry 所在的扇区号(LBA
	mov	cl, 2
	
    call	Func_ReadOneSector ; 读取 FATEntry 所在的扇区, 一次读两个, 避免在边界
			; 发生错误, 因为一个 FATEntry 可能跨越两个扇区
	
    pop	dx
	add	bx, dx      
	mov	ax, [es:bx] ;具体到索引到fat[ax]的字节
	cmp	byte [Odd], 1
	jnz	LABEL_EVEN_2
	shr	ax, 4       ; 根据奇偶消除间隙
LABEL_EVEN_2:
	and	ax, 0FFFh   ; 只取前12个字节
	pop	bx
	pop	es
	ret




;=======	tmp variable
RootDirSizeForLoop	dw	RootDirSectors
SectorNo		dw	0
Odd			db	0

;=======	display messages
StartBootMessage:	db	"Start Boot"
NoLoaderMessage:	db	"ERROR:No LOADER Found"
LoaderFileName:		db	"LOADER  BIN",0

times (510 - ($ - $$)) db 0

dw  0aa55h