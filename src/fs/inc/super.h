#ifndef __SUPER_H__
#define __SUPER_H__

typedef struct dir_entry dir_entry_t;
// 记录着目标文件系统的引导扇区信息-操作系统为文件系统分配的资源信息
typedef struct super_block
{
    list_t node;          /* 所有超级块的链表_节点*/
    // 记录着根目录的目录项，此目录项在文件系统中并不存在实体结构， 是为了便于搜索特意抽象出来的
    dir_entry_t *root; // directory mount point

    struct buffer *buf; // 超级块描述符 buffer
    dev_t dev;            // 设备号
    u32_t count;            // 引用计数
    s32_t type;             // 文件系统类型:
    size_t sector_size;   // 扇区大小
    size_t block_size;    // 块大小
    list_t inode_list;    // 使用中 inode 链表
    // inode_t *imount;      // 安装到的 inode

    // 包含操作： superblock结构的读写, inode的写
    super_block_operations_t *sb_ops;

    u64_t s_flags; // mount mark
    // 用于保存各类文件系统的特有数据信息
    void *private_sb_info; // 对于fat32文件系统来说，该指针连接的是 FAT32_sb_info 结构体
}spblk_t;

#endif