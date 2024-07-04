#include "fs.h"
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
#include "time.h"
#include "inode.h"
#include "bitmap.h"
#include "super.h"
struct super_block_operations minix_super_ops;

// 计算 inode nr 对应的块号
static inline idx_t inode_block(minix_sb_info_t *desc, idx_t nr)
{
    // inode 编号 从 1 开始
    return 2 + desc->imap_blocks + desc->zmap_blocks + (nr - 1) / BLOCK_INODES;
}

// 分配一个物理块
static idx_t minix_balloc(super_t *super) {
    
    buffer_t *buf = NULL;
    idx_t bit = 0;
    bitmap_t map;
    
    minix_sb_info_t *m_sb = (minix_sb_info_t *)super.private_sb_info;
    idx_t bidx = 2 + m_sb->imap_blocks; // 块位图的块号,启动块 超级块 inode位图块... 

    for (size_t i = 0; i < m_sb->zmap_blocks; i++) {
        buf = bread(super->dev, bidx + i, BLOCK_SIZE);

        // 将整个缓冲区作为位图
        bitmap_make(&map, buf->data, BLOCK_SIZE);

        // 从位图中扫描一位
        bit = bitmap_scan(&map, 1);
        if (bit != -1) {
            // 如果扫描成功，则 标记缓冲区脏，中止查找
            bitmap_set(&map, bit, true);
            buf->dirty = true;
            break;
        }
    }
    brelse(buf); // todo 调试期间强同步
    return bit;
}

// 回收一个物理块
static void minix_bfree(super_t* super, idx_t bit) {
    
    buffer_t *buf = NULL;
    bitmap_t map;
    minix_sb_info_t *m_sb = (minix_sb_info_t *)super.private_sb_info;
    idx_t bidx = 2 + m_sb->imap_blocks; // 块位图的块号

    for (size_t i = 0; i < m_sb->zmap_blocks; i++) {
        
        if(bit > BLOCK_BITS * (i + 1)) // 跳过开始块
            continue;

        buf = bread(super->dev, bidx + i, BLOCK_SIZE);

        // 将整个缓冲区作为位图
        bitmap_make(&map, buf->data, BLOCK_SIZE);
        
        // 必须置位
        assert(bitmap_scan_test(&map, bit) == 1);
        
        bitmap_set(&map, bit, 0);

        // 标记缓冲区脏
        buf->dirty = true;
        break;
    }

    brelse(buf); // todo 调试期间强同步
}

