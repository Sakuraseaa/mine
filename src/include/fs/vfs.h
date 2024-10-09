// VFS是对所有文件系统的高度抽象和概括，操作系统将所有支持的文件系统都隐藏在VFS中
// VFS会提供统一的抽象函数接口供应用程序访问
// VFS 总结归纳出访问文件系统的操作方法，每种文件系统都拥有一份操作方法的执行副本，
// 它们会根据自身情况和特点对这些操作方法予以实现
#ifndef __VFS_H__
#define __VFS_H__

extern list_t super_list;
extern struct super_block *current_sb;

struct super_block_operations;
struct index_node_operations;
struct dir_entry_operations;
struct file_operations;
struct index_node;
struct dir_entry;

typedef struct ide_part_t
{
    char name[8];            // 分区名称
    struct ide_disk_t *disk; // 磁盘指针
    u32_t system;              // 分区类型
    u32_t start;               // 分区起始物理扇区号 LBA
    u32_t count;               // 分区占用的扇区数
} ide_part_t;

// 是进程和VFS的纽带，它是抽象出来的，不存在于物质介质中
// 文件的读写访问(同步/异步)，IO控制以及其他操作方法
// 文件描述符
typedef struct file
{
    long position;      // 本文件的当前访问位置
    u64_t mode; // mode 保存着文件的访问模式和操作模式

    struct dir_entry *dentry; // 本文件对应的目录项

    struct file_operations *f_ops; 

    // 用于保存各类文件系统的特有数据信息-具体到每个文件
    void *private_data;
}file_t;

struct super_block_operations
{
    void (*write_superblock)(struct super_block *sb); // 把内存中的超级快和磁盘的超级快进行同步
    void (*put_superblock)(struct super_block *sb);   // 释放超级快
    void (*write_inode)(struct index_node *inode); // 把一个索引节点写入磁盘
};

struct index_node_operations
{
    long (*create)(struct index_node *inode, struct dir_entry *dentry, int mode);                                                           // 为dentry对象建立一个新的索引节点
    struct dir_entry *(*lookup)(struct index_node *parent_inode, struct dir_entry *dest_dentry);                                            // 在特定目录中寻找索引节点
    long (*mkdir)(struct index_node *inode, struct dir_entry *dentry, int mode);                                                            // 创建一个新目录
    long (*rmdir)(struct index_node *inode, struct dir_entry *dentry);                                                                      // 删除inode目录中的dentry目录项代表的文件
    long (*rename)(struct index_node *old_inode, struct dir_entry *old_dentry, struct index_node *new_inode, struct dir_entry *new_dentry); // 移动文件
    long (*getattr)(struct dir_entry *dentry, u64_t *attr);                                                                         // 在通知所有节点需要对磁盘中更新时，VFS会调用该函数
    long (*setattr)(struct dir_entry *dentry, u64_t *attr);                                                                         // 在修改索引节点后，通知发生了”改变事件“
    long (*unlink)(struct index_node *dir, struct dir_entry* dentry); // 删除文件
};

struct dir_entry_operations
{
    long (*compare)(struct dir_entry *parent_dentry, char *source_filename, char *destination_filename); // 比较两个文件名称
    long (*hash)(struct dir_entry *dentry, char *filename);                                              // 该函数为目录项生成散列值
    long (*release)(struct dir_entry *dentry);                                                           // 释放目录项对象
    long (*iput)(struct dir_entry *dentry, struct index_node *inode);                                    // 释放inode索引
    long (*d_delete)(struct dir_entry *dentry);
};

// 文件描述符
typedef int (*filldir_t)(void *buf,char *name, long namelen,long offset);
struct file_operations
{
    long (*open)(struct index_node *inode, struct file *filp);
    long (*close)(struct index_node *inode, struct file *filp);
    long (*read)(struct file *filp, char *buf, u64_t count, long *position);
    long (*write)(struct file *filp, char *buf, u64_t count, long *position);
    long (*lseek)(struct file *filp, long offset, long origin);
    long (*ioctl)(struct index_node *inode, struct file *filp, u64_t cmd, u64_t arg);
    long (*readdir)(struct file* filp, void* dirent, filldir_t filler);
};



#endif