#include "lib.h"
#include "signal.h"
#include "wait.h"
int wait(int *status)
{
    return wait4((int)-1, status, 0);
}

int waitpid(int pid, int* status, int options)
{
    return wait4(pid, status, options);
}

extern void sig_restore(void);
sighadler_t signal(long signum, sighadler_t handler) {
    if(signum < 1 && signum > NSIG) {
        printf("ERROR!!!! invaild signal_value");
        return NULL;
    }

    return SIGNAL(signum, handler, sig_restore);
}

int kill(long pid, long signum) {
    if(signum < 1 && signum > NSIG) {
        printf("ERROR!!!! invaild signal_value");
        return NULL;
    }

    return KILL(pid, signum);
}





#include "dirent.h"
#include "fcntl.h"
#include "lib.h"
#include "memory.h"

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
        dir = (struct DIR*)kmalloc(sizeof(struct DIR), 0);
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
    kfree(dir);
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