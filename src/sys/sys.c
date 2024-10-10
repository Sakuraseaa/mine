#include "toolkit.h"
#include "fskit.h"
#include "syskit.h"
#include "devkit.h"
#include "arch_x86kit.h"
#include "syskit.h"
#include "kernelkit.h"
#include "test.h"
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

u64_t no_system_call(void)
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

u64_t sys_sleep(u64_t seconds)
{
    time_sleep(seconds);
    return 0;
}

u64_t sys_getpid(void)
{
    return current->pid;
}

u64_t sys_putstring(u32_t FRcolor, s8_t *string)
{

    color_printk(FRcolor, BLACK, string);

    return 0;
}

/**
 * @brief sys_getcwd用于将当前运行线程的工作目录的绝对路径写入到buf中
 *
 * @param buf 由调用者提供存储工作目录路径的缓冲区, 若为NULL则将由操作系统进行分配
 * @param size 若调用者提供buf, 则size为buf的大小
 * @return char* 若成功且buf为NULL, 则操作系统会分配存储工作目录路径的缓冲区, 并返回首地址; 若失败则为NULL
 */
s8_t *sys_getcwd(str_t buf, u64_t size) {
    assert(buf != nullptr);
    
    dir_entry_t* dir = current->i_pwd;
    dir_entry_t* par_dir = dir->parent;
    if(par_dir == dir) {
        buf[0] = '/';
        buf[1] = 0;
        return buf;
    }
    // ERROR! :: 此处的buf是用户数据
    //
    memset(buf, 0, size);
    char full_path_reverse[64] = {0}; // all right, i think here must use heap-memory and buffer mechanism
    
    // 从子目录开始逐层向上找, 一直找到根目录为止, 每次查找都会把当前的目录名复制到full_path_reverse中
    // 例如子目录现在是"/fd1/fd1.1/fd1.1.1/fd1.1.1.1",
    // 则运行结束之后, full_path_reverse为 "/fd1.1.1.1/fd1.1.1/fd1.1/fd1"
    while(par_dir != dir) {
        strcat(full_path_reverse, "/");
        strcat(full_path_reverse, dir->name);
        dir = par_dir;
        par_dir = par_dir->parent;
    }

    /* 至此full_path_reverse中的路径是反着的,
     * 即子目录在前(左),父目录在后(右) ,
     * 现将full_path_reverse中的路径反置 */
    s8_t *last_slash = nullptr; // 用于记录字符串中最后一个 / 的地址
    
    // 把full_path_reverse从后向前遇见 / 就截断一下
    // 添加到buf尾巴后面
    while ((last_slash = strrchr(full_path_reverse, '/')))
    {
        s32_t len = strlen(buf);

        strcpy(buf + len, last_slash);
        // 最后一位设置为0, 这样就是下一次strrchr就从这里开始查起
        *last_slash = 0;
    }

    return buf;
}


/**
 * @brief VFS的文件打开函数 = 给本进程要打开的文件filename创建文件描述符，文件描述符是进程私有的
 *  目录项结构,inode结构的缓存与释放是个问题
 *  为系统增加缓冲区功能
 * @param filename 需要打开文件的路径
 * @param flags  文件操作标志位，描述文件的访问模式和操作模式,WRITE,READ,TRUNC, APPEND
 * @return u64_t 返回一个最小未使用的正整数来代表这个文件对应的文件描述符，执行失败返回-1
 */