// 获取 inode 第 block 块的索引值
// 如果不存在 且 create 为 true，则创建
u16 minix_bmap(inode_t *inode, idx_t block, bool create) {
    assert(block >= 0 && block < TOTAL_BLOCK);
    u64 index = block;
    minix_inode_t* m_inode = (minix_dentry_t*)inode->private_index_info;
    u16 *array = m_inode->zone;
    buffer_t* buf = NULL;

    // 当前处理级别
    int level = 0;

    // 当前子级别块数量
    int divider = 1;

    // 直接块
    if (block < DIRECT_BLOCK)
        goto reckon;

    block -= DIRECT_BLOCK;

    // 一级间接块
    if (block < INDIRECT1_BLOCK)
    {
        index = DIRECT_BLOCK;
        level = 1;
        divider = 1;
        goto reckon;
    }

    block -= INDIRECT1_BLOCK;
    assert(block < INDIRECT1_BLOCK);
    index = DIRECT_BLOCK + 1;
    level = 2;
    divider = BLOCK_INDEXES;

reckon:
    for (; level >= 0; level--)
    {
        // 如果不存在 且 create 则申请一块文件块
        if ((array[index] == 0 )&& create) {
            
            array[index] = minix_balloc(inode->super);
            buf->dirty = true;
        }

        brelse(buf);

        // 如果 level == 0 或者 索引不存在，直接返回
        if (level == 0 || !array[index])
            return array[index];

        // level 不为 0，处理下一级索引
        buf = bread(inode->dev, array[index], BLOCK_SIZE);
        index = block / divider;
        block = block % divider;
        divider /= BLOCK_INDEXES;
        array = (u16 *)buf->data;
    }

    return -1;
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
struct dir_entry *minix_lookup(struct index_node *parent_inode, struct dir_entry *dest_dentry) {
    
    minix_inode_t* m_inode = (minix_inode_t*)parent_inode->private_index_info;
    u64 dentry_count = parent_inode->file_size / sizeof(minix_dentry_t);
    for() {

    }
}
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



// 获得设备 dev 的 nr inode
static inode_t *iget(dev_t dev, idx_t nr)
{
    inode_t *inode = find_inode(dev, nr);
    if (inode)
    {
        inode->count++;
        inode->atime = NOW();
        return inode;
    }
    super_t* super = get_super(dev);
    minix_sb_info_t *minix_sb = (minix_sb_info_t*)(super->private_sb_info);
    
    inode = (inode_t*)kmalloc(sizeof(inode_t), 0);
    inode->attribute = FS_ATTR_DIR; // 这个变量有点糟糕
    inode->dev = dev;
    inode->inode_ops = &minix_inode_ops;
    inode->f_ops = &minix_file_ops;
    inode->count = 1; // 该变量应该是 一个原子变量
    inode->nr = nr;
    list_init(&inode->i_sb_list);
    list_add_to_behind(&super->inode_list, &inode->i_sb_list);
    inode->sb = super;

    inode->buf = bread(dev, inode_block(minix_sb, nr), BLOCK_SIZE);
    minix_inode_t *mit = &((minix_inode_t*)inode->buf->data)[(inode->nr - 1) % BLOCK_INODES];
    
    inode->ctime = inode->mtime = inode->atime = mit->mtime;
    inode->i_mode = mit->mode;
    inode->file_size = mit->size;
    inode->gid = mit->gid;
    inode->uid = mit->uid;
    inode->blocks = (inode->file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    inode->private_index_info = mit;
    return inode;
}



/**
 * @brief 为minix文件系统创建读超级块程序
 *          finished 2024-7-3 21:26 第一版
 * @param DPTE 分区表项
 * @param buf NULL
 * @return struct super_block* 超级块结构体
 */
struct super_block *minix_read_superblock(struct Disk_Partition_Table_Entry *DPTE, void *buf)
{
    struct super_block *sbp = NULL;
    minix_sb_info_t* minix_sb;
    unsigned char* bbuf = (unsigned char*)kmalloc(512, 0);
    bbuf[0] = 0xff;
    // ===============================  读取super block =====================================
    sbp = (struct super_block *)kmalloc(sizeof(struct super_block), 0);
    memset(sbp, 0, sizeof(struct super_block));

    sbp->dev = 2; // here need rewrite
    sbp->block_size = BLOCK_SIZE;
    sbp->sector_size = SECTOR_SIZE;
    sbp->type = FS_TYPE_MINIX;
    sbp->count = 1;
    list_init(&sbp->inode_list);
    list_init(&sbp->node);
    list_add_to_behind(&super_list, &sbp->node);
    sbp->sb_ops = &minix_super_ops;

    sbp->private_sb_info = minix_sb = (minix_sb_info_t *)kmalloc(sizeof(minix_sb_info_t), 0);
    memset(sbp->private_sb_info, 0, sizeof(minix_sb_info_t));

    sbp->buf = bread(sbp->dev, 1, sbp->block_size);
    memcpy(sbp->buf->data, sbp->private_sb_info, sizeof(minix_sb_info_t));
    
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
    sbp->root->dir_inode = iget(sbp->dev, 1);

    return sbp;
}


struct file_system_type MINIX_fs_type = {
        .name = "MINIX",
        .fs_flags = 0,
        .read_superblock = minix_read_superblock,
        .next = NULL,
};

