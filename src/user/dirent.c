#include "dirent.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "stddef.h"

/**
 * @brief 打开目录
 * 
 * @param path 目录文件的路径 
 * @return struct DIR* 目录属性结构体 
 */
struct DIR* opendir(const char* path)
{
    int fd = 0;
    struct DIR* dir = NULL;
    fd = open(path, O_DIRECTORY);
    
    if(fd >= 0)
        dir = (struct DIR*)malloc(sizeof(struct DIR), 0);
    else
        return NULL;

    memset(dir, 0, sizeof(struct DIR));
    
    dir->buf_pos = 0;
    dir->buf_end = 256;
    dir->fd = fd;

    return dir;
}

int closedir(struct DIR* dir)
{
    close(dir->fd);
    free(dir);
    return 0;
}

/**
 * @brief 读目录项
 * 
 * @param dir 
 * @return struct dirent* 
 */
struct dirent* readdir(struct DIR*dir)
{
    int len = 0;
    memset(dir->buf, 0, 256);
    len = getdents(dir->fd, (struct dirent*)dir->buf, 256);
    
    if(len > 0)
        return (struct dirent*)dir->buf;
    else
        return NULL;
}

