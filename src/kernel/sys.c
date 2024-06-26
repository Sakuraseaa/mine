#include "time.h"
#include "stab.h"
#include "execv.h"
#include "errno.h"
#include "printk.h"
#include "fcntl.h"
#include "lib.h"
#include "VFS.h"
#include "fat32.h"
#include "stdio.h"
#include "keyboard.h"
#include "sys.h"
#include "debug.h"
// 系统调用有关
/*
normal
system call number:	rax
arg1:	rdi
arg2:	rsi
arg3:	rdx
arg4:	rcx
arg5:	r8
arg6:	r9

sysenter need rdx(rip) rcx(rsp)
syscall need rcx(rip) r11(rflags)

xchg rdx to r10, rcx to r11
*/

unsigned long no_system_call(void)
{
    color_printk(RED, BLACK, "no_system_call is calling\n");
    return -ENOSYS;
}

/*
__asm__	(
".global puts	\n\t"
".type	puts,	@function \n\t"
"puts:		\n\t"
"pushq	%r10	\n\t"
"pushq	%r11	\n\t"
"movq	$__NR_puts	,	%rax	\n\t"
"leaq	sysexit_return_address(%rip),	%r10	\n\t"
"movq	%rsp,	%r11		\n\t"
"sysenter			\n\t"
"sysexit_return_address:	\n\t"
"xchgq	%rdx,	%r10	\n\t"
"xchgq	%rcx,	%r11	\n\t"
"popq	%r11	\n\t"
"popq	%r10	\n\t"
);
*/

unsigned long sys_sleep(unsigned int seconds)
{
    time_sleep(seconds);
    return 0;
}

long sys_getpid(void)
{
    return current->pid;
}

unsigned long sys_putstring(char *string)
{
    color_printk(WHITE, BLACK, string);
    // color_printk(ORANGE, WHITE, "%s", string);

    return 0;
}

/**
 * @brief VFS的文件打开函数 = 给本进程要打开的文件filename创建文件描述符，文件描述符是进程私有的
 *  目录项结构,inode结构的缓存与释放是个问题
 *  为系统增加缓冲区功能
 * @param filename 需要打开文件的路径
 * @param flags  文件操作标志位，描述文件的访问模式和操作模式,WRITE,READ,TRUNC, APPEND
 * @return unsigned long 返回一个最小未使用的正整数来代表这个文件对应的文件描述符，执行失败返回-1
 */