u64_t sys_open(str_t filename, s32_t flags)
{
    s8_t *path = nullptr;
    s64_t pathlen = 0;
    s64_t error = 0;
    dir_entry_t *Parent_dentry = nullptr, *Child_dentry = nullptr;
    s32_t path_flags = 0;
    dir_entry_t *dentry = nullptr;
    file_t *filp = nullptr;
    file_t **f = nullptr;
    s32_t fd = -1; // 文件描述符
    s32_t i = 0;

    // a. 把目标路径名从应用层复制到内核层
    path = (char *)knew(PAGE_4K_SIZE, 0);
    if (path == nullptr)
        return -ENOMEM;
    memset(path, 0, PAGE_4K_SIZE);
    pathlen = strlen(filename);
    // pathlen = strnlen_user(filename, PAGE_4K_SIZE); 为了在内核中也可以使用sys_open(), 目前先注释进行调试
    if (pathlen <= 0)
    {
        kdelete(path, PAGE_4K_SIZE);
        return -EFAULT;
    }
    else if (pathlen >= PAGE_4K_SIZE)
    {
        kdelete(path, PAGE_4K_SIZE);
        return -ENAMETOOLONG;
    }
    // strncpy_from_user(filename, path, pathlen);
    strncpy(path, filename, pathlen);

    // b.获取path文件的目录项,
    if (flags & O_CREAT) // 根据是否创建新文件判断得到 文件的目录项 还是 文件所属目录的目录项
        path_flags = 1;

    dentry = path_walk(path, path_flags, &Child_dentry); // b.2得到目录项
    kdelete(path, PAGE_4K_SIZE);
    
    if (dentry == nullptr)
        return -ENOENT;

    if (flags & O_CREAT)
    {
        Parent_dentry = dentry;
        // 创建文件
        if (Parent_dentry->dir_inode->inode_ops && Parent_dentry->dir_inode->inode_ops->create)
            Parent_dentry->dir_inode->inode_ops->create(Parent_dentry->dir_inode, Child_dentry, 0);
        else
            return -EACCES;
        
        dentry = Child_dentry;
        goto sys_open_over_judge;
    }


    if (!(flags & O_DIRECTORY) && dentry->dir_inode->attribute == FS_ATTR_DIR)
        return -EISDIR;
    if((flags & O_DIRECTORY) && (dentry->dir_inode->attribute != FS_ATTR_DIR))
        return -ENOTDIR;

sys_open_over_judge:

    // c.为目标文件,目标进程创建文件描述符, filp什么意思？file description?
    filp = (file_t *)knew(sizeof(file_t), 0);
    memset(filp, 0, sizeof(file_t));
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
        kdelete(filp, sizeof(file_t));
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
        if (f[i] == nullptr)
        {
            fd = i;
            break;
        }

    if (i == TASK_FILE_MAX)
    {
        kdelete(filp, sizeof(file_t));
        ////reclaim inode_t & dir_entry_t
        return -EMFILE;
    }
    f[fd] = filp;

    return fd; // 返回文件描述符数组下标
}

u64_t sys_mkdir(str_t filename) {
    s8_t *path = nullptr;
    s64_t pathlen = 0;
    s64_t error = 0;
    dir_entry_t *Child_dentry = nullptr;
    dir_entry_t *dentry = nullptr;

    // a. 把目标路径名从应用层复制到内核层
    path = (char *)knew(PAGE_4K_SIZE, 0);
    if (path == nullptr)
        return -ENOMEM;
    memset(path, 0, PAGE_4K_SIZE);
    pathlen = strlen(filename);
    // pathlen = strnlen_user(filename, PAGE_4K_SIZE); 为了在内核中也可以使用sys_open(), 目前先注释进行调试
    if (pathlen <= 0) {
        kdelete(path, PAGE_4K_SIZE);
        return -EFAULT;
    } else if (pathlen >= PAGE_4K_SIZE) {
        kdelete(path, PAGE_4K_SIZE);
        return -ENAMETOOLONG;
    }
    // strncpy_from_user(filename, path, pathlen);
    strncpy(path, filename, pathlen);


    dentry = path_walk(path, 1, &Child_dentry); // b.2得到目录项
    kdelete(path, PAGE_4K_SIZE);
    if (dentry == nullptr)
        return -ENOENT;
    
    assert(Child_dentry->parent == dentry);
    if(dentry->dir_inode->inode_ops && dentry->dir_inode->inode_ops->mkdir)
        error = dentry->dir_inode->inode_ops->mkdir(dentry->dir_inode, Child_dentry, 0);

    return error;
}

