#ifndef __DIRENT_H__
#define __DIRENT_H__
#include "dirent.h"

// 目录文件的句柄
struct DIR
{
    int fd;
    int buf_pos;
    int buf_end;
    char buf[256];
};

// direntory，抽象出来的目录文件属性
struct dirent
{
    long d_offset;
    long d_type;
    long d_namelen;
    char d_name[];
};

struct DIR* opendir(const char* path);
int closedir(struct DIR* dir);
struct dirent* readdir(struct DIR*dir);

#endif