unsigned long sys_open(char *filename, int flags)
{
    char *path = NULL;
    long pathlen = 0;
    long error = 0;
    struct dir_entry *Parent_dentry = NULL, *Child_dentry = NULL;
    int path_flags = 0;
    struct dir_entry *dentry = NULL;
    struct file *filp = NULL;
    struct file **f = NULL;
    int fd = -1; // 文件描述符
    int i;

    // a. 把目标路径名从应用层复制到内核层
    path = (char *)kmalloc(PAGE_4K_SIZE, 0);
    if (path == NULL)
        return -ENOMEM;
    memset(path, 0, PAGE_4K_SIZE);
    pathlen = strlen(filename);
    // pathlen = strnlen_user(filename, PAGE_4K_SIZE); 为了在内核中也可以使用sys_open(), 目前先注释进行调试
    if (pathlen <= 0)
    {
        kfree(path);
        return -EFAULT;
    }
    else if (pathlen >= PAGE_4K_SIZE)
    {
        kfree(path);
        return -ENAMETOOLONG;
    }
    // strncpy_from_user(filename, path, pathlen);
    strncpy(path, filename, pathlen);

    // b.获取path文件的目录项,
    if (flags & O_CREAT) // 根据是否创建新文件判断得到 文件的目录项 还是 文件所属目录的目录项
        path_flags = 1;

    dentry = path_walk(path, path_flags, &Child_dentry); // b.2得到目录项

    if (dentry == NULL)
        return -ENOENT;
    if (!(flags & O_DIRECTORY) && dentry->dir_inode->attribute == FS_ATTR_DIR)
        return -EISDIR;
    if((flags & O_DIRECTORY) && (dentry->dir_inode->attribute != FS_ATTR_DIR))
        return -ENOTDIR;


    if (flags & O_CREAT)
    {
        Parent_dentry = dentry;
        // 创建文件
        if (Parent_dentry->dir_inode->inode_ops && Parent_dentry->dir_inode->inode_ops->create)
            Parent_dentry->dir_inode->inode_ops->create(Parent_dentry->dir_inode, Child_dentry, 0);
        else
            return -EACCES;
    }
    kfree(path);

    // c.为目标文件,目标进程创建文件描述符, filp什么意思？file description?
    filp = (struct file *)kmalloc(sizeof(struct file), 0);
    memset(filp, 0, sizeof(struct file));
    filp->dentry = dentry;
    filp->mode = flags;

    if (dentry->dir_inode->attribute & FS_ATTR_DEVICE_KEYBOARD)
        filp->f_ops = &keyboard_fops;
    else
        filp->f_ops = dentry->dir_inode->f_ops;

    if (filp->f_ops && filp->f_ops->open)
        error = filp->f_ops->open(dentry->dir_inode, filp); // 让具体的文件系统，比如fat32执行特定的打开操作

    if (error != 1)
    { // 内核只释放了文件描述符占用的内存空间，而未释放inode结构和dentry结构占用的内存空间
        // 因为释放它们是一个漫长的过程，其中必将设计路径所以内所有结构的回收，缓存，销毁等管理细节
        kfree(filp);
        return -EFAULT;
    }

    // O_TRUNC改变了文件的长度
    if (filp->mode & O_TRUNC)
        filp->dentry->dir_inode->file_size = 0;
    // O_APPEND改变文件的当前访问位置
    if (filp->mode & O_APPEND)
        filp->position = filp->dentry->dir_inode->file_size;
    // d.在进程PCB的描述符数组中，记录新创建出的文件描述符
    f = current->file_struct;

    for (i = 0; i < TASK_FILE_MAX; i++)
        if (f[i] == NULL)
        {
            fd = i;
            break;
        }

    if (i == TASK_FILE_MAX)
    {
        kfree(filp);
        ////reclaim struct index_node & struct dir_entry
        return -EMFILE;
    }
    f[fd] = filp;

    return fd; // 返回文件描述符数组下标
}

/**
 * @brief 关闭文件描述符 = 释放文件描述符拥有的资源，切断进程和文件描述符之间的关联
 *
 * @param fd 文件描述符
 * @return unsigned long 如果close函数执行成功，则返回0，否则返回-1，errno变量会记录错误码
 */
unsigned long sys_close(int fd)
{
    struct file *filp = NULL;
  //  color_printk(GREEN, BLACK, "sys_close:%d\n", fd);
    if (fd < 0 || fd >= TASK_FILE_MAX)
        return -EBADF;

    filp = current->file_struct[fd];
    if (filp->f_ops && filp->f_ops->close)
        filp->f_ops->close(filp->dentry->dir_inode, filp);
    kfree(filp);
    current->file_struct[fd] = NULL;

    return 0;
}

/**
 * @brief 从fd指定的文件中读取count字节的数据，并把数据保存在缓存区buf中
 * read函数总是通过文件描述符来对文件进行读取操作, 因此在调用read函数前必须使用open函数获取目标文件的文件描述符
 * @param fd 文件描述符句柄
 * @param buf 读取数据缓冲区
 * @param count 读取数据的长度
 * @return unsigned long
 */
unsigned long sys_read(int fd, void *buf, long count)
{
    struct file *filp = NULL;
    unsigned long ret = 0;

    if (fd < 0 || fd >= TASK_FILE_MAX)
        return -EBADF;
    if (count < 0)
        return -EINVAL;

    filp = current->file_struct[fd];
    if (filp->f_ops && filp->f_ops->read)
        ret = filp->f_ops->read(filp, buf, count, &filp->position);

    DEBUGK("read_fd:%d read_size:%d\n", fd, ret);

    return ret;
}

int sys_stat(const char *pathname, struct stat *statbuf);
int sys_fstat(int fd, struct stat *statbuf);

