#ifndef __INODE_H__
#define __INODE_H__

// 记录文件在文件系统中的物理信息和文件在操作系统中的抽象信息
typedef struct index_node
{

    list_t i_sb_list;        // 超级块链表_结点
    
    mode_t i_mode;           // 文件模式
    size_t file_size;        // 文件大小
    u64_t blocks;    // 本文件占用了几个512B数据块 ？
    u64_t attribute; // 用于保存目录项的属性, 有了i_mode之后，该属性应该被修改删除。

    struct buffer *buf; // inode 描述符对应 buffer
    
    dev_t dev;  // 设备号
    dev_t rdev; // 虚拟设备号

    idx_t nr;     // i 节点号
    size_t count; // 引用计数

    time_t atime; // 访问时间
    time_t mtime; // 修改时间
    time_t ctime; // 创建时间

    s32_t type;    // 文件系统类型

    s32_t uid; // 用户 id
    s32_t gid; // 组 id

    struct task_struct *rxwaiter; // 读等待进程
    struct task_struct *txwaiter; // 写等待进程

    struct super_block *sb; // 超级块指针

    file_operations_t*f_ops;           // 文件操作： 打开/关闭，读/写
    index_node_operations_t *inode_ops; // inode操作：创建
    // 用于保存各类文件系统的特有数据信息
    void *private_index_info; // 对于fat32系统，这里指向的是 struct FAT32_inode_info
}inode_t;


#endif