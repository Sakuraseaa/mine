#include "printk.h"
#include "serial.h"
#include <stdarg.h>

static char debugk_buf[1024];
extern serial_t serials[2];

void debugk(char *file, int line, const char *fmt, ...)
{
    int i = sprintf(debugk_buf, "[%s] [%d] ", file, line);
    serial_write(&serials[0], debugk_buf, i);


    va_list args;
    va_start(args, fmt);
    i = vsprintf(debugk_buf, fmt, args);
    va_end(args);
    serial_write(&serials[0], debugk_buf, i);

}