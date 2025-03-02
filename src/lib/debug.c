#include "toolkit.h"
#include "devkit.h"

extern serial_t serials[2];

void debugk(cstr_t file, cstr_t func, s32_t line, cstr_t fmt, ...)
{
    va_list args;
    char_t debugk_buf[1024] = {0};

    s32_t i = sprintf(debugk_buf, "[%s:%d %s] ", strrchr(file,'/') + 1, line,func);
    va_start(args, fmt);
    i += vsprintf(debugk_buf + i, fmt, args);
    va_end(args);
    serial_write(&serials[0], debugk_buf, i);
}

void user_spin(str_t filename, cstr_t func, u64_t line, cstr_t condition)
{
    system_error("[%s %d:%s]: %s", strrchr(filename,'/') + 1, line, func, condition);
    // color_printk(RED, BLACK,  "[%s %d:%s]: %s", strrchr(filename,'/') + 1, line, func, condition);
    cli(); 
    while(1);
}
