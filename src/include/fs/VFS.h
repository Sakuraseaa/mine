// VFS是对所有文件系统的高度抽象和概括，操作系统将所有支持的文件系统都隐藏在VFS中
// VFS会提供统一的抽象函数接口供应用程序访问
// VFS 总结归纳出访问文件系统的操作方法，每种文件系统都拥有一份操作方法的执行副本，
// 它们会根据自身情况和特点对这些操作方法予以实现
#ifndef __VFS_H__
#define __VFS_H__

#include "lib.h"
#include "fat32.h"
#include "types.h"
#include "buffer.h"

extern list_t super_list;
extern struct super_block *current_sb;

// 硬盘分区表项
struct Disk_Partition_Table_Entry
{
    unsigned char flags;             // 0x80 = 活动分区标记(表明此分区的引导扇区中包含引导程序, 可引导),0 = 非活动分区
    unsigned char start_head;        // 分区起始磁头号
    unsigned short start_sector : 6, // 0 ~ 5 - 分区起始扇区号
        start_cylinder : 10;         // 6 ~ 15 - 分区起始柱面号
    unsigned char type;              // 文件类型ID, 0b表示FAT32
    unsigned char end_head;          // 分区结束磁头号
    unsigned short end_sector : 6,   // 0 ~ 5 - 分区结束扇区号
        end_cylinder : 10;           // 6 ~ 15 - 分区起始柱面号
    unsigned int start_LBA;          // 分区起始偏移扇区
    unsigned int sectors_limit;      // 分区扇区数
} __attribute__((packed));

// MBR
struct Disk_Partition_Table
{
    unsigned char BS_reserved[446];
    struct Disk_Partition_Table_Entry DPTE[4];
    unsigned short BS_TRailSig;
} __attribute__((packed));

// 记录VFS 支持的文件系统类型
struct file_system_type
{
    char *name;
    int fs_flags;
    // read_superblock 存有解析文件系统引导扇区的方法
    struct super_block *(*read_superblock)(struct Disk_Partition_Table_Entry *DPTE, void *buf);
    // 当挂载文件系统时，操作系统只需沿着文件系统链表搜索文件系统名
    // 一旦匹配成功就调用read_superblock方法，为文件系统创建超级块结构
    struct file_system_type *next;
};

struct super_block_operations;
struct index_node_operations;
struct dir_entry_operations;
struct file_operations;
struct index_node;

typedef struct ide_part_t
{
    char name[8];            // 分区名称
    struct ide_disk_t *disk; // 磁盘指针
    u32 system;              // 分区类型
    u32 start;               // 分区起始物理扇区号 LBA
    u32 count;               // 分区占用的扇区数
} ide_part_t;

// 记录着目标文件系统的引导扇区信息-操作系统为文件系统分配的资源信息
typedef struct super_block
{
    list_t node;          /* 所有超级块的链表_节点*/
    // 记录着根目录的目录项，此目录项在文件系统中并不存在实体结构， 是为了便于搜索特意抽象出来的
    struct dir_entry *root;

    buffer_t *buf; // 超级块描述符 buffer
    dev_t dev;            // 设备号
    u32 count;            // 引用计数
    int type;             // 文件系统类型
    size_t sector_size;   // 扇区大小
    size_t block_size;    // 块大小
    list_t inode_list;    // 使用中 inode 链表
    // struct index_node *imount;      // 安装到的 inode

    // 包含操作： superblock结构的读写, inode的写
    struct super_block_operations *sb_ops;

    // 用于保存各类文件系统的特有数据信息
    void *private_sb_info; // 对于fat32文件系统来说，该指针连接的是 FAT32_sb_info 结构体
}super_t;

