#include "VFS.h"
#include "fat32.h"
#include "printk.h"
struct file_system_type filesystem = {"filesystem", 0};

// 当前文件系统的超级块
struct super_block *root_sb = NULL;
// 文件系统的注册
/**
 * @brief
 *
 * @param fs 文件系统类型元素
 * @return unsigned long
 */
unsigned long register_filesystem(struct file_system_type *fs)
{
    struct file_system_type *p = NULL;
    // 阻止重复注册相同类型文件系统
    for (p = &filesystem; p; p = p->next)
        if (!strcmp(fs->name, p->name))
            return 0;
    // 链表的头插法 - 把fs文件类型加入文件类型链表
    fs->next = filesystem.next;
    filesystem.next = fs;
    return 1;
}

/**
 * @brief 文件系统的挂载函数- function mount_root, 挂载名为name的文件系统 == 建立文件系统对应的超级块结构体super block
 *        调用该文件系统专有的 read_superblock 函数
 * @param name 文件系统名
 * @param DPTE 描述文件系统的分区表项
 * @param buf  FAT32文件系统的引导扇区
 * @return struct super_block* （fat32）文件系统的超级块
 */
struct super_block *mount_fs(char *name, struct Disk_Partition_Table_Entry *DPTE, void *buf)
{
    struct file_system_type *p = NULL;

    for (p = &filesystem; p; p = p->next)
        if (!strcmp(p->name, name))
            return p->read_superblock(DPTE, buf);

    return 0;
}

// 文件系统的注销
unsigned long unregister_filesystem(struct file_system_type *fs)
{
    struct file_system_type *p = &filesystem;

    while (p->next)
        if (p->next == fs)
        {
            p->next = p->next->next;
            fs->next = NULL;
            return 1;
        }
        else
            p = p->next;
    return 0;
}

/**
 * @brief 搜索文件name。
 *
 * @param name 文件名称
 * @param flags 当形参flags = 1时, path_walk函数返回目标父目录的目录项，否则返回目标目录项
 * @return struct dir_entry* 搜索失败返回NULL, dir_entry和dentry动态申请的内存，由上层调用者释放
 */
struct dir_entry *path_walk(char *name, unsigned long flags)
{
    char *tmpname = NULL;
    int tmpnamelen = 0;
    struct dir_entry *parent = root_sb->root;
    struct dir_entry *path = NULL;

    // 越过路径前的 '/'
    while (*name == '/')
        name++;
    // 路径若为空，则返回根目录
    if (!*name)
        return parent;

    // 此处为路径上的一系列文件，都创建了dir_entry结构体
    for (;;)
    {
        // 取得一层目录名字,存入dentryname缓冲区
        tmpname = name;
        while (*name && (*name != '/'))
            name++;
        tmpnamelen = name - tmpname;

        path = (struct dir_entry *)kmalloc(sizeof(struct dir_entry), 0);
        memset(path, 0, sizeof(struct dir_entry));

        // 准备好要找的文件名
        path->name = kmalloc(tmpnamelen + 1, 0);
        memset(path->name, 0, tmpnamelen + 1);
        memcpy(tmpname, path->name, tmpnamelen);
        path->name_length = tmpnamelen;

        // lookup函数从当前目录中搜索与目标名想匹配的目录项。
        // 如果匹配成功，那么lookup函数将返回目标名的短目录项，失败返回NULL
        if (parent->dir_inode->inode_ops->lookup(parent->dir_inode, path) == NULL)
        { // 查找失败，释放申请的内存资源，返回
            color_printk(RED, WHITE, "can not find file or dir:%s\n", path->name);
            kfree(path->name);
            kfree(path);
            return NULL;
        }

        // child_node 来记录我是谁的子文件, 加入到父目录的列表中
        // suddires_list 来记录我的子文件都有谁
        list_init(&path->child_node);
        list_init(&path->subdirs_list);
        path->parent = parent; // 在当前路径中，记录父目录
        list_add_to_behind(&parent->subdirs_list, &path->child_node);

        if (!*name) // 检测字符串是否结束
            goto last_component;
        while (*name == '/') // 递增name到下一个路径名称
            name++;
        if (!*name)
            goto last_slash;
        parent = path;
    }

last_component: // 最后的组成成分
last_slash:     // 最后的斜杠

    // 当形参flags = 1时, path_walk函数返回目标父目录的目录项，否则返回目标目录项
    if (flags & 1)
        return parent;

    return path;
}
