#ifndef _DEBUG_H_
#define _DEBUG_H_
void debugk(cstr_t file,cstr_t func, s32_t line, cstr_t fmt, ...);

#if ENABLE_DEBUG
    #define DEBUGK(fmt, ...) debugk(__FILE__, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#else
    #define DEBUGK(fmt, ...) ((void)0) // 空操作，防止编译警告
#endif

#endif