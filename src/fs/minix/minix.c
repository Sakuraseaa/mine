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
#include "buffer.h"
struct super_block_operations minix_super_ops;

// 计算 inode nr 对应的块号
static inline idx_t inode_block(minix_sb_info_t *desc, idx_t nr)
{
    // inode 编号 从 1 开始
    return 2 + desc->imap_blocks + desc->zmap_blocks + (nr - 1) / BLOCK_INODES;
}

//// these operation need cache and list - 为缓存目录项提供操作方法
long minix_compare(struct dir_entry *parent_dentry, char *source_filename, char *destination_filename) { return 0; }
long minix_hash(struct dir_entry *dentry, char *filename) { return 0; }
long minix_release(struct dir_entry *dentry) { return 0; }                        // 释放目录项
long minix_iput(struct dir_entry *dentry, struct index_node *inode) { return 0; } // 释放inode索引
struct dir_entry_operations minix_dentry_ops =
    {
        .compare = minix_compare,
        .hash = minix_hash,
        .release = minix_release,
        .iput = minix_iput,
};



// 负责为访问文件数据提供操作方法
long minix_open(struct index_node *inode, struct file *filp) { return 1; }
long minix_close(struct index_node *inode, struct file *filp) { return 1; }
long minix_read(struct file *filp, char *buf, unsigned long count, long *position) {}
long minix_write(struct file *filp, char *buf, unsigned long count, long *position) {}
// 设置文件指针的位置。这个函数的使用能否提升到VFS层面?
long minix_lseek(struct file *filp, long offset, long origin) {}
long minix_ioctl(struct index_node *inode, struct file *filp, unsigned long cmd, unsigned long arg) { return 0; }
long minix_readdir(struct file* filp, void * dirent, filldir_t filler) {}
struct file_operations minix_file_ops =
    {
        .open = minix_open,
        .close = minix_close,
        .read = minix_read,
        .write = minix_write,
        .lseek = minix_lseek,
        .ioctl = minix_ioctl,
        .readdir = minix_readdir,
};


long minix_create(struct index_node *inode, struct dir_entry *dentry, int mode) {}
struct dir_entry *minix_lookup(struct index_node *parent_inode, struct dir_entry *dest_dentry) {}
long minix_mkdir(struct index_node *inode, struct dir_entry *dentry, int mode) { return 0; }
long minix_rmdir(struct index_node *inode, struct dir_entry *dentry) { return 0;}
long minix_rename(struct index_node *old_inode, struct dir_entry *old_dentry, struct index_node *new_inode, struct dir_entry *new_dentry) { return 0; }
long minix_getattr(struct dir_entry *dentry, unsigned long *attr) { return 0;}
long minix_setattr(struct dir_entry *dentry, unsigned long *attr) { return 0;}

struct index_node_operations minix_inode_ops =
    {
        .create = minix_create,
        .lookup = minix_lookup,
        .mkdir = minix_mkdir,
        .rmdir = minix_rmdir,
        .rename = minix_rename,
        .getattr = minix_getattr,
        .setattr = minix_setattr,
};







/**
 * @brief 为minix文件系统创建读超级块程序
 *
 * @param DPTE 分区表项
 * @param buf NULL
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

    sbp->sb_ops = &minix_super_ops;
    sbp->private_sb_info = minix_sb = (minix_sb_info_t *)kmalloc(sizeof(minix_sb_info_t), 0);
    memset(sbp->private_sb_info, 0, sizeof(minix_sb_info_t));

    IDE_device_operation.transfer(ATA_READ_CMD, DPTE->start_LBA + 2, 1, (unsigned char *)bbuf);
    
    memcpy(bbuf,sbp->private_sb_info, sizeof(minix_sb_info_t));
    // ================================== 读取根目录 =====================================
    color_printk(ORANGE, BLACK, "MINIX FSinfo\n Firstdatalba:%#08lx\tinode_count:%#08lx\tlog_zone_size:%#08lx\n \
inode_map_size:%08lx\t zone_map_size:%08lx\t minix_magic:%08lx\n",
                minix_sb->firstdatazone,  minix_sb->inodes, minix_sb->log_zone_size, minix_sb->imap_blocks,
                minix_sb->zmap_blocks, minix_sb->magic);
    
    // directory entry 
    sbp->root = (dir_entry_t*)kmalloc(sizeof(dir_entry_t), 0);
    memset(sbp->root, 0, sizeof(dir_entry_t));

    list_init(&sbp->root->child_node);
    list_init(&sbp->root->subdirs_list);
    sbp->root->parent = sbp->root;
    sbp->root->name_length = 1;
    sbp->root->name = (char*)kmalloc(2, 0);
    sbp->root->name[0] = '/';
    sbp->root->dir_ops = &minix_dentry_ops;
    
    // creat root inode
    inode_t* root = (inode_t*)kmalloc(sizeof(inode_t), 0);
    root->buf = bread(2, minix_sb->firstdatazone , BLOCK_SIZE);
    root->attribute = FS_ATTR_DIR;
    root->inode_ops = &minix_inode_ops;
    root->f_ops = &minix_file_ops;
    root->file_size = 0;
    root->sb = sbp;
    root->blocks = (root->file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    root->count = 1;
    root->gid = current->pid;

    sbp->root->dir_inode = root;

    
    memset(bbuf, 0, sizeof(512));
    IDE_device_operation.transfer(ATA_READ_CMD, DPTE->start_LBA + inode_block(minix_sb, 1)*2, 1, (unsigned char *)bbuf);
    minix_inode_t* inode = (minix_inode_t*)bbuf;
    for(; inode->size != 0 ;){
        inode++;
    }
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

