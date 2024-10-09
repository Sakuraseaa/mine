#ifndef _PRINTK_F_H_
#define _PRINTK_F_H_

void putchar(unsigned int *fb, int Xsize, int x, int y, unsigned int FRcolor, unsigned int BKcolor, unsigned char font);
int skip_atoi(const char **s);
int sprintf(char *buf, const char *fmt, ...);
// int vsprintf(char *buf, const char *fmt, va_list args) __attribute__((force_stack_args));
int vsprintf(char *buf, const char *fmt, va_list args);
int color_printk(unsigned int FRcolor, unsigned int BKcolor, const char *fmt, ...);
void frame_buffer_init();

#endif // _PRINTK_F_H_