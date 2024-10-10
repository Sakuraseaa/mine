// VFS是对所有文件系统的高度抽象和概括，操作系统将所有支持的文件系统都隐藏在VFS中
// VFS会提供统一的抽象函数接口供应用程序访问
// VFS 总结归纳出访问文件系统的操作方法，每种文件系统都拥有一份操作方法的执行副本，
// 它们会根据自身情况和特点对这些操作方法予以实现
#ifndef __VFS_H__
#define __VFS_H__

extern list_t super_list;
extern struct super_block *current_sb;

typedef struct super_block_operations super_block_operations_t;
typedef struct index_node_operations index_node_operations_t;
typedef struct dir_entry_operations dir_entry_operations_t;
typedef struct file_operations file_operations_t;
typedef struct index_node inode_t;
typedef struct dir_entry dir_entry_t;

typedef struct ide_part_t
{
    char_t name[8];            // 分区名称
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
    s64_t position;      // 本文件的当前访问位置
    u64_t mode; // mode 保存着文件的访问模式和操作模式

    dir_entry_t *dentry; // 本文件对应的目录项

    file_operations_t*f_ops; 

    // 用于保存各类文件系统的特有数据信息-具体到每个文件
    void *private_data;
}file_t;

typedef struct super_block_operations
{
    void (*write_superblock)(struct super_block *sb); // 把内存中的超级快和磁盘的超级快进行同步
    void (*put_superblock)(struct super_block *sb);   // 释放超级快
    void (*write_inode)(inode_t *inode); // 把一个索引节点写入磁盘
}super_block_operations_t;

struct index_node_operations
{
    s64_t (*create)(inode_t *inode, dir_entry_t *dentry, s32_t mode);                                                           // 为dentry对象建立一个新的索引节点
    dir_entry_t *(*lookup)(inode_t *parent_inode, dir_entry_t *dest_dentry);                                            // 在特定目录中寻找索引节点
    s64_t (*mkdir)(inode_t *inode, dir_entry_t *dentry, s32_t mode);                                                            // 创建一个新目录
    s64_t (*rmdir)(inode_t *inode, dir_entry_t *dentry);                                                                      // 删除inode目录中的dentry目录项代表的文件
    s64_t (*rename)(inode_t *old_inode, dir_entry_t *old_dentry, inode_t *new_inode, dir_entry_t *new_dentry); // 移动文件
    s64_t (*getattr)(dir_entry_t *dentry, u64_t *attr);                                                                         // 在通知所有节点需要对磁盘中更新时，VFS会调用该函数
    s64_t (*setattr)(dir_entry_t *dentry, u64_t *attr);                                                                         // 在修改索引节点后，通知发生了”改变事件“
    s64_t (*unlink)(inode_t *dir, dir_entry_t* dentry); // 删除文件
};

struct dir_entry_operations
{
    s64_t (*compare)(dir_entry_t *parent_dentry, char *source_filename, char *destination_filename); // 比较两个文件名称
    s64_t (*hash)(dir_entry_t *dentry, char *filename);                                              // 该函数为目录项生成散列值
    s64_t (*release)(dir_entry_t *dentry);                                                           // 释放目录项对象
    s64_t (*iput)(dir_entry_t *dentry, inode_t *inode);                                    // 释放inode索引
    s64_t (*d_delete)(dir_entry_t *dentry);
};

// 文件描述符
typedef s32_t (*filldir_t)(void *buf,char *name, s64_t namelen,s64_t offset);
struct file_operations
{
    s64_t (*open)(inode_t *inode, file_t *filp);
    s64_t (*close)(inode_t *inode, file_t *filp);
    s64_t (*read)(file_t *filp, char *buf, u64_t count, s64_t *position);
    s64_t (*write)(file_t *filp, char *buf, u64_t count, s64_t *position);
    s64_t (*lseek)(file_t *filp, s64_t offset, s64_t origin);
    s64_t (*ioctl)(inode_t *inode, file_t *filp, u64_t cmd, u64_t arg);
    s64_t (*readdir)(file_t* filp, void* dirent, filldir_t filler);
};

#endif