#ifndef _DEBUG_H_
#define _DEBUG_H_

void debugk(u8_t type, cstr_t file,cstr_t func, s32_t line, cstr_t fmt, ...);

#define DEBUG_INFO_INDEX 0
#define DEBUG_ERROR_INDEX 1
#define DEBUG_WARNING_INDEX 2

#define ENABLE_DEBUG 1
#if ENABLE_DEBUG
    #define DEBUGK(fmt, ...) debugk(DEBUG_INFO_INDEX, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
    #define ERRORK(fmt, ...) debugk(DEBUG_ERROR_INDEX, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
    #define WARNINGK(fmt, ...) debugk(DEBUG_WARNING_INDEX, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#else
    #define DEBUGK(fmt, ...) ((void)0) // 空操作，防止编译警告
    #define ERRORK(fmt, ...) ((void)0) // 空操作，防止编译警告
    #define WARNINGK(fmt, ...) ((void)0) // 空操作，防止编译警告
#endif

#endif