u64_t sys_rmdir(str_t filename) {
    s8_t *path = nullptr;
    s64_t pathlen = 0;
    s64_t error = 0;
    dir_entry_t *Child_dentry = nullptr;
    dir_entry_t *dentry = nullptr;

    // a. 把目标路径名从应用层复制到内核层
    path = (char *)knew(PAGE_4K_SIZE, 0);
    if (path == nullptr)
        return -ENOMEM;
    memset(path, 0, PAGE_4K_SIZE);
    pathlen = strlen(filename);
    // pathlen = strnlen_user(filename, PAGE_4K_SIZE); 为了在内核中也可以使用sys_open(), 目前先注释进行调试
    if (pathlen <= 0) {
        kdelete(path, PAGE_4K_SIZE);
        return -EFAULT;
    }
    else if (pathlen >= PAGE_4K_SIZE) {
        kdelete(path, PAGE_4K_SIZE);
        return -ENAMETOOLONG;
    }
    // strncpy_from_user(filename, path, pathlen);
    strncpy(path, filename, pathlen);


    dentry = path_walk(path, 2, &Child_dentry); // b.2得到目录项
    kdelete(path, PAGE_4K_SIZE);
    
    if (dentry == nullptr)
        return -ENOENT;
    
    assert(ISDIR(Child_dentry->dir_inode->i_mode));
    assert(Child_dentry->parent == dentry);
    if(dentry->dir_inode->inode_ops && dentry->dir_inode->inode_ops->rmdir)
        error = dentry->dir_inode->inode_ops->rmdir(dentry->dir_inode, Child_dentry);
    if(error == OKay) {
        kdelete(Child_dentry->dir_inode, sizeof(inode_t));

        list_del(&Child_dentry->child_node);

        kdelete(Child_dentry, sizeof(dir_entry_t));
    }
    return error;
}

u64_t sys_unlink(str_t filename) {
    s8_t *path = nullptr;
    s64_t pathlen = 0;
    s64_t error = 0;
    dir_entry_t *Child_dentry = nullptr;
    dir_entry_t *dentry = nullptr;


    // a. 把目标路径名从应用层复制到内核层
    path = (char *)knew(PAGE_4K_SIZE, 0);
    if (path == nullptr)
        return -ENOMEM;
    memset(path, 0, PAGE_4K_SIZE);
    pathlen = strlen(filename);
    // pathlen = strnlen_user(filename, PAGE_4K_SIZE); 为了在内核中也可以使用sys_open(), 目前先注释进行调试
    if (pathlen <= 0) {
        kdelete(path, PAGE_4K_SIZE);
        return -EFAULT;
    }
    else if (pathlen >= PAGE_4K_SIZE) {
        kdelete(path, PAGE_4K_SIZE);
        return -ENAMETOOLONG;
    }
    // strncpy_from_user(filename, path, pathlen);
    strncpy(path, filename, pathlen);


    dentry = path_walk(path, 2, &Child_dentry); 
    kdelete(path, PAGE_4K_SIZE);
    if (dentry == nullptr)
        return -ENOENT;
    
    assert(Child_dentry->parent == dentry);

    if(ISDIR(Child_dentry->dir_inode->i_mode)) {
        
        color_printk(RED, WHITE, "Can't remove dir. please user \'mkdir\' again.\n"); 
        // 其实这句错误信息，应该在户空间通过得到的erron 输出，不适合使用内核输出
        
        return -EISDIR;
    }
    
    assert(ISREG(Child_dentry->dir_inode->i_mode));

    if(dentry->dir_inode->inode_ops && (dentry->dir_inode->inode_ops->unlink && Child_dentry->dir_ops->d_delete)) {
    
        error = dentry->dir_inode->inode_ops->unlink(dentry->dir_inode, Child_dentry); // delete file
        Child_dentry->dir_ops->d_delete(Child_dentry); // delete dir entry
    
    }

    return error;
}

/**
 * @brief 关闭文件描述符 = 释放文件描述符拥有的资源，切断进程和文件描述符之间的关联
 *
 * @param fd 文件描述符
 * @return u64_t 如果close函数执行成功，则返回0，否则返回-1，errno变量会记录错误码
 */
u64_t sys_close(s32_t fd)
{
    file_t *filp = nullptr;
    if (fd < 0 || fd >= TASK_FILE_MAX)
        return -EBADF;

    filp = current->file_struct[fd];
    if (filp->f_ops && filp->f_ops->close)
        filp->f_ops->close(filp->dentry->dir_inode, filp);
    kdelete(filp, sizeof(file_t));
    current->file_struct[fd] = nullptr;

    return 0;
}

