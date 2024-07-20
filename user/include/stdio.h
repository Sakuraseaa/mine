#ifndef __STDIO_H__

#define __STDIO_H__
#include <stdarg.h>
#define	SEEK_SET	0	/* Seek relative to start-of-file */
#define	SEEK_CUR	1	/* Seek relative to current position */
#define	SEEK_END	2	/* Seek relative to end-of-file */

#define SEEK_MAX	3

// printf
#define WHITE 0x00ffffff  // 白
#define BLACK 0x00000000  // 黑
#define RED 0x00ff0000	  // 红
#define ORANGE 0x00ff8000 // 橙
#define YELLOW 0x00ffff00 // 黄
#define GREEN 0x0000ff00  // 绿
#define BLUE 0x000000ff	  // 蓝
#define INDIGO 0x0000ffff // 靛
#define PURPLE 0x008000ff // 紫

int putstring(unsigned int FRcolor, const char *string);
int printf(const char *fmt, ...);
int sprintf(char * buf,const char * fmt,...);
int color_printf(unsigned int FRcolor, const char* fmt, ...);
int vsprintf(char * buf,const char *fmt, va_list args);

#endif
