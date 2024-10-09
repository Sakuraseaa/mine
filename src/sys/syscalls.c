#include "SYSCALL.h"

// 这里SYSCALL_COMMON定义了两次，若是未来的我怀疑了，使用gcc -E 本文件，看一下就行
// 第一次用于定义系统调用向量表system_call_table的处理函数名
#define SYSCALL_COMMON(nr, sym) extern unsigned long sym(void);
SYSCALL_COMMON(0, no_system_call)
#include "syscalls.h"
#undef SYSCALL_COMMON // 取消第一次定义

// 第二次SYSCALL_COMMON用于把第一次声明的处理函数加入到系统调用表里面
#define SYSCALL_COMMON(nr, sym) [nr] = sym,
#define MAX_SYSTEM_CALL_NR 256
typedef unsigned long (*system_call_t)(void);

system_call_t system_call_table[MAX_SYSTEM_CALL_NR] = {
    [0 ... MAX_SYSTEM_CALL_NR - 1] = no_system_call,
#include "syscalls.h"
};