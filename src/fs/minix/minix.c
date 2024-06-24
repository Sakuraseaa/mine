#include "lib.h"
#include "VFS.h"
#include "errno.h"
#include "memory.h"
#include "stdio.h"
#include "debug.h"
#include "dirent.h"
#include "disk.h"
#include "minix.h"

struct index_node_operations MINIX_inode_ops;
struct file_operations MINIX_file_ops;
struct dir_entry_operations MINIX_dentry_ops;
struct super_block_operations MINIX_sb_ops;



/**
 * @brief 为fat32文件系统编写的引导扇区解析方法, 主要建立并初始化超级块
 *
 * @param DPTE MBR的分区表
 * @param buf fat32文件系统的引导扇区
 * @return struct super_block* 超级块结构体
 */
struct super_block *minix_read_superblock(struct Disk_Partition_Table_Entry *DPTE, void *buf)
{
    struct super_block *sbp = NULL;
    minix_sb_info_t* sb;
    unsigned char bbuf[520] = {0};
    // ===============================  读取super block =====================================
    sbp = (struct super_block *)kmalloc(sizeof(struct super_block), 0);
    memset(sbp, 0, sizeof(struct super_block));

    sbp->sb_ops = &MINIX_sb_ops;
    sbp->private_sb_info = sb = (minix_sb_info_t *)kmalloc(sizeof(minix_sb_info_t), 0);
    memset(sbp->private_sb_info, 0, sizeof(minix_sb_info_t));


    IDE_device_operation.transfer(ATA_READ_CMD, DPTE->start_LBA + 2, 1, (unsigned char *)bbuf);
    
    memcpy(bbuf,sbp->private_sb_info, sizeof(minix_sb_info_t));
    // ================================== 读取根目录 =====================================

    return sbp;
}


struct file_system_type MINIX_fs_type =
    {
        .name = "MINIX",
        .fs_flags = 0,
        .read_superblock = minix_read_superblock,
        .next = NULL,
};