/**
 * @brief 从fd指定的文件中读取count字节的数据，并把数据保存在缓存区buf中
 * read函数总是通过文件描述符来对文件进行读取操作, 因此在调用read函数前必须使用open函数获取目标文件的文件描述符
 * @param fd 文件描述符句柄
 * @param buf 读取数据缓冲区
 * @param count 读取数据的长度
 * @return u64_t
 */
u64_t sys_read(s32_t fd, void *buf, s64_t count)
{
    file_t *filp = nullptr;
    u64_t ret = 0;

    if (fd < 0 || fd >= TASK_FILE_MAX)
        return -EBADF;
    if (count < 0)
        return -EINVAL;

    filp = current->file_struct[fd];
    if (filp->f_ops && filp->f_ops->read)
        ret = filp->f_ops->read(filp, buf, count, &filp->position);

    return ret;
}

u64_t sys_getNow(void) {
    return NOW();
}

// 写文件函数
u64_t sys_write(s32_t fd, void *buf, s64_t count)
{
    file_t *filp = nullptr;
    u64_t ret = 0;

    if (fd < 0 || fd >= TASK_FILE_MAX)
        return -EBADF;
    if (count < 0)
        return -EINVAL;

    filp = current->file_struct[fd];
    if (filp->f_ops && filp->f_ops->write)
        ret = filp->f_ops->write(filp, buf, count, &filp->position);

    return ret; // 成功的话返回的是写入的字节数
}

/**
 * @brief 设置文件的当前访问位置， offset 和 whence组成访问位置
 *  SEEK_SET 文件的起始位置/SEEK_CUR 文件的当前位置/SEEK_END 文件的末尾位置
 * @param filds 指定文件描述符
 * @param offset 偏移值
 * @param whence 基地址
 * @return u64_t 设置成功，返回距离文件起始位置的偏移/否则不改变访问返回-1/errno变量会记录错误码
 */
