#ifndef __FS_H__
#define __FS_H__

enum
{
    FS_TYPE_NONE = 0,
    FS_TYPE_PIPE,
    FS_TYPE_SOCKET,
    FS_TYPE_MINIX,
    FS_TYPE_FAT32,
};
// 硬盘分区表项
struct Disk_Partition_Table_Entry
{
    u8_t flags;             // 0x80 = 活动分区标记(表明此分区的引导扇区中包含引导程序, 可引导),0 = 非活动分区
    u8_t start_head;        // 分区起始磁头号
    u16_t start_sector : 6, // 0 ~ 5 - 分区起始扇区号
        start_cylinder : 10;         // 6 ~ 15 - 分区起始柱面号
    u8_t type;              // 文件类型ID, 0b表示FAT32
    u8_t end_head;          // 分区结束磁头号
    u16_t end_sector : 6,   // 0 ~ 5 - 分区结束扇区号
        end_cylinder : 10;           // 6 ~ 15 - 分区起始柱面号
    u32_t start_LBA;          // 分区起始偏移扇区
    u32_t sectors_limit;      // 分区扇区数
} __attribute__((packed));

// MBR
struct Disk_Partition_Table
{
    u8_t BS_reserved[446];
    struct Disk_Partition_Table_Entry DPTE[4];
    u16_t BS_TRailSig;
} __attribute__((packed));

// 记录VFS 支持的文件系统类型
struct file_system_type
{
    str_t name;
    s32_t fs_flags;
    // read_superblock 存有解析文件系统引导扇区的方法
    struct super_block *(*read_superblock)(struct Disk_Partition_Table_Entry *DPTE, void *buf);
    // 当挂载文件系统时，操作系统只需沿着文件系统链表搜索文件系统名
    // 一旦匹配成功就调用read_superblock方法，为文件系统创建超级块结构
    struct file_system_type *next;
};

#endif