/***************************************************
*		版权声明
*
*	本操作系统名为：MINE
*	该操作系统未经授权不得以盈利或非盈利为目的进行开发，
*	只允许个人学习以及公开交流使用
*
*	代码最终所有权及解释权归田宇所有；
*
*	本模块作者：	田宇
*	EMail:		345538255@qq.com
*
*
***************************************************/

#ifndef __STDIO_H__

#define __STDIO_H__
#include <stdarg.h>

#define	SEEK_SET	0	/* Seek relative to start-of-file */
#define	SEEK_CUR	1	/* Seek relative to current position */
#define	SEEK_END	2	/* Seek relative to end-of-file */

#define SEEK_MAX	3

int putstring(int FRcolor, char *string);
int printf(const char *fmt, ...);
int sprintf(char * buf,const char * fmt,...);
int vsprintf(char * buf,const char *fmt, va_list args);


#endif
