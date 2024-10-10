#ifndef __DIRENT_H__
#define __DIRENT_H__

// 目录文件的句柄
typedef struct DIR
{
    s32_t fd;
    s32_t buf_pos;
    s32_t buf_end;
    char_t buf[256];
}dir_t;

// direntory，抽象出来的目录文件属性
typedef struct dirent
{
    s64_t nr;
    s64_t d_offset;
    s64_t d_namelen;
    char_t d_name[];
}dirent_t;

#endif