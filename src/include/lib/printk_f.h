#ifndef _PRINTK_F_H_
#define _PRINTK_F_H_

void putchar(u32_t *fb, s32_t Xsize, s32_t x, s32_t y, u32_t FRcolor, u32_t BKcolor, u8_t font);
s32_t skip_atoi(cstr_t *s);
s32_t sprintf(str_t buf, cstr_t fmt, ...);
// int vsprintf(char *buf, const char *fmt, va_list args) __attribute__((force_stack_args));
s32_t vsprintf(str_t buf, cstr_t fmt, va_list args);
s32_t color_printk(u32_t FRcolor, u32_t BKcolor, cstr_t fmt, ...);
void frame_buffer_init();

#endif // _PRINTK_F_H_