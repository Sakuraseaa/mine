#include "fs.h"
#include "lib.h"
#include "types.h"
#include "VFS.h"
#include "errno.h"
#include "memory.h"
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
#include "stat.h"
#include "assert.h"

static buffer_t* imap = NULL;
static buffer_t* zmap[2] = {NULL};

inode_t *iget(dev_t dev, idx_t nr);
struct super_block_operations minix_super_ops;

// 计算 inode nr 对应的块号
static inline idx_t inode_block(minix_sb_info_t *desc, idx_t nr)
{
    // inode 编号 从 1 开始
    return 2 + desc->imap_blocks + desc->zmap_blocks + (nr - 1) / BLOCK_INODES;
}

// 分配一个硬盘上的 inode
static idx_t minix_ialloc(super_t *super) {
    
    buffer_t *buf = NULL;
    idx_t bit = 0;
    bitmap_t map;
    
    minix_sb_info_t *m_sb = (minix_sb_info_t *)super->private_sb_info;
    idx_t bidx = 2; // 块位图的块号,启动块 超级块

    for (size_t i = 0; i < m_sb->imap_blocks; i++) {
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

// 释放一个inode节点
static void minix_ifree(super_t* super, idx_t bit) {
    buffer_t *buf = NULL;
    bitmap_t map;
    minix_sb_info_t *m_sb = (minix_sb_info_t *)super->private_sb_info;
    idx_t bidx = 2; // 块位图的块号

    for (size_t i = 0; i < m_sb->imap_blocks; i++) {
        
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

// 分配一个物理块
static idx_t minix_balloc(super_t *super) {
    
    buffer_t *buf = NULL;
    idx_t bit = 0;
    bitmap_t map;
    
    minix_sb_info_t *m_sb = (minix_sb_info_t *)super->private_sb_info;
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
    return m_sb->firstdatazone + bit;
}

// 回收一个物理块
static void minix_bfree(super_t* super,u16 block) {
    
    buffer_t *buf = NULL;
    bitmap_t map;
    minix_sb_info_t *m_sb = (minix_sb_info_t *)super->private_sb_info;
    idx_t bidx = 2 + m_sb->imap_blocks; // 块位图的块号
    block -= m_sb->firstdatazone;

    for (size_t i = 0; i < m_sb->zmap_blocks; i++) {
        
        if(block > BLOCK_BITS * (i + 1)) // 跳过开始块
            continue;

        buf = bread(super->dev, bidx + i, BLOCK_SIZE);

        // 将整个缓冲区作为位图
        bitmap_make(&map, buf->data, BLOCK_SIZE);
        
        // 必须置位
        assert(bitmap_scan_test(&map, block) == 1);
        
        bitmap_set(&map, block, 0);

        // 标记缓冲区脏
        buf->dirty = true;
        break;
    }

    brelse(buf); // todo 调试期间强同步
}

// 得到一个 新 inode实体
static inode_t *minix_new_node(dev_t dev, idx_t nr, int32 file_type) {

    u16 umask = current->umask;
    int32 mode = 0;
    inode_t* inode = iget(dev, nr);
    minix_inode_t* m_inode = (minix_inode_t*)inode->private_index_info;
    
    memset(m_inode, 0, sizeof(minix_inode_t));
    inode->i_mode = inode->blocks = inode->file_size = 0;
    inode->rdev = inode->type = 0;

    m_inode->gid = inode->gid = current->gid;
    m_inode->uid = inode->uid = current->uid;
    m_inode->mtime = inode->atime = inode->ctime = inode->mtime = NOW();
    m_inode->nlinks = 0; // 再为其创建目录项的时候 加1
    
    if(file_type == IFDIR)
        inode->attribute = FS_ATTR_DIR;
    else if(file_type == IFREG)
        inode->attribute = FS_ATTR_FILE;
    
    // 设置文件类型和权限
    mode |= (0777 & ~umask);
    mode |= file_type;
    m_inode->mode = inode->i_mode = mode;

    // 同步本次inode
    inode->buf->dirty = true;
    bwrite(inode->buf);
    return inode;
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
            
            array[index] = minix_balloc(inode->sb);
            if(buf)
                buf->dirty = true;
        }

        brelse(buf);

        // 如果 level == 0 或者 索引不存在，直接返回
        if( level == 0 || array[index] == 0)
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
long minix_delete(struct dir_entry *dentry) { 
    // 当然这里是需要重写的
    if(ISREG(dentry->dir_inode->i_mode)) {
        kfree(dentry->dir_inode);

        list_del(&dentry->child_node);

        slab_free(Dir_Entry_Pool, dentry, 0); // 这一句话应该再 VFS 层实现
    } else if (ISDIR(dentry->dir_inode->i_mode)) {

    }
    return 0; 

} 
struct dir_entry_operations minix_dentry_ops =
    {
        .compare = minix_compare,
        .hash = minix_hash,
        .release = minix_release,
        .iput = minix_iput,
        .d_delete = minix_delete,
};



// 负责为访问文件数据提供操作方法
long minix_open(struct index_node *inode, struct file *filp) { return 1; }
long minix_close(struct index_node *inode, struct file *filp) { return 1; }

long minix_read(struct file *filp, char *buf, u64 count, int64 *position) {
    
    inode_t* inode = filp->dentry->dir_inode;

    int64 index = *position / BLOCK_SIZE;
    int64 offset = *position % BLOCK_SIZE;
    u64 cnt = 0, length = 0, block = 0;
    buffer_t* bh;
    int64 ret = 0;

    // B.计算出可读取数据长度
    if (*position + count > inode->file_size)
        cnt = count = inode->file_size - *position;
    else
        cnt = count;

    if(cnt == 0)
        return ret;

    ret = cnt;    
    // C. 循环体实现数据读取过程
    do
    {
        block = minix_bmap(inode, index, 0);

        bh = bread(inode->dev, block, BLOCK_SIZE);

        // c.3. 计算本次从buffer缓冲区中复制给用户的数据长度
        length = cnt <= (BLOCK_SIZE - offset) ? cnt : BLOCK_SIZE - offset;

        // c.4. 根据buf是进程区内存 or 内核区内存，使用不同的复制函数
        if ((unsigned long)buf < TASK_SIZE)
            copy_to_user(bh->data + offset, buf, length);
        else
            memcpy(bh->data + offset, buf, length);
        
        bh->valid = true;

        index++; // 下一个块
        // c.5. 更新变量，进行下一次读取
        cnt -= length;
        offset -= offset; // 第二次循环后，offset = 0
        *position += length; // 更新文件读取指针 位置
        buf += length;

        brelse(bh);

    } while (cnt != 0);

    inode->atime = NOW();
    
    return ret;
}

long minix_write(struct file *filp, char *buf, unsigned long count, long *position) {
    
    inode_t* inode = filp->dentry->dir_inode;
    minix_inode_t* m_inode = (minix_inode_t*)inode->private_index_info;

    int64 index = *position / BLOCK_SIZE;
    int64 offset = *position % BLOCK_SIZE;
    u64 cnt = count, length = 0, block = 0;
    buffer_t* bh;
    int64 ret = 0;

    ret = cnt;    
    // C. 循环体实现数据读取过程
    do
    {
        block = minix_bmap(inode, index, true);

        bh = bread(inode->dev, block, BLOCK_SIZE);

        // c.3. 计算本次从buffer缓冲区中复制给用户的数据长度
        length = cnt <= (BLOCK_SIZE - offset) ? cnt : BLOCK_SIZE - offset;

        // c.4. 根据buf是进程区内存 or 内核区内存，使用不同的复制函数
        if ((unsigned long)buf < TASK_SIZE)
            copy_to_user(buf, bh->data + offset, length);
        else
            memcpy(buf, bh->data + offset, length);
        
        bh->dirty = true;

        index++; // 下一个块
        // c.5. 更新变量，进行下一次读取
        cnt -= length;
        offset -= offset; // 第二次循环后，offset = 0
        *position += length; // 更新文件读取指针 位置
        buf += length;

        brelse(bh);

    } while (cnt != 0);
    
    // B.更新文件数据长度和文件访问时间
    if (*position + count > inode->file_size)
        m_inode->size = inode->file_size = *position + count;
    
    m_inode->mtime = inode->atime = inode->ctime = NOW();
    // 标记inode已经被修改
    inode->buf->dirty = true;
    
    // TODO: 写入磁盘 ？
    bwrite(inode->buf);

    return ret;
}

long minix_ioctl(struct index_node *inode, struct file *filp, unsigned long cmd, unsigned long arg) { return 0; }

long minix_readdir(struct file* filp, void * dirent, filldir_t filler) {
    minix_dentry_t mentry;

    int64 ret = -1;
    if((ret = minix_read(filp, (char*)&mentry, sizeof(minix_dentry_t), &filp->position))!= sizeof(minix_dentry_t))
        return -1;

    struct dirent* dt = dirent;
    dt->nr = mentry.nr;
    dt->d_namelen = strlen(mentry.name);
    dt->d_offset = 0;
    memcpy(mentry.name, dt->d_name, dt->d_namelen);
    
    return ret;
}

struct file_operations minix_file_ops =
    {
        .open = minix_open,
        .close = minix_close,
        .read = minix_read,
        .write = minix_write,
        .lseek = FS_lseek,
        .ioctl = minix_ioctl,
        .readdir = minix_readdir,
};

/**
 * @brief 在目录中创建文件对应的 目录项
 * 
 * @param dir 目录的inode
 * @param dentry 文件的inode
 * @return buffer_t* 
 */
static buffer_t *add_dentry(inode_t *dir, struct dir_entry* dentry) {

    assert(dir->attribute == FS_ATTR_DIR)

    // A. 准备好目录项
    minix_dentry_t* dent = (minix_dentry_t*)kmalloc(sizeof(minix_dentry_t), 0); 
    memset(dent, 0, sizeof(minix_dentry_t));

    dent->nr = dentry->dir_inode->nr;
    memcpy(dentry->name, dent->name, dentry->name_length);

    // B. 遍历目录文件, 增加目录项
    u64 zone_idx = 0, block = 0;
    buffer_t* buf = NULL;
    minix_inode_t* m_parent_inode = dir->private_index_info;
    minix_inode_t* m_inode = dentry->dir_inode->private_index_info;
    minix_dentry_t* entry = NULL;
    while(true) {

        if(!buf || ((u64)entry) > ((u64)(buf->data) + BLOCK_SIZE)) {
            
            brelse(buf);
            block = minix_bmap(dir, zone_idx, true);
            assert(block > 0);

            buf = bread(dir->dev, block, BLOCK_SIZE);
            entry = (minix_dentry_t*)buf->data;
            zone_idx++;
        }
        
        if(entry->nr == 0 && entry->name[0] == '\0') {
            
            memcpy(dent, entry, sizeof(minix_dentry_t));
            
            // 强制同步 目录的修改时间？目录的大小？
            m_inode->nlinks++;                      // 更新文件的inode
            dentry->dir_inode->buf->dirty = true;
            bwrite(dentry->dir_inode->buf);
            
            dir->file_size += sizeof(minix_dentry_t); // 更新目录inode 节点信息
            dir->buf->dirty = true;
            m_parent_inode->size = dir->file_size;
            m_parent_inode->mtime = dir->ctime;
            dir->ctime = NOW();
            bwrite(dir->buf);

            buf->dirty = true;          // 更新目录项
            brelse(buf);
            break;
        }

        entry++;
    }

    return buf;
}


/**
 * @brief 删除文件名为name的文件，所对应的目录项
 * 
 * @param dir   被删除文件所在 目录
 * @param inode 被删除文件的inode
 * @param nr  
 * @param name 被删除文件的 文件名
 */
static void del_dentry(inode_t* dir, minix_inode_t* m_child_inode, u16 nr, char* name) {
    assert(dir->attribute == FS_ATTR_DIR)

    //遍历目录文件, 寻找目录项
    u64 zone_idx = 0, block = 0;
    buffer_t* buf = NULL;
    minix_inode_t* m_parent_inode = dir->private_index_info;
    minix_dentry_t* entry = NULL;
    while(true) {

        if(!buf || ((u64)entry) > ((u64)(buf->data) + BLOCK_SIZE)) {
            
            brelse(buf);
            block = minix_bmap(dir, zone_idx, true);
            assert(block > 0);

            buf = bread(dir->dev, block, BLOCK_SIZE);
            entry = (minix_dentry_t*)buf->data;
            zone_idx++;
        }
        
        if(entry->nr == nr && (strcmp(entry->name, name) == 0)) {
            
            dir->file_size -= sizeof(minix_dentry_t); // 更新目录inode 节点信息
            assert(m_parent_inode->nlinks > 0);
            m_parent_inode->size = dir->file_size;
            m_parent_inode->mtime = dir->ctime;
            dir->ctime = NOW();
            dir->buf->dirty = true;
            bwrite(dir->buf);

            m_child_inode->nlinks--; // 这里是否存在同步问题？
            
            entry->nr = 0;
            entry->name[0] = '\0';
            buf->dirty = true;          // 更新目录文件
            brelse(buf);
            break;
        }

        entry++;
    }

    return buf;
}


/**
 * @brief 释放m_inode所描述文件占有所有物理块
 * 
 * @param minix_sb 
 * @param m_inode 
 */
static void realse_file_data(super_t* minix_sb, minix_inode_t* m_inode) {
    
    u64 index = 0, i = 0, j = 0;
    u16 block = 0, *blk_indexs = NULL;
    buffer_t* buf = NULL;
    
    // a. 释放文件数据
    for(;index < DIRECT_BLOCK ; index++) { // 直接块
        block = m_inode->zone[index];
        if(block != 0)
            minix_bfree(minix_sb, block);
    }

    if(m_inode->zone[index] != 0) { // 间接块
        block = m_inode->zone[index];
        buf = bread(minix_sb->dev, block, BLOCK_SIZE);
        blk_indexs = (u16*)buf->data;
        
        for(; i < INDIRECT1_BLOCK; i++) { // 释放块内存
            if(blk_indexs[i] != 0 )
                minix_bfree(minix_sb, blk_indexs[i]);
        }
        
        minix_bfree(minix_sb, block); // 释放间接块内存
        index++;
    }

    if(m_inode->zone[index] != 0) { // 双重间接块
        // waiting rewrite
        // 这里的实现 需要三重循环，有没有什么方法。可以减少时间复杂度
        assert(0);
    }


}

/**
 * @brief
 *
 * @param inode 父目录的inode结点
 * @param dentry 新文件的目录项
 * @param mode
 * @return long
 */
long minix_create(struct index_node *inode, struct dir_entry *dentry, int32 mode) {
    super_t* sb = inode->sb;
    u64 nr = 0;

    if(dentry->name_length >= MINIX1_NAME_LEN)
        return -1;

    // A. 给新文件创建inode 并且初始化
    nr = minix_ialloc(sb);
    dentry->dir_inode = minix_new_node(sb->dev, nr, IFREG);
    
    // B. 给新文件创建 目录项
    add_dentry(inode, dentry);

    return 0;
}

struct dir_entry *minix_lookup(struct index_node *parent_inode, struct dir_entry *dest_dentry) {
    
    u64 dentries = parent_inode->file_size / sizeof(minix_dentry_t);
    idx_t i = 0, block = 0;
    buffer_t *buf = NULL;
    minix_dentry_t *entry = NULL;


    for(; i < dentries; entry++) {
        if(!buf || ((u64)entry) >= ((u64)buf->data + BLOCK_SIZE)) {

            brelse(buf);
            block = minix_bmap(parent_inode, i / BLOCK_DENTRIES, false);
            if(block == 0) {
                i += BLOCK_DENTRIES;
                continue;
            }

            buf = bread(parent_inode->dev, block, BLOCK_SIZE);
            entry = (minix_dentry_t*)buf->data;
        }

        if(entry->nr == 0 && entry->name[0] == '\0')
            continue;
        if(strncmp(entry->name, dest_dentry->name, dest_dentry->name_length) == 0) {
            dest_dentry->dir_inode = iget(parent_inode->dev, entry->nr);
            break;
        }
        i++;
    }

    return dest_dentry;
}

/**
 * @brief
 *
 * @param inode 父目录的inode结点
 * @param dentry 新文件的目录项
 * @param mode
 * @return long
 */
long minix_mkdir(struct index_node *inode, struct dir_entry *dentry, int mode) { 

    super_t* sb = inode->sb;
    u64 nr = 0;
    char name[4] = {0};

    if(dentry->name_length >= MINIX1_NAME_LEN)
        return -1;

    // A. 给新文件创建inode 并且初始化
    nr = minix_ialloc(sb);
    dentry->dir_inode = minix_new_node(sb->dev, nr, IFDIR);

    // B. 给新文件创建 目录项
    add_dentry(inode, dentry);


    // C. 在新文件创建 "." 和 ".." 目录项
    inode_t* i_child = dentry->dir_inode;

    dir_entry_t* i_entry = (dir_entry_t*)slab_malloc(Dir_Entry_Pool, 0);
    
    i_entry->dir_inode = i_child;
    i_entry->name_length = 1;
    name[0] = '.';
    i_entry->name = name;
    add_dentry(i_child, i_entry);

    i_entry->dir_inode = inode;
    i_entry->name_length = 2;
    name[1] = '.';
    add_dentry(i_child, i_entry);

    slab_free(Dir_Entry_Pool, i_entry, 0);
    
    return 0; 
}

long minix_rmdir(struct index_node *dir, struct dir_entry *dentry) { 
    long ret = -1;

    super_t* minix_sb = dentry->d_sb;
    minix_inode_t* m_inode = (minix_dentry_t*)dentry->dir_inode->private_index_info;
    
    if(m_inode->size != (sizeof(minix_dentry_t) * 2)) {
        color_printk(RED, BLACK, "rmdir: filed to remove: Directory not empty!\n");
        return -ENOTEMPTY;
    }
    
    // a. 释放目录数据
    realse_file_data(minix_sb, m_inode); // "." 和 ".."的目录项
    
    // b. here have joker. 我把父目录的m_inode传入了，进行 n_link链接减一的操作。
    del_dentry(dir, dir->private_index_info, dentry->dir_inode->nr, dentry->name);
    
    // c. 释放文件占用硬盘的inode
    minix_ifree(minix_sb, dentry->dir_inode->nr);
    
    // d. 删除 inode 在超级块中的索引
    list_del(&dentry->dir_inode->i_sb_list);
    
    // e. 释放 inode 私有引用
    ret = brelse(dentry->dir_inode->buf);
    
    return ret;
}

/**
 * @brief 删除文件。
 * 
 * @param dir 
 * @param dentry 
 * @return long 
 */
long minix_unlink(struct index_node *dir, struct dir_entry *dentry) {
    long ret = -1;

    super_t* minix_sb = dentry->d_sb;
    minix_inode_t* m_inode = (minix_dentry_t*)dentry->dir_inode->private_index_info;
    
    // a. 释放文件数据
    realse_file_data(minix_sb, m_inode);

    // c. 删除文件对应目录项
    del_dentry(dir, m_inode, dentry->dir_inode->nr, dentry->name);
    
    assert(m_inode->nlinks == 0);
    // b. 释放文件占用硬盘的inode
    minix_ifree(minix_sb, dentry->dir_inode->nr);
        
    // d. 删除 inode 在超级块中的索引
    list_del(&dentry->dir_inode->i_sb_list);
    
    // e. 释放 inode 私有引用
    brelse(dentry->dir_inode->buf);

    return ret;
}

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
        .unlink = minix_unlink,
};


// 同步该inode 到硬盘
inode_t *iput(dev_t dev, idx_t nr) {
    
    inode_t *inode = find_inode(dev, nr);
    if (inode)
    {
        inode->count--;
        inode->atime = NOW();
        return inode;
    }

    // super_t* super = get_super(dev);
    // minix_sb_info_t *minix_sb = (minix_sb_info_t*)(super->private_sb_info);

    // inode->buf = bread(dev, inode_block(minix_sb, nr), BLOCK_SIZE);
}

// 获得设备 dev 的 nr inode
inode_t *iget(dev_t dev, idx_t nr)
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
    inode->dev = dev;
    inode->inode_ops = &minix_inode_ops;
    inode->f_ops = &minix_file_ops;
    inode->count = 1; // 该变量应该是 一个原子变量
    inode->nr = nr;
    list_init(&inode->i_sb_list);
    list_add_to_behind(&super->inode_list, &inode->i_sb_list);
    inode->sb = super;
    inode->type = super->type; // 文件系统类型

    inode->buf = bread(dev, inode_block(minix_sb, nr), BLOCK_SIZE);
    minix_inode_t *mit = &((minix_inode_t*)inode->buf->data)[(inode->nr - 1) % BLOCK_INODES];
    
    inode->ctime = inode->mtime = inode->atime = mit->mtime;
    inode->i_mode = mit->mode;
    inode->file_size = mit->size;
    inode->gid = mit->gid;
    inode->uid = mit->uid;
    inode->blocks = (inode->file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    if(ISDIR(inode->i_mode)) 
        inode->attribute = FS_ATTR_DIR;
    else 
        inode->attribute = FS_ATTR_FILE;

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
    minix_sb_info_t* minix_sb = NULL;
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
    color_printk(ORANGE, BLACK, "MINIX FSinfo\nFirstdatalba:%#08lx\tinode_count:%#08lx\tlog_zone_size:%#08lx\n\
inode_map_size:%08lx\t zone_map_size:%08lx\t minix_magic:%08lx\n",
                minix_sb->firstdatazone,  minix_sb->inodes, minix_sb->log_zone_size, minix_sb->imap_blocks,
                minix_sb->zmap_blocks, minix_sb->magic);
    
    // directory entry 
    sbp->root = (dir_entry_t*)slab_malloc(Dir_Entry_Pool, 0);

    sbp->root->parent = sbp->root;
    sbp->root->name_length = 1;
    sbp->root->name = (char*)kmalloc(2, 0);
    sbp->root->name[0] = '/';
    sbp->root->dir_ops = &minix_dentry_ops;
    sbp->root->d_sb = sbp;

    // creat root inode
    sbp->root->dir_inode = iget(sbp->dev, 1);

    sbp->s_flags = false; // 标记挂载

    // ==============调试期间,暂把inode位图 和 物理块位图都读入内存，便于观察=================
    imap = bread(sbp->dev, 2,BLOCK_SIZE);
    zmap[0] = bread(sbp->dev, 2 + minix_sb->imap_blocks, BLOCK_SIZE);
    zmap[1] = bread(sbp->dev, 2 + minix_sb->imap_blocks + 1, BLOCK_SIZE);
    // ==================================================
    
    
    return sbp;

}


struct file_system_type MINIX_fs_type = {
        .name = "MINIX",
        .fs_flags = 0,
        .read_superblock = minix_read_superblock,
        .next = NULL,
};

