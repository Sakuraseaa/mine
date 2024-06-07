#ifndef __INIT_H__

#define __INIT_H__

#define NR_SCAN_CODES 0x80
#define MAP_COLS 2
#define PAUSEBREAK 1
#define PRINTSCREEN 2
#define OTHERKEY 4
#define FLAG_BREAK 0x80

int shift_l = 0, shift_r = 0, ctrl_l = 0, ctrl_r = 0, alt_l = 0, alt_r = 0;

unsigned char pausebreak_scode[] = {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5};


#define SYSTEM_REBOOT (1UL << 0)
#define SYSTEM_POWEROFF (1UL << 1)
int close(int fildes);
long read(int fildes, void *buf, long nbyte);
long write(int fildes, const void *buf, long nbyte);
long lseek(int fildes, long offset, int whence);
int open(const char *path, int oflag);
int fork(void);
int vfork(void);
unsigned long reboot(unsigned long cmd,void * arg);
int putstring(char *string);
int printf(const char *fmt, ...);

#endif