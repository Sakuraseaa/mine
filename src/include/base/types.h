#ifndef _BASTYPE_T_H
#define _BASTYPE_T_H

#define NULL 0
#define nullptr ((void *)0)

typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned int u32_t;
typedef unsigned long u64_t;
typedef char s8_t;
typedef short s16_t;
typedef int s32_t;
typedef long s64_t;
typedef long sint_t;
typedef u64_t uint_t;
typedef u64_t cpuflg_t;
typedef u64_t adr_t;
typedef u64_t size_t;
typedef char bool;
typedef sint_t bool_t;
typedef const char* cstr_t;
typedef char* str_t;
typedef char char_t;
typedef const char cchar_t;
typedef u64_t drv_t;
typedef u64_t mrv_t;
typedef sint_t drvstus_t;
typedef sint_t sysstus_t;
typedef sint_t hand_t;
typedef char* buf_t;
typedef u64_t size_t;
typedef u32_t reg_t;
typedef int pid_t;
typedef int dev_t;
typedef u64_t time_t;
typedef u64_t idx_t;
typedef s64_t fd_t;
typedef int err_t;
typedef u16_t mode_t; // 文件权限


#define ALIGN_UP8(x) (((x) + 7) & ~7)

typedef void (*inthandler_t)();
typedef drv_t (*i_handle_t)(uint_t int_nr);
typedef drv_t (*f_handle_t)(uint_t int_nr,void* sframe);
typedef drvstus_t (*intflthandle_t)(uint_t ift_nr,void* device,void* sframe);
typedef u64_t mmstus_t;

#define KLINE static inline
#define PUBLIC
#define private	static
#define EXTERN extern
#define KEXTERN extern
#define TRUE    1
#define	FALSE	0
#define DFCERRSTUS (-1) // default error status
#define DFCOKSTUS (0) // default ok status
#define NO_HAND (-1)
#define ALIGN(x, a)     (((x) + (a) - 1) & ~((a) - 1))

#define LKHEAD_T __attribute__((section(".head.text")))
#define LKHEAD_D __attribute__((section(".head.data")))
#define LKINIT 

#define EOS '\0' // 字符串结尾

#define false 0
#define true 1

#endif