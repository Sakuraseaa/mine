#ifndef __TYPES_H__
#define __TYPES_H__

#define NULL ((void *)0) // 空

#define EOS '\0' // 字符串结尾

#define false 0
#define true 1
typedef char bool;

// 弱符号
#define weak __attribute__((__weak__))

// 不会返回的函数
#define noreturn __attribute__((__noreturn__))

// 用于定义特殊的结构体
#define _packed __attribute__((packed))

// 用于省略函数的栈�?
#define _ofp __attribute__((optimize("omit-frame-pointer")))

#define _inline __attribute__((always_inline)) inline

typedef unsigned long size_t;

typedef char int8;
typedef short int16;
typedef int int32;
typedef long int64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

typedef int32 pid_t;
typedef int32 dev_t;

typedef u64 time_t;
typedef u64 idx_t;

typedef u16 mode_t; // 文件权限

typedef int64 fd_t;
typedef enum std_fd_t
{
    STDIN_FILENO,
    STDOUT_FILENO,
    STDERR_FILENO,
} std_fd_t;

typedef int32 off_t; // 文件偏移

typedef int err_t; // 错�??类型

#endif