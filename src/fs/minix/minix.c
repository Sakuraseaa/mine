#include "lib.h"
#include "VFS.h"
#include "errno.h"
#include "memory.h"
#include "stdio.h"
#include "debug.h"
#include "dirent.h"
#include "disk.h"
#include "minix.h"
#include "printk.h"

struct index_node_operations MINIX_inode_ops;
struct file_operations MINIX_file_ops;
struct dir_entry_operations MINIX_dentry_ops;
struct super_block_operations MINIX_sb_ops;



/**
 * @brief 为fat32文件系统编写的引导扇区解析方�?, 主�?�建立并初�?�化超级�?
 *
 * @param DPTE MBR的分区表
 * @param buf fat32文件系统的引导扇�?
 * @return struct super_block* 超级块结构体
 */
struct super_block *minix_read_superblock(struct Disk_Partition_Table_Entry *DPTE, void *buf)
{
    struct super_block *sbp = NULL;
    minix_sb_info_t* minix_sb;
    unsigned char* bbuf = (unsigned char*)kmalloc(512, 0);
    // ===============================  读取super block =====================================
    sbp = (struct super_block *)kmalloc(sizeof(struct super_block), 0);

    sbp->block_size = BLOCK_SIZE;
    sbp->sector_size = SECTOR_SIZE;
    sbp->type = FS_TYPE_MINIX;
    memset(sbp, 0, sizeof(struct super_block));

    sbp->sb_ops = &MINIX_sb_ops;
    sbp->private_sb_info = minix_sb = (minix_sb_info_t *)kmalloc(sizeof(minix_sb_info_t), 0);
    memset(sbp->private_sb_info, 0, sizeof(minix_sb_info_t));

    IDE_device_operation.transfer(ATA_READ_CMD, DPTE->start_LBA + 2, 1, (unsigned char *)bbuf);
    
    memcpy(bbuf,sbp->private_sb_info, sizeof(minix_sb_info_t));
    // ================================== 读取根目�? =====================================
    color_printk(ORANGE, BLACK, "MINIX FSinfo\n Firstdatalba:%#08lx\tinode_count:%#08lx\tlog_zone_size:%#08lx\n \
inode_map_size:%08lx\t zone_map_size:%08lx\t minix_magic:%08lx\n",
                minix_sb->firstdatazone,  minix_sb->inodes, minix_sb->log_zone_size, minix_sb->imap_blocks,
                minix_sb->zmap_blocks, minix_sb->magic);
    
    kfree(bbuf);
    return sbp;
}


struct file_system_type MINIX_fs_type =
    {
        .name = "MINIX",
        .fs_flags = 0,
        .read_superblock = minix_read_superblock,
        .next = NULL,
};

