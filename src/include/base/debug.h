#ifndef _DEBUG_H_
#define _DEBUG_H_

void debugk(cstr_t file,cstr_t func, s32_t line, cstr_t fmt, ...);
#define DEBUGK(fmt, args...) debugk(__BASE_FILE__, __FUNCTION__, __LINE__, fmt, ##args)
// #define DEBUGK(fmt, args...) debugk(__BASE_FILE__, __LINE__, fmt, ##args)
// #define DEBUGK(fmt, args...) debugk(__FILE__, __LINE__, fmt, ##args)
// #define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#endif