// 记录文件在文件系统中的物理信息和文件在操作系统中的抽象信息
typedef struct index_node
{

    list_t i_sb_list;        // 超级块链表
    
    mode_t i_mode;           // 文件模式
    size_t file_size;        // 文件大小
    unsigned long blocks;    // 本文件占用了几个512B数据块 ？
    unsigned long attribute; // 用于保存目录项的属性, 有了i_mode之后，该属性应该被修改删除。

    buffer_t *buf; // inode 描述符对应 buffer
    
    dev_t dev;  // 设备号
    dev_t rdev; // 虚拟设备号

    idx_t nr;     // i 节点号
    size_t count; // 引用计数

    time_t atime; // 访问时间
    time_t mtime; // 修改时间
    time_t ctime; // 创建时间

    int type;    // 文件系统类型

    int uid; // 用户 id
    int gid; // 组 id

    struct task_struct *rxwaiter; // 读等待进程
    struct task_struct *txwaiter; // 写等待进程

    struct super_block *sb; // 超级块指针

    struct file_operations *f_ops;           // 文件操作： 打开/关闭，读/写
    struct index_node_operations *inode_ops; // inode操作：创建
    // 用于保存各类文件系统的特有数据信息
    void *private_index_info; // 对于fat32系统，这里指向的是 struct FAT32_inode_info
}inode_t;

#define FS_ATTR_FILE (1UL << 0)            // 文件
#define FS_ATTR_DIR (1UL << 1)             // 目录
#define FS_ATTR_DEVICE_KEYBOARD (1UL << 2) // 设备文件 - 键盘

// 用于描述文件/目录在文件系统中的层级关系-目录项
typedef struct dir_entry
{
    char *name;      // 文件名
    int name_length; // 文件长度

    // 描述目录项之间的层级关系
    struct List child_node;
    struct List subdirs_list; // subdirectory - 子目录

    struct index_node *dir_inode; // 本目录项描述的文件的inode
    struct dir_entry *parent;     // 父目录项

    struct dir_entry_operations *dir_ops; // 目录项操作方法：
}dir_entry_t;
typedef int (*filldir_t)(void *buf,char *name, long namelen,long offset);


// 是进程和VFS的纽带，它是抽象出来的，不存在于物质介质中
// 文件的读写访问(同步/异步)，IO控制以及其他操作方法
// 文件描述符
struct file
{
    long position;      // 本文件的当前访问位置
    unsigned long mode; // mode 保存着文件的访问模式和操作模式

    struct dir_entry *dentry; // 本文件对应的目录项

    struct file_operations *f_ops;

    // 用于保存各类文件系统的特有数据信息-具体到每个文件
    void *private_data;
};

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
    long (*getattr)(struct dir_entry *dentry, unsigned long *attr);                                                                         // 在通知所有节点需要对磁盘中更新时，VFS会调用该函数
    long (*setattr)(struct dir_entry *dentry, unsigned long *attr);                                                                         // 在修改索引节点后，通知发生了”改变事件“
};

struct dir_entry_operations
{
    long (*compare)(struct dir_entry *parent_dentry, char *source_filename, char *destination_filename); // 比较两个文件名称
    long (*hash)(struct dir_entry *dentry, char *filename);                                              // 该函数为目录项生成散列值
    long (*release)(struct dir_entry *dentry);                                                           // 释放目录项对象
    long (*iput)(struct dir_entry *dentry, struct index_node *inode);                                    // 释放inode索引
};

// 文件描述符
struct file_operations
{
    long (*open)(struct index_node *inode, struct file *filp);
    long (*close)(struct index_node *inode, struct file *filp);
    long (*read)(struct file *filp, char *buf, unsigned long count, long *position);
    long (*write)(struct file *filp, char *buf, unsigned long count, long *position);
    long (*lseek)(struct file *filp, long offset, long origin);
    long (*ioctl)(struct index_node *inode, struct file *filp, unsigned long cmd, unsigned long arg);
    long (*readdir)(struct file* filp, void* dirent, filldir_t filler);
};

struct super_block *mount_fs(char *name, struct Disk_Partition_Table_Entry *DPTE, void *buf);
unsigned long register_filesystem(struct file_system_type *fs);
unsigned long unregister_filesystem(struct file_system_type *fs);
struct dir_entry *path_walk(char *name, unsigned long flags, struct dir_entry **create_file);

void DISK1_FAT32_FS_init();
#endif