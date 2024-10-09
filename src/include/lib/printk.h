#ifndef __PRINTK_H__
#define __PRINTK_H__

#include <stdarg.h>

#define ZEROPAD 1  /* pad with zero */
#define SIGN 2	   /* unsigned/signed long */
#define PLUS 4	   /* show plus */
#define SPACE 8	   /* space if plus */
#define LEFT 16	   /* left justified */
#define SPECIAL 32 /* 0x */
#define SMALL 64   /* use 'abcdef' instead of 'ABCDEF' */

#define is_digit(c) ((c) >= '0' && (c) <= '9')

#define WHITE 0x00ffffff  // 白
#define BLACK 0x00000000  // 黑
#define RED 0x00ff0000	  // 红
#define ORANGE 0x00ff8000 // 橙
#define YELLOW 0x00ffff00 // 黄
#define GREEN 0x0000ff00  // 绿
#define BLUE 0x000000ff	  // 蓝
#define INDIGO 0x0000ffff // 靛
#define PURPLE 0x008000ff // 紫

#define VBE_Phy_address 0xfd000000
/* e0000000 -bochs
fd000000  -qemu
*/
extern unsigned char font_ascii[256][16];

char buf[4096] = {0};

struct position
{
	int XResolution;
	int YResolution;

	int XPosition;
	int YPosition;

	int XCharSize;
	int YCharSize;

	unsigned int *FB_addr;
	unsigned long FB_length;

	fair_spinlock_t printk_lock;
} Pos;


#define do_div(n, base) ({ \
int __res; \
__asm__("divq %%rcx":"=a" (n),"=d" (__res):"0" (n),"1" (0),"c" (base)); \
__res; })

#define system_error(fmt, ...) color_printk(RED, BLACK, fmt, ##__VA_ARGS__)

#endif