u64_t sys_lseek(s32_t filds, s64_t offset, s32_t whence)
{
    file_t *filp = nullptr;
    u64_t ret = 0;
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
u64_t sys_fork()
{
    // 索引到父进程的内核栈执行现场
    pt_regs_t *regs = (pt_regs_t *)current->thread->rsp0 - 1;
    // color_printk(GREEN, BLACK, "sys_fork\n");
    // regs.rsp 是指向哪里呢？
    return do_fork(regs, 0, regs->rsp, 0);
}

/**
 * @brief vfork()创建的子进程与父进程共享地址空间，当vfork函数执行后，子进程无法独立运行
 *  必须与exec类函数联合使用
 * @return u64_t
 */
u64_t sys_vfork()
{
    pt_regs_t *regs = (pt_regs_t *)current->thread->rsp0 - 1; // 这个是什么东西 ？
    color_printk(GREEN, BLACK, "sys_vfork\n");
    return do_fork(regs, CLONE_VM | CLONE_FS | CLONE_SIGNAL, regs->rsp, 0);
}

/**
 * @brief 进行堆的管理
 *        if(brk == 0) ->说明进程希望获取堆的起始地址
 *        else if(brk < current->mm->end_brk) -> 说明进程希望释放一部分堆地址空间
 *        else 执行 do_brk() -> expand brk space
 * @param brk
 * @return u64_t 新的堆空间结束地址
 */
u64_t sys_brk(u64_t brk)
{
    u64_t new_brk = PAGE_2M_ALIGN(brk);
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

u64_t sys_reboot(u64_t cmd, void *arg)
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
    return EOK;
}

extern s32_t fill_dentry(void* buf, s8_t*name, s64_t namelen, s64_t offset);
u64_t sys_getdents(s32_t fd, void* dirent, s64_t count)
{
    file_t* filp = nullptr;
    u64_t ret = 0;
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

u64_t sys_chdir(s8_t* filename)
{
    str_t path = nullptr;
    s64_t pathlen = 0;
    dir_entry_t* dentry = nullptr;

    path = (char*) knew(PAGE_4K_SIZE, 0);
    if(path == nullptr)
        return -ENOMEM;
    memset(path, 0, PAGE_4K_SIZE);

    pathlen = strnlen_user(filename, PAGE_4K_SIZE);
    if(pathlen <= 0) {
        kdelete(path, PAGE_4K_SIZE);
        return -EFAULT;
    } else if(pathlen >= PAGE_4K_SIZE) {
        kdelete(path, PAGE_4K_SIZE);
        return -ENAMETOOLONG;
    }

    strncpy_from_user(filename, path, pathlen);
    
    dentry = path_walk(path, 0, nullptr);
    if (dentry == nullptr)
        return -ENOENT;
    
    kdelete(path, PAGE_4K_SIZE);
    
    // 改变当前进程工作目录
    current->i_pwd = dentry;
    
    if(dentry == nullptr)
        return -ENOENT;
    if(dentry->dir_inode->attribute != FS_ATTR_DIR)
        return -ENOTDIR;

    return 0;
}


u64_t sys_execve()
{
    str_t pathname = nullptr;
    s64_t pathlen = 0;
    s64_t error = 0;
    pt_regs_t* regs = (pt_regs_t*)current->thread->rsp0 - 1;
    
    DEBUGK("sys_execve\n");

    pathname = (char*)knew(PAGE_4K_SIZE, 0);
    if(pathname == nullptr)
        return -ENOMEM;
    
    memset(pathname, 0, PAGE_4K_SIZE);
    pathlen = strnlen_user((char*)regs->rdi, PAGE_4K_SIZE);

    if(pathlen <= 0)
    {
        kdelete(pathname, PAGE_4K_SIZE);
        return -EFAULT;
    }else if(pathlen >= PAGE_4K_SIZE)
    {
        kdelete(pathname, PAGE_4K_SIZE);
        return -ENAMETOOLONG;
    }


    strncpy_from_user((char*)regs->rdi, pathname, pathlen);
    error = do_execve(regs, pathname, (char**)regs->rsi, nullptr);

    kdelete(pathname, PAGE_4K_SIZE);
    return error;
}

/**
 * @brief 释放内存空间结构体，
 * 		exit_mm()与copy_mm()的分配空间分布结构体初始化过程相逆
 * @param tsk
 */
void exit_mm(task_t *tsk)
{
	// u64_t *tmp4 = nullptr, *tmp3 = nullptr, *tmp2 = nullptr;
    // u64_t tmp1 = 0; // page address
	// size_t i = 0, j = 0, k = 0;
	// struct Page* p = nullptr;
	// if (tsk->flags & PF_VFORK)
	// 	return;

	// struct mm_struct *newmm = tsk->mm;
	// tmp4 = Phy_To_Virt(newmm->pgd);

    // // 😟 😋 🤬

    // /* recycle all memory pages. these include Data, Code, Stack, Heap...*/
    // /* 这里操作页表，还是有一点点小难度的。些许风霜罢了 🧐*/
	// for(i = 0; i < 256; i++) {	// 遍历 PML4 页表
	// 	if((*(tmp4 + i)) & PAGE_Present) {
	// 		tmp3 = Phy_To_Virt(*(tmp4 + i) & ~(0xfffUL)); // 屏蔽目录项标志位，获取PDPT页表地址
			
	// 		for (j = 0; j < 512; j++) { // 遍历 PDPT 页表
	// 			if((*(tmp3 + j)) & PAGE_Present) {
					
	// 				tmp2 = Phy_To_Virt(*(tmp3 + j) & ~(0xfffUL)) ; //遍历 PDT 页表项
	// 				for(k = 0; k < 512; k++) {
	// 					if((*(tmp2 + k)) & PAGE_Present) {
	// 						tmp1 = (*(tmp2 + k)); // 得到物理页
	// 						p = (memory_management_struct.pages_struct + (tmp1  >> PAGE_2M_SHIFT));
    //                         free_pages(p, 1); // 释放物理页
    //                     }
    //                 }
	// 				kdelete(tmp2); // 释放 PDT表
	// 			}
	// 		}
	// 		kdelete(tmp3); // 释放 PDPT 表 
	// 	}
	// }

	// kdelete(Phy_To_Virt(tsk->mm->pgd)); // release PMl4's memory

	// if (tsk->mm != nullptr)
	// 	kdelete(tsk->mm);
}

u64_t sys_wait4(u64_t pid, s32_t *status, s32_t options,void *rusage)
{
    s64_t retval = 0;
    task_t* child = nullptr;
    task_t* tsk = nullptr;

    // color_printk(GREEN, BLACK,"sys_wait4\n");
    for(tsk =&init_task_union.task; tsk->next != &init_task_union.task; tsk = tsk->next)
    {
        if(tsk->next->pid == pid)
        {
            child = tsk->next;
            break;
        }
    }

    if( child == nullptr )   return -ECHILD;
    if( options != 0 )    return -EINVAL;
    
    // 直到子进程的成为僵尸进程，才会继续进行sys_wait4函数流程
    while( child->state != TASK_ZOMBIE )
        interruptible_sleep_on(&current->wait_childexit); // 阻塞
    

    copy_to_user(&child->exit_code, status, sizeof(s64_t));
    tsk->next = child->next; // 在PCB列表中，删除掉当前进程PCB
    kdelete(child, sizeof(task_t)); // 回收PCB, 内核栈

    return retval;
}

u64_t sys_exit(s32_t exit_code)
{
    DEBUGK("sys_exit\n");
    return do_exit(exit_code);
}

inode_t *namei(s8_t* filename);
u64_t sys_stat(s8_t* filename, stat_t* statbuf) {

    inode_t* inode = namei(filename);

    if(inode == nullptr)
        return -ENOMEM;

    statbuf->size = inode->file_size;
    statbuf->uid = inode->uid;
    statbuf->atime = inode->atime;
    statbuf->ctime = inode->ctime;
    statbuf->mtime = inode->mtime;
    statbuf->gid = inode->gid;
    statbuf->nr = inode->nr;
    statbuf->dev = inode->dev;
    statbuf->mode = inode->i_mode;
    statbuf->rdev = 0; // 这个虚拟设备时干嘛的？ 虚拟内存 供给内核高速申请的？
    if(inode->sb->type == FS_TYPE_MINIX) {
        
        char* buf = (char*)inode->private_index_info;
        statbuf->nlinks = buf[13]; // minix文件系统的inode结构 中 第十三个字节记录着 本inode的目录项链接数
    
    }else
        statbuf->nlinks = 0;
    
    return 0;
}

u64_t sys_cleanScreen(void) {
    // 清屏命令
    memset(Pos.FB_addr,0, Pos.FB_length);
	Pos.XPosition = 0;
	Pos.YPosition = 0;

    return 0;
}

void dir_Tree(dir_entry_t* cur, s32_t depth) {
    color_printk(WHITE, BLACK, "|");
    
    dir_entry_t* child = nullptr;

    list_t* End = &cur->subdirs_list;
    list_t* node = End->next;
    if (cur->dir_inode->attribute == FS_ATTR_DIR)
    {
        color_printk(INDIGO, BLACK, " %s\n",cur->name);
    } else 
        color_printk(WHITE, BLACK, " %s\n",cur->name);
    
    for(; node != End; node = node->next) {
        child = container_of(node, dir_entry_t, child_node);

        // here is format print according to depth
        for(s32_t i = 0; i < depth; i++)
            color_printk(WHITE, BLACK, "  ");
        
        color_printk(WHITE, BLACK, "+");
        
        for(s32_t i = 0; i < depth; i++)
            color_printk(WHITE, BLACK, "-");
        
        color_printk(WHITE, BLACK, "->");
        //=======================================
        
        dir_Tree(child, depth + 1);
    }
}

u64_t sys_info(s8_t order) {
    
    switch (order)
    {
    case 'A':  // 显示系统目录项树
        dir_entry_t *parent = current_sb->root; // 父目录项
        dir_Tree(parent, 0);
        break;
    case 'B':// 显示本进程的虚拟内存 到 物理内存的映射
        test_show_vir_phy(current);
        break;
    default:
        break;
    }

    return 0;
}


u64_t sys_fstat(s32_t fd, stat_t *statbuf)
{
    return EOK;
}