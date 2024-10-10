#ifndef __INIT_H__

#define __INIT_H__

#include "fskit.h"
#define NR_SCAN_CODES 0x80
#define MAP_COLS 2
#define PAUSEBREAK 1
#define PRINTSCREEN 2
#define OTHERKEY 4
#define FLAG_BREAK 0x80

s32_t shift_l = 0, shift_r = 0, ctrl_l = 0, ctrl_r = 0, alt_l = 0, alt_r = 0;

unsigned char pausebreak_scode[] = {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5};
#define SYSTEM_REBOOT (1UL << 0)
#define SYSTEM_POWEROFF (1UL << 1)
s32_t close(s32_t fildes);
s64_t read(s32_t fildes, void *buf, s64_t nbyte);
s64_t write(s32_t fildes, const void *buf, s64_t nbyte);
s64_t lseek(s32_t fildes, s64_t offset, s32_t whence);
s32_t open(const char *path, s32_t oflag);
s32_t mkdir(const char* path);
s32_t fork(void);
s32_t vfork(void);
u64_t reboot(u64_t cmd,void * arg);
s32_t putstring(u32_t FRcolor,char *string);
s32_t printf(const char *fmt, ...);
s32_t execve(const char* path, char* const argv[], char* const envp[]);
s64_t getpid();
void sleep(s64_t);
char *getcwd(char *buf, s32_t size);
s64_t stat(char* pahtname, stat_t* statBuf);
u64_t chdir(char* path);
#endif