#ifndef __DIRENT_H__
#define __DIRENT_H__

// 目录文件的句柄
struct DIR
{
    s32_t fd;
    s32_t buf_pos;
    s32_t buf_end;
    char buf[256];
};

// direntory，抽象出来的目录文件属性
struct dirent
{
    s64_t nr;
    s64_t d_offset;
    s64_t d_namelen;
    s8_t d_name[];
};

#endif