#ifndef _PRINTK_F_H_
#define _PRINTK_F_H_

void putchar(u32_t *fb, int Xsize, int x, int y, u32_t FRcolor, u32_t BKcolor, u8_t font);
int skip_atoi(const char **s);
int sprintf(char *buf, const char *fmt, ...);
// int vsprintf(char *buf, const char *fmt, va_list args) __attribute__((force_stack_args));
int vsprintf(char *buf, const char *fmt, va_list args);
int color_printk(u32_t FRcolor, u32_t BKcolor, const char *fmt, ...);
void frame_buffer_init();

#endif // _PRINTK_F_H_