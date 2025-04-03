#include "toolkit.h"
#include "devkit.h"

extern serial_t serials[2];
static cstr_t debug_type[] = {
    "INFO",
    "ERROR",
    "WARNING",
};

void debugk(u8_t type, cstr_t file, cstr_t func, s32_t line, cstr_t fmt, ...)
{
    va_list args;
    char_t debugk_buf[1024] = {0};
    struct time tm;
    get_time(&tm);

    s32_t i = sprintf(debugk_buf, "[%04d/%02d/%02d %02d:%02d:%02d] [%s] [%s:%d] [%s] ", tm.year, tm.month, tm.day, tm.hour, 
    tm.minute, tm.second, debug_type[type],strrchr(file,'/') + 1, line, func);

    va_start(args, fmt);
    i += vsprintf(debugk_buf + i, fmt, args);
    va_end(args);
    debugk_buf[i] = '\n'; /* 添加换行符 */

    serial_write(&serials[0], debugk_buf, i + 1);
}

void user_spin(str_t filename, cstr_t func, u64_t line, cstr_t condition)
{
    system_error("[%s %d:%s]: %s", strrchr(filename,'/') + 1, line, func, condition);
    // color_printk(RED, BLACK,  "[%s %d:%s]: %s", strrchr(filename,'/') + 1, line, func, condition);
    cli(); 
    while(1);
}
