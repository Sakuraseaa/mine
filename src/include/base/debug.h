#ifndef _DEBUG_H_
#define _DEBUG_H_

void debugk(u8_t type, cstr_t file,cstr_t func, s32_t line, cstr_t fmt, ...);

#define LOG_INFO_INDEX 0
#define LOG_ERROR_INDEX 1
#define LOG_WARNING_INDEX 2
#define LOG_FAIL_INDEX 3
#define LOG_DEBUG_INDEX 4

#define ENABLE_DEBUG 1
#if ENABLE_DEBUG
    #define INFOK(fmt, ...) debugk(LOG_INFO_INDEX, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
    #define ERRORK(fmt, ...) debugk(LOG_ERROR_INDEX, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
    #define WARNK(fmt, ...) debugk(LOG_WARNING_INDEX, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
    #define FAILK(fmt, ...) debugk(LOG_FAIL_INDEX, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
    #define DEBUGK(fmt, ...) debugk(LOG_DEBUG_INDEX, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#else
    #define DEBUGK(fmt, ...) ((void)0) // 空操作，防止编译警告
    #define ERRORK(fmt, ...) ((void)0) // 空操作，防止编译警告
    #define WARNK(fmt, ...) ((void)0) // 空操作，防止编译警告
#endif

#endif