// 写文件函数
unsigned long sys_write(int fd, void *buf, long count)
{
    struct file *filp = NULL;
    unsigned long ret = 0;

    if (fd < 0 || fd >= TASK_FILE_MAX)
        return -EBADF;
    if (count < 0)
        return -EINVAL;

    filp = current->file_struct[fd];
    if (filp->f_ops && filp->f_ops->write)
        ret = filp->f_ops->write(filp, buf, count, &filp->position);


    DEBUGK("written_fd:%d written_size:%d\n", fd, ret);

    return ret; // 成功的话返回的是写入的字节数
}

/**
 * @brief 设置文件的当前访问位置， offset 和 whence组成访问位置
 *  SEEK_SET 文件的起始位置/SEEK_CUR 文件的当前位置/SEEK_END 文件的末尾位置
 * @param filds 指定文件描述符
 * @param offset 偏移值
 * @param whence 基地址
 * @return unsigned long 设置成功，返回距离文件起始位置的偏移/否则不改变访问返回-1/errno变量会记录错误码
 */
unsigned long sys_lseek(int filds, long offset, int whence)
{
    struct file *filp = NULL;
    unsigned long ret = 0;
    // color_printk(GREEN, BLACK, "sys_lseek:%d\n", filds);
    if (filds < 0 || filds >= TASK_FILE_MAX)
        return -EBADF;
    if (whence < 0 || whence >= SEEK_MAX)
        return -EINVAL;

    filp = current->file_struct[filds];

    if (filp->f_ops && filp->f_ops->lseek)
        ret = filp->f_ops->lseek(filp, offset, whence);

    return ret;
}

/**
 * @brief fork()会为父进程(当前进程)创建了一个子进程，这个子进程将复制父进程的绝大部的内容，
 * 当fork函数执行后，父/子进程均可独立运行
 */
unsigned long sys_fork()
{
    // 索引到父进程的内核栈执行现场
    struct pt_regs *regs = (struct pt_regs *)current->thread->rsp0 - 1;
    // color_printk(GREEN, BLACK, "sys_fork\n");
    // regs.rsp 是指向哪里呢？
    return do_fork(regs, 0, regs->rsp, 0);
}

/**
 * @brief vfork()创建的子进程与父进程共享地址空间，当vfork函数执行后，子进程无法独立运行
 *  必须与exec类函数联合使用
 * @return unsigned long
 */
unsigned long sys_vfork()
{
    struct pt_regs *regs = (struct pt_regs *)current->thread->rsp0 - 1; // 这个是什么东西 ？
    color_printk(GREEN, BLACK, "sys_vfork\n");
    return do_fork(regs, CLONE_VM | CLONE_FS | CLONE_SIGNAL, regs->rsp, 0);
}

/**
 * @brief 进行堆的管理
 *        if(brk == 0) ->说明进程希望获取堆的起始地址
 *        else if(brk < current->mm->end_brk) -> 说明进程希望释放一部分堆地址空间
 *        else 执行 do_brk() -> expand brk space
 * @param brk
 * @return unsigned long 新的堆空间结束地址
 */
unsigned long sys_brk(unsigned long brk)
{
    unsigned long new_brk = PAGE_2M_ALIGN(brk);
    // color_printk(GREEN, BLACK, "sys_brk:%018lx\n", brk);
    // color_printk(RED, BLACK, "brk:%#0x18lx, new_brk:%#018lx,current->mm->end_brk:%#018lx", brk, new_brk, current->mm->end_brk);
    if (new_brk == 0) // return brk base address
        return current->mm->start_brk;
    if (new_brk < current->mm->end_brk) // release brk space
        return 0;

    new_brk = do_brk(current->mm->end_brk, new_brk - current->mm->end_brk); // expand brk space

    current->mm->end_brk = new_brk;
    // color_printk(RED, BLACK, "brk:%#0x18lx, new_brk:%#018lx,current->mm->end_brk:%#018lx", brk, new_brk, current->mm->end_brk);
    return new_brk;
}

unsigned long sys_reboot(unsigned long cmd, void *arg)
{
    color_printk(GREEN, BLACK, "sys_reboot\n");
    switch (cmd)
    {
    case SYSTEM_REBOOT:
        io_out8(0x64, 0xFE);
        break;

    case SYSTEM_POWEROFF:
        color_printk(RED, BLACK, "sys_reboot cmd SYSTEM_POWEROFF\n");
        break;

    default:
        color_printk(RED, BLACK, "sys_reboot cmd ERROR!\n");
        break;
    }
}

extern int fill_dentry(void* buf, char*name, long namelen, long offset);
unsigned long sys_getdents(int fd, void* dirent, long count)
{
    struct file* filp = NULL;
    unsigned long ret = 0;
    // color_printk(GREEN, BLACK, "sys_getdents:%d\n",fd);
    if(fd < 0 || fd > TASK_FILE_MAX)
        return -EBADF;
    if(count < 0)
        return -EINVAL;
    filp = current->file_struct[fd];

    if(filp->f_ops && filp->f_ops->readdir)
        ret = filp->f_ops->readdir(filp, dirent, &fill_dentry);
    return ret;
}

unsigned long sys_chdir(char* filename)
{
    char* path = NULL;
    long pathlen = 0;
    struct dir_entry* dentry = NULL;

    color_printk(GREEN, BLACK, "sys_chdir\n");
    path = (char*) kmalloc(PAGE_4K_SIZE, 0);
    if(path == NULL)
        return -ENOMEM;
    memset(path, 0, PAGE_4K_SIZE);

    pathlen = strnlen_user(filename, PAGE_4K_SIZE);
    if(pathlen <= 0)
    {
        kfree(path);
        return -EFAULT;
    } else if(pathlen >= PAGE_4K_SIZE)
    {
        kfree(path);
        return -ENAMETOOLONG;
    }
    strncpy_from_user(filename, path, pathlen);
    dentry = path_walk(path, 0, NULL);
    kfree(path);

    if(dentry == NULL)
        return -ENOENT;
    if(dentry->dir_inode->attribute != FS_ATTR_DIR)
        return -ENOTDIR;

    return 0;
}


unsigned long sys_execve()
{
    char* pathname = NULL;
    long pathlen = 0;
    long error = 0;
    struct pt_regs* regs = (struct pt_regs*)current->thread->rsp0 - 1;
    
    // color_printk(GREEN, BLACK, "sys_execve\n");

    pathname = (char*)kmalloc(PAGE_4K_SIZE, 0);
    if(pathname == NULL)
        return -ENOMEM;
    
    memset(pathname, 0, PAGE_4K_SIZE);
    pathlen = strnlen_user((char*)regs->rdi, PAGE_4K_SIZE);

    if(pathlen <= 0)
    {
        kfree(pathname);
        return -EFAULT;
    }else if(pathlen >= PAGE_4K_SIZE)
    {
        kfree(pathname);
        return -ENAMETOOLONG;
    }


    strncpy_from_user((char*)regs->rdi, pathname, pathlen);
    error = do_execve(regs, pathname, (char**)regs->rsi, NULL);

    kfree(pathname);
    return error;
}

unsigned long sys_wait4(unsigned long pid, int *status, int options,void *rusage)
{
    long retval = 0;
    struct task_struct* child = NULL;
    struct task_struct* tsk = NULL;

    // color_printk(GREEN, BLACK,"sys_wait4\n");
    for(tsk =&init_task_union.task; tsk->next != &init_task_union.task; tsk = tsk->next)
    {
        if(tsk->next->pid == pid)
        {
            child = tsk->next;
            break;
        }
    }

    if( child == NULL )   return -ECHILD;
    if( options != 0 )    return -EINVAL;
    
    // 直到子进程的成为僵尸进程，才会继续进行sys_wait4函数流程
    while( child->state != TASK_ZOMBIE )
        interruptible_sleep_on(&current->wait_childexit); // 阻塞
    

    copy_to_user(&child->exit_code, status, sizeof(long));
    tsk->next = child->next; // 在PCB列表中，删除掉当前进程PCB
    exit_mm(child);
    kfree(child);

    return retval;
}

unsigned long sys_exit(int exit_code)
{
    // color_printk(GREEN, BLACK, "sys_exit\n");
    return do_exit(exit_code);
}


