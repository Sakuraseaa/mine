#ifndef _DEBUG_H_
#define _DEBUG_H_

void debugk(char *file, int line, const char *fmt, ...);

#define DEBUGK(fmt, args...) debugk(__BASE_FILE__, __LINE__, fmt, ##args)
// #define DEBUGK(fmt, args...) debugk(__FILE__, __LINE__, fmt, ##args)
// #define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#endif