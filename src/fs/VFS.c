#include "VFS.h"
#include "fat32.h"
#include "printk.h"
#include "memory.h"
struct file_system_type filesystem = {"filesystem", 0};
#define MAX_FILE_NAME_LEN PAGE_4K_SIZE
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
 * @brief path_parse用于获得文件路径pathname的顶层路径, 顶层路径存入到name_store中
 *
 * @example pathname="/home/sk", char name_store[10]
 *          path_parse(pathname, name_store) -> return "/sk", *name_store="home"
 *
 * @param pathname 需要解析的文件路径
 * @param name_store 主调函数提供的缓冲区
 * @return char* 指向除顶层路径之外的子路径字符串的地址
 */
static char *path_parse(char *pathname, char *name_store)
{
    // 根目录不需要解析, 跳过即可
    if (pathname[0] == '/')
        while (*(++pathname) == '/')
            ; // 跳过'//a', '///b'

    while (*pathname != '/' && *pathname != 0)
        *name_store++ = *pathname++;

    if (pathname[0] == 0) // pathname为空, 则表示路径已经结束了, 此时返回NULL
        return NULL;

    return pathname;
}

/**
 * @brief path_depth_cnt用于返回路径pathname的深度. 注意, 所谓路径的深度是指到文件的深度, 例如: /a的深度是1, /a/b的深度
 *        是2, /a/b/c/d/e的深度是5
 *
 * @param pathname 需要判断深度的路径名
 * @return uint32_t 路径的深度
 */
static int path_depth_cnt(char *pathname)
{
    char *p = pathname;
    char name[MAX_FILE_NAME_LEN];
    unsigned int depth = 0;
    p = path_parse(p, name);
    while (name[0])
    {
        depth++;
        memset(name, 0, MAX_FILE_NAME_LEN);
        if (p)
            p = path_parse(p, name);
    }
    return depth;
}

/**
 * @brief 搜索文件name。
 *
 * @param name 文件名称
 * @param flags 当形参flags = 1时, path_walk函数返回目标父目录的目录项，否则返回目标目录项
 * @param create_file 只有在sys_open中创建文件的时候，该参数才有效。这是传出参数。其中记录新文件的目录项信息
 * @return struct dir_entry* 搜索失败返回NULL, dir_entry和dentry动态申请的内存，由上层调用者释放
 */

struct dir_entry *path_walk(char *name, unsigned long flags, struct dir_entry **create_file)
{
    char *tmpname = NULL;
    int tmpnamelen = 0, nameDep = 0, Count = 0;
    struct dir_entry *parent = root_sb->root;
    struct dir_entry *path = NULL;

    // 越过路径前的 '/'
    while (*name == '/')
        name++;
    // 路径若为空，则返回根目录
    if (!*name)
        return parent;

    nameDep = path_depth_cnt(name); // 计算文件名深度

    // 此处为路径上的一系列文件，都创建了dir_entry结构体
    for (;;)
    {
        Count++;
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
        // 注意此处的Path是一个传出参数，如果在parent中，寻找成功。
        // 那么path中记录目标文件的index_node, 通过inode可以获得该文件的所有信息
        if (parent->dir_inode->inode_ops->lookup(parent->dir_inode, path) == NULL)
        { // 查找失败，释放申请的内存资源，返回
            if ((flags & 1) && (Count == nameDep))
            {
                // 如果是创建文件，即时是查找失败了，也继续进行循环
                // 但如果此处是中间路径错误了怎么办？这里的逻辑重写的 日后
                goto continue_for;
            }

            color_printk(RED, WHITE, "can not find file or dir:%s\n", path->name);
            kfree(path->name);
            kfree(path);

            return NULL;
        }
    continue_for:
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
    {
        if (path->dir_inode)
        {
            color_printk(BLACK, RED, "File already exit!!!");
            return NULL;
        }
        *create_file = path;
        return parent;
    }

    return path;
}
