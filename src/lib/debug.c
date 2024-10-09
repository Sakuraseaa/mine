#include "toolkit.h"
#include "serial.h"
#include <stdarg.h>

static char debugk_buf[1024];
extern serial_t serials[2];

void debugk(const char *file, const char* func, int line, const char *fmt, ...)
{
    int i = sprintf(debugk_buf, "[%s %d:%s] ", strrchr(file,'/') + 1, line,func);
    serial_write(&serials[0], debugk_buf, i);


    va_list args;
    va_start(args, fmt);
    i = vsprintf(debugk_buf, fmt, args);
    va_end(args);
    serial_write(&serials[0], debugk_buf, i);
}

void user_spin(char *filename, const char *func, u64_t line, const char *condition)
{
    system_error("[%s %d:%s]: %s", strrchr(filename,'/') + 1, line, func, condition);
    // color_printk(RED, BLACK,  "[%s %d:%s]: %s", strrchr(filename,'/') + 1, line, func, condition);
    cli(); 
    while(1);
}
