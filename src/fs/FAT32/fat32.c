#include "toolkit.h"
#include "fskit.h"
#include "devkit.h"
#include "mmkit.h"

#define FAT_DENRY_SIZE 32
#define EOC 0x0ffffff8 // end of clusterchain

struct Disk_Partition_Table DPT;
struct FAT32_BootSector fat32_bootsector;
struct FAT32_FSInfo fat32_fsinfo;

// unsigned long FirstDataSector = 0; // 数据区起始扇区号
// unsigned long BytesPerClus = 0;    // 每簇字节数
// unsigned long FirstFAT1Sector = 0; // FAT1表起始扇区号
// unsigned long FirstFAT2Sector = 0; // FAT2 表起始扇区号

/*FAT32簇号虽然占用32位，但只有28位有效*/
/**
 * @brief 在fat表中读出 簇号为fat_entry 对应的fat表项值
 *
 * @param fat_entry
 * @return unsigned int 簇号/结束标志
 */
unsigned int DISK1_FAT32_read_FAT_Entry(struct FAT32_sb_info *fsbi, unsigned int fat_entry)
{
    unsigned int buf[128];
    memset(buf, 0, 512);
    // fat_entry >> 7: 是因为一个扇区有128个fat表项， 这样会定位到簇号对应的偏移扇区
    IDE_device_operation.transfer(ATA_READ_CMD, fsbi->FAT1_firstsector + (fat_entry >> 7), 1, (unsigned char *)buf);

    // color_printk(BLUE, BLACK, "DISK1_FAT32_read_FAT_Entry fat_entry:%#018lx, %#010x\n", fat_entry, buf[fat_entry & 0x7f]);

    // fat_entry & 7f: 来定位本扇区内的，那个簇号
    return buf[fat_entry & 0x7f] & 0x0fffffff;
}

/**
 * @brief 在fat_entry对应的fat表项中，修改fat表项值为value
 *
 * @param fat_entry
 * @param value
 * @return unsigned long 成功返回1
 */
unsigned long DISK1_FAT32_write_FAT_Entry(struct FAT32_sb_info *fsbi, unsigned int fat_entry, unsigned int value)
{
    unsigned int buf[128];
    int i;
    memset(buf, 0, 512);
    // 读fat
    IDE_device_operation.transfer(ATA_READ_CMD, fsbi->FAT1_firstsector + (fat_entry >> 7), 1, (unsigned char *)buf);

    buf[fat_entry & 0x7f] = (buf[fat_entry & 0x7f] & 0xf0000000) | (value & 0x0fffffff);
    // 写fat1,fat2,...表
    for (i = 0; i < fsbi->NumFATs; i++)
        IDE_device_operation.transfer(ATA_WRITE_CMD, fsbi->FAT1_firstsector + fsbi->sector_per_FAT * i + (fat_entry >> 7), 1, (unsigned char *)buf);
    return 1;
}

/**
 * @brief 从FAT32文件系统中搜索出空闲簇号
 *
 * @param fsbi
 * @return unsigned long
 */
unsigned long FAT32_find_available_cluster(struct FAT32_sb_info *fsbi)
{
    int i, j;
    unsigned long sector_per_fat = fsbi->sector_per_FAT;
    unsigned int buf[128];

    // fsbi->fat_fsinfo->FSI_Free_Count & fsbi->fat_fsinfo->FSI_Nxt_Free not exactly, so unuse
    for (i = 0; i < sector_per_fat; i++)
    {
        memset(buf, 0, 512);
        IDE_device_operation.transfer(ATA_READ_CMD, fsbi->FAT1_firstsector + i, 1, (unsigned char *)buf);

        for (j = 0; j < 128; j++)
        {
            if ((buf[j] & 0x0fffffff) == 0)
                return j + i * 128; // return (i << 7) + j；
        }
    }
    return -1;
}

// 负责为访问文件数据提供操作方法
long FAT32_open(struct index_node *inode, struct file *filp) { return 1; }
long FAT32_close(struct index_node *inode, struct file *filp) { return 1; }

/**
 * @brief 在filp指定的文件中，从文件起始偏移position个字节开始，读取count字节数据，存入buf中
 *
 * @param filp
 * @param buf 存储读出数据的缓存区
 * @param count 要读取的字节数
 * @param position 相对于文件的偏移位置/目标位置, 该参数会被FAT32_read进行修改
 * @return long 成功返回读取的字节数，失败返回错误码
 */
long FAT32_read(struct file *filp, char *buf, unsigned long count, long *position)
{
    struct FAT32_inode_info *finode = filp->dentry->dir_inode->private_index_info;
    struct FAT32_sb_info *fsbi = filp->dentry->dir_inode->sb->private_sb_info;

    unsigned long cluster = finode->first_cluster; // 文件访问位置所在簇号
    unsigned long sector = 0;
    int i, length = 0;
    long retval = 0;
    int index = *position / fsbi->bytes_per_cluster;   // 目标位置偏移簇数 / 扇区数
    long offset = *position % fsbi->bytes_per_cluster; // 目标位置簇内偏移字节 / 扇区内偏移字节
    char *buffer = (char *)knew(fsbi->bytes_per_cluster, 0);

    if (!cluster)
        return -EFAULT;
    // A.得到要读的簇号
    for (i = 0; i < index; i++)
        cluster = DISK1_FAT32_read_FAT_Entry(fsbi, cluster);

    // B.计算出可读取数据长度
    if (*position + count > filp->dentry->dir_inode->file_size)
        index = count = filp->dentry->dir_inode->file_size - *position;
    else
        index = count;

    // preempt:先占，先取，current->preempt_count是当前进程持有自旋锁数量
    // color_printk(GREEN, BLACK, "FAT32_read- first_cluster:%d, size:%d, preempt_count:%d\n", finode->first_cluster, filp->dentry->dir_inode->file_size, current->preempt_count);

    // C. 循环体实现数据读取过程
    do
    {
        // c.0.清空读取缓冲区
        memset(buffer, 0, fsbi->bytes_per_cluster);
        // c.1.计算要读取的扇区号
        sector = fsbi->Data_firstsector + (cluster - 2) * fsbi->sector_per_cluster;

        // c.2. 读取整个簇的数据
        if (!IDE_device_operation.transfer(ATA_READ_CMD, sector, fsbi->sector_per_cluster, (unsigned char *)buffer))
        {
            color_printk(RED, BLACK, "FAT32 FS(read) read disk ERROR!!!!!!\n");
            retval = -EIO;
            break;
        }

        // c.3. 计算本次从buffer缓冲区中复制给用户的数据长度
        length = index <= (fsbi->bytes_per_cluster - offset) ? index : fsbi->bytes_per_cluster - offset;

        // c.4. 根据buf是进程区内存 or 内核区内存，使用不同的复制函数
        if ((unsigned long)buf < TASK_SIZE)
            copy_to_user(buffer + offset, buf, length);
        else
            memcpy(buffer + offset, buf, length);

        // c.5. 更新变量，进行下一次读取
        index -= length;
        buf += length;
        offset -= offset; // 第二次循环后，offset = 0
        *position += length;

    } while (index && (cluster = DISK1_FAT32_read_FAT_Entry(fsbi, cluster)));

    kdelete(buffer, fsbi->bytes_per_cluster);

    // index值若为0，则说明数据已经成功从文件读取出来，随即返回已读取数据长度
    // 否则说明数据读取错误，进而返回错误码
    if (!index)
        retval = count;
    return retval;
}

/**
 * @brief 在filp指定的文件中，从文件起始偏移position个字节开始，写出count字节数据，
 *
 * @param filp
 * @param buf 存储要写入数据的缓存区
 * @param count 要写入的字节数
 * @param position 相对于文件的偏移位置/目标位置
 * @return long 成功返回读取的字节数，失败返回错误码
 */
long FAT32_write(struct file *filp, char *buf, unsigned long count, long *position)
{
    // a. 得到FAT32文件系统的元数据
    struct FAT32_inode_info *finode = filp->dentry->dir_inode->private_index_info;
    struct FAT32_sb_info *fsbi = filp->dentry->dir_inode->sb->private_sb_info;

    unsigned long cluster = finode->first_cluster; // 要写入文件的第一个簇
    unsigned long next_cluster = 0;
    unsigned long sector = 0;
    int i, length = 0;
    long retval = 0;
    long flags = 0; // 是否要第一次写空文件
    int index = *position / fsbi->bytes_per_cluster;
    long offset = *position % fsbi->bytes_per_cluster;
    char *buffer = (char *)knew(fsbi->bytes_per_cluster, 0);

    if (!cluster) // 要写入的文件没有被分配簇号，那么获得簇号
    {
        cluster = FAT32_find_available_cluster(fsbi);
        flags = 1;
    }
    else // 计算要写入的簇号
        for (i = 0; i < index; i++)
            cluster = DISK1_FAT32_read_FAT_Entry(fsbi, cluster);

    if (!cluster)
    { // 硬盘已无法分配多余簇号
        kdelete(buffer, fsbi->bytes_per_cluster);
        return -ENOSPC;
    }

    if (flags)
    { // 创建文件后，第一次写入文件时，缓存本文件的目录，
        finode->first_cluster = cluster;
        filp->dentry->dir_inode->sb->sb_ops->write_inode(filp->dentry->dir_inode);
        // 在申请好的FAT表项内先填入写入标志
        DISK1_FAT32_write_FAT_Entry(fsbi, cluster, EOC);
    }

    // index = 是要写入的总字节数
    index = count;

    // 当取得目标簇号以后，便可开始数据写入工作
    // 当数据全部写入，或文件系统已满，或在硬盘写入期间出错，循环体才会退出
    do
    {
        // 文件不是首次创建，先读出文件要写入的位置
        memset(buffer, 0, fsbi->bytes_per_cluster);

        sector = fsbi->Data_firstsector + (cluster - 2) * fsbi->sector_per_cluster;

        // flags标记当前簇的操作状态为已使用还是新分配。
        // 对于已使用的簇，必须将簇数据读出来，再写入数据覆盖到读取缓存区中
        if (!flags)
            if (!IDE_device_operation.transfer(ATA_READ_CMD, sector, fsbi->sector_per_cluster, (unsigned char *)buffer))
            {
                color_printk(RED, BLACK, "FAT32 FS(write) read disk ERROR!!!!\n");
                retval = -EIO;
                break;
            }

        length = index <= (fsbi->bytes_per_cluster - offset) ? index : fsbi->bytes_per_cluster - offset;

        if ((unsigned long)buf < TASK_SIZE)
            copy_from_user(buf, buffer + offset, length);
        else
            memcpy(buf, buffer + offset, length);

        // 把准备好的数据写入硬盘
        if (!IDE_device_operation.transfer(ATA_WRITE_CMD, sector, fsbi->sector_per_cluster, (unsigned char *)buffer))
        {
            color_printk(RED, BLACK, "FAT32 FS(write) write disk ERROR!!!!\n");
            retval = -EIO;
            break;
        }

        // 更新下一次循环的数据
        index -= length;
        buf += length;
        offset -= offset;
        *position += length;

        // 得到下一个簇号
        if (index)
            next_cluster = DISK1_FAT32_read_FAT_Entry(fsbi, cluster);
        else
            break;
        // 文件写入到，最后一个簇号
        if (next_cluster >= EOC)
        {
            next_cluster = FAT32_find_available_cluster(fsbi);
            if (!next_cluster)
            {
                kdelete(buffer, fsbi->bytes_per_cluster);
                return -ENOSPC;
            }

            // 簇号的连接就是链表
            DISK1_FAT32_write_FAT_Entry(fsbi, cluster, next_cluster);
            DISK1_FAT32_write_FAT_Entry(fsbi, next_cluster, EOC); // 写入结束标志
            flags = 1;
        }

        cluster = next_cluster;

    } while (index);

    // 本次写操作，写超了文件大小，所以要更新目录项。日后要添加的功能:此处也应该更新一下修改时间呢？
    if (*position > filp->dentry->dir_inode->file_size)
    {
        filp->dentry->dir_inode->file_size = *position;
        filp->dentry->dir_inode->sb->sb_ops->write_inode(filp->dentry->dir_inode);
    }

    kdelete(buffer, fsbi->bytes_per_cluster);
    if (!index)
        retval = count;
    return retval;
}

long FAT32_ioctl(struct index_node *inode, struct file *filp, unsigned long cmd, unsigned long arg) { return 0; }

int fill_dentry(void* buf, char*name, long namelen, long offset)
{
    struct dirent* dent = (struct dirent*)buf;
    if((unsigned long) buf < TASK_SIZE && !verify_area(buf, sizeof(struct dirent) + namelen))
        return -EFAULT;
    memcpy(name, dent->d_name, namelen);
    dent->d_namelen = namelen;
    dent->d_offset = offset;
    return sizeof(struct dirent) + namelen;

}

long FAT32_readdir(struct file* filp, void * dirent, filldir_t filler)
{
    struct FAT32_inode_info* finode = filp->dentry->dir_inode->private_index_info;
    struct FAT32_sb_info* fsbi = filp->dentry->dir_inode->sb->private_sb_info;

    unsigned int cluster = 0;
    unsigned long sector = 0;
    unsigned char * buf = NULL;
    char* name = NULL;
    int namelen = 0;
    int i = 0, j = 0, x = 0,y = 0;
    struct FAT32_Directory* tmpdentry = NULL;
    struct FAT32_LongDirectory* tmpldentry = NULL;

    buf = knew(fsbi->bytes_per_cluster, 0);
    
    //定位本次操作的簇号。
    cluster = finode->first_cluster;
    j = filp->position / fsbi->bytes_per_cluster;
    for(i = 0; i < j; i++)
    {
        cluster = DISK1_FAT32_read_FAT_Entry(fsbi,cluster);
        if(cluster > 0x0ffffff7) // 0x0fffffff7文件结束标志。
        { // 目录完了
           // color_printk(RED, BLACK, "FAT32 FS(readdir) cluster didn't exist\n");
            return 0;
        }
    }

next_cluster:
    sector = fsbi->Data_firstsector + (cluster - 2) * fsbi->sector_per_cluster;
    if(!(IDE_device_operation.transfer(ATA_READ_CMD,sector, fsbi->sector_per_cluster, buf)))
    {
        color_printk(RED, BLACK, "FS::FAT32_readdir read disk ERROR!!!\n");
        kdelete(buf, fsbi->bytes_per_cluster);
        return 0;
    }

    tmpdentry = (struct FAT32_Directory*)(buf + (filp->position % fsbi->bytes_per_cluster));
    for(i = filp->position % fsbi->bytes_per_cluster; i < fsbi->bytes_per_cluster; i += 32, tmpdentry++,filp->position+=32)
    {
        if(tmpdentry->DIR_Attr == ATTR_LONG_NAME) continue;
        if(tmpdentry->DIR_Name[0] == 0xe5 || tmpdentry->DIR_Name[0] == 0x00 ||
            tmpdentry->DIR_Name[0] == 0x05)
            continue;
        
        namelen = 0;
        tmpldentry = (struct FAT32_LongDirectory*)tmpdentry - 1;

        if(tmpldentry->LDIR_Attr == ATTR_LONG_NAME && tmpldentry->LDIR_Ord != 0xe5 &&
        tmpldentry->LDIR_Ord != 0x00 && tmpldentry->LDIR_Ord != 0x05)
        {
            j = 0;
            // long file / dir name read
            while(tmpldentry->LDIR_Attr == ATTR_LONG_NAME && tmpldentry->LDIR_Ord != 0xe5 && 
            tmpldentry->LDIR_Ord != 0x00 && tmpldentry->LDIR_Ord != 0x05)
            {
                j++;
                if(tmpldentry->LDIR_Ord & 0x40)
                    break;
                tmpldentry--;
            }
            name = knew(j * 13+ 1, 0);
            memset(name, 0 , j * 13 + 1);
            tmpldentry = (struct FAT32_LongDirectory*)tmpdentry - 1;

            for(x = 0; x < j; x++, tmpldentry--)
            {
                for(y = 0; y < 5; y++)
                    if(tmpldentry->LDIR_Name1[y] != 0xffff && tmpldentry->LDIR_Name1[y] != 0x0000)
                        name[namelen++] = (char)tmpldentry->LDIR_Name1[y];
                for(y = 0; y < 6; y++)
                    if(tmpldentry->LDIR_Name2[y] != 0xffff && tmpldentry->LDIR_Name2[y] != 0x0000)
                        name[namelen++] = (char)tmpldentry->LDIR_Name2[y];
                for(y = 0; y < 2; y++)
                    if(tmpldentry->LDIR_Name3[y] != 0xffff && tmpldentry->LDIR_Name3[y] != 0x0000)
                        name[namelen++] = (char)tmpldentry->LDIR_Name3[y];
            }
            goto find_lookup_success;
        }

        name = knew(15, 0);
        memset(name, 0 , 15);
        // short file/dir beae name compare
        for(x = 0; x < 8; x ++)
        {
            if(tmpdentry->DIR_Name[x] == ' ') break;
            if(tmpdentry->DIR_NTRes & LOWERCASE_BASE)
                name[namelen++] = tmpdentry->DIR_Name[x] + 32;
            else
                name[namelen++] = tmpdentry->DIR_Name[x];
        }
        if(tmpdentry->DIR_Attr & ATTR_DIRECTORY)
            goto find_lookup_success;
        name[namelen++] = '.';
        //short file ext name compare
        for(x = 8; x < 11; x++)
        {
            if(tmpdentry->DIR_Name[x] == ' ') break;
            if(tmpdentry->DIR_NTRes&LOWERCASE_EXT)
                name[namelen++] = tmpdentry->DIR_Name[x] + 32;
            else
                name[namelen++] = tmpdentry->DIR_Name[x]; 
        }
        if(x == 8)
            name[--namelen] = 0;
        goto find_lookup_success;
    }
    cluster = DISK1_FAT32_read_FAT_Entry(fsbi, cluster);
    if(cluster < 0x0ffffff7)
        goto next_cluster;
    kdelete(buf, fsbi->bytes_per_cluster);
    return 0;
find_lookup_success:
    filp->position += 32;
    return filler(dirent,name,namelen, 0);
}
struct file_operations FAT32_file_ops =
    {
        .open = FAT32_open,
        .close = FAT32_close,
        .read = FAT32_read,
        .write = FAT32_write,
        .lseek = FS_lseek,
        .ioctl = FAT32_ioctl,
        .readdir = FAT32_readdir,
};

/**
 * @brief Returns an unsigned byte checksum computed on an unsigned byte array.
 * The array must be 11 bytes long and is assumed to contain  a name stored
 * in the format of a MS-DOS directory entry.
 *
 * @param pFcbName Pointer to an unsigned byte array assumed to be 11 bytes long.
 * @return unsigned char Sum An 8-bit unsigned checksum of the array pointed to by pFcbName.
 */
static unsigned char FAT32_ChkSum(unsigned char *pFcbName)
{
    short FcbNameLen;
    unsigned char sum;
    sum = 0;
    for (FcbNameLen = 11; FcbNameLen != 0; FcbNameLen--)
    {
        // NOTE: The operation si an unsigned char rotate right
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *pFcbName++;
    }
    return sum;
}
#define LDir_NameCount 13
/**
 * @brief 创建FAT32的目录项
 *        a. 创建目录项并初始化
 *        b. 为新建立的文件， 创建了inode结点
 * @param inode 父目录的inode结构
 * @param dentry 里面保存了要创建目录项的文件名和长度
 * @param mode
 * @param size 传出参数，记录目录项占用字节数
 * @return struct FAT32_LongDirectory*
 */
static struct FAT32_LongDirectory *Create_FAT32DEntry(struct index_node *inode, struct dir_entry *dentry, int mode, unsigned long *size)
{
    struct FAT32_LongDirectory *fld, *fld0;
    struct FAT32_Directory *fd;
    struct FAT32_sb_info *fsbi = inode->sb->private_sb_info;
    int i = 0, j = 0, LDir_count = 0, k = 0;
    long ExpandNameIndex = -1, namelen = dentry->name_length; // 扩展名索引，字符串长度

    char tmpName[namelen + 1];
    memset(tmpName, 0, namelen + 1);
    if (dentry->name_length > 64)
    {
        color_printk(WHITE, BLACK, "FAT32(Create_FAT32DEntry) ERROR:: Name Length Too Long!");
        return NULL;
    }
    strncpy(tmpName, dentry->name, namelen);

    LDir_count = namelen / LDir_NameCount + 1; // 计算需要创建的长目录项个数
    fld = knew(FAT_DENRY_SIZE * (LDir_count + 1), 0);
    memset(fld, 0xff, FAT_DENRY_SIZE * (LDir_count + 1));

    *size = FAT_DENRY_SIZE * (LDir_count + 1); // 为传出参数赋值

    fd = (struct FAT32_Directory *)(fld + LDir_count); // 定位短目录项的长度
    // ------------------------- 初始化长目录项------------------
    fld0 = (struct FAT32_LongDirectory *)fd - 1;
    unsigned char ChkSum_ = FAT32_ChkSum((unsigned char*)tmpName);
    j = 0;
    for (; k < LDir_count; k++, fld0--)
    {
        fld0->LDIR_Attr = ATTR_LONG_NAME;

        if (k == LDir_count - 1)
            fld0->LDIR_Ord = (0x40 | (k + 1)); // 长目录项结束标志
        else
            fld0->LDIR_Ord = (k + 1);

        fld0->LDIR_Chksum = ChkSum_;
        fld0->LDIR_FstClusLO = fld0->LDIR_Type = 0;

        for (i = 0; i < 5 && j < namelen;)
            fld0->LDIR_Name1[i++] = tmpName[j++];
        for (i = 0; i < 6 && j < namelen;)
            fld0->LDIR_Name2[i++] = tmpName[j++];
        for (i = 0; i < 2 && j < namelen;)
            fld0->LDIR_Name3[i++] = tmpName[j++];
    }

    // 添加LDIR_NAME的文件名结束标志
    int last_index = namelen % LDir_NameCount;
    if (last_index >= 11)
    {
        last_index -= 11;
        fld0->LDIR_Name3[last_index] = 0;
    }
    else if (last_index >= 5)
    {
        last_index -= 5;
        fld0->LDIR_Name2[last_index] = 0;
    }
    else
        fld0->LDIR_Name1[last_index] = 0;

    // ------------------- 初始化短目录项 -------------------------------
    fd->DIR_NTRes = fd->DIR_LastAccDate = 0; // 关于时间的属性，目前忽略为0
    fd->DIR_CrtTimeTenth = fd->DIR_CrtDate = fd->DIR_CrtTime = 0;
    fd->DIR_WrtTime = fd->DIR_WrtDate = 0;
    // 此处没有给性文件分配簇号
    fd->DIR_FstClusHI = fd->DIR_FstClusLO = 0;
    // 文件大小为0
    fd->DIR_FileSize = 0;
    // 拥有长文件名，段目录项. 目前只支持创建文件，而非目录
    fd->DIR_Attr = ATTR_ARCHIVE;

    // 拷贝文件名, ASCII中 0x20是 空格.
    // 短目录项长度11B, 其中基础名8B, 扩展名3B
    memset(fd->DIR_Name, 0x20, sizeof(fd->DIR_Name));

    ExpandNameIndex = str_find_char(dentry->name, '.', dentry->name_length);
    if (ExpandNameIndex == 0)
    {
        color_printk(WHITE, BLACK, "The name \".\" not vaild as a file or a floder name");
        return 0;
    }
    // 把文件名词都转换成大写, 短文件的目录项都强制是大写的
    upper(tmpName);
    // 拷贝基础名
    i = j = 0;
    while (j < 8 && i < ExpandNameIndex)
        fd->DIR_Name[j++] = tmpName[i++];
    // 拷贝扩展名
    j = 8, i = ExpandNameIndex + 1;
    while (j < 11 && i < namelen && ExpandNameIndex != -1)
        fd->DIR_Name[j++] = tmpName[i++];
    // ------------------------------------------------------------------
    /*寻找成功后，创建本文件的index_node，通过物理目录项，获得该文件的全部信息*/
    struct index_node *p;
    p = (struct index_node *)knew(sizeof(struct index_node), 0);
    memset(p, 0, sizeof(struct index_node));
    // 文件普遍都有的信息
    p->file_size = fd->DIR_FileSize;
    p->blocks = (p->file_size + fsbi->bytes_per_cluster - 1) / fsbi->bytes_per_cluster;
    p->attribute = (fd->DIR_Attr & ATTR_DIRECTORY) ? FS_ATTR_DIR : FS_ATTR_FILE;
    p->sb = inode->sb;
    p->f_ops = &FAT32_file_ops;
    p->inode_ops = &FAT32_inode_ops;
    struct FAT32_inode_info *finode;
    // FAT32 特有的
    p->private_index_info = (struct FAT32_inode_info *)knew(sizeof(struct FAT32_inode_info), 0);
    memset(p->private_index_info, 0, sizeof(struct FAT32_inode_info));
    finode = p->private_index_info;
    if (mode)
    {
        finode->first_cluster = 0x10000000;
        fd->DIR_FstClusHI = 0x1000;
        p->attribute = FS_ATTR_DEVICE_KEYBOARD;
    }
    else
        finode->first_cluster = 0;

    // 文件对应目录项的位置信息 -- 在FAT32_create中填充
    finode->dentry_location = 0;
    finode->dentry_position = 0;

    finode->create_date = fd->DIR_CrtDate;
    finode->create_time = fd->DIR_CrtTime;
    finode->write_date = fd->DIR_WrtDate;
    finode->write_time = fd->DIR_WrtTime;

    dentry->dir_inode = p;
    return fld;
}

/**
 * @brief
 *
 * @param inode 父目录的inode结点
 * @param dentry 新文件的目录项
 * @param mode
 * @return long
 */
long FAT32_create(struct index_node *inode, struct dir_entry *dentry, int mode)
{
    // a. 得到FAT32文件系统的元数据
    struct FAT32_inode_info *Parent_finode = inode->private_index_info, *finode;
    struct FAT32_sb_info *fsbi = inode->sb->private_sb_info;
    unsigned long cluster = Parent_finode->first_cluster; // 要写入文件的第一个簇
    unsigned long next_cluster = 0;
    unsigned long sector = 0;
    long retval = 0, i = 0;
    unsigned long dir_entry_size_total = 0;
    char *buffer = (char *)knew(fsbi->bytes_per_cluster, 0);

    struct FAT32_LongDirectory *fld;
    struct FAT32_Directory *entry;

    fld = Create_FAT32DEntry(inode, dentry, mode, &dir_entry_size_total);
    finode = dentry->dir_inode->private_index_info;
    int flag_finish = 1;
    do
    {
        memset(buffer, 0, fsbi->bytes_per_cluster);

        sector = fsbi->Data_firstsector + (cluster - 2) * fsbi->sector_per_cluster;

        if (!IDE_device_operation.transfer(ATA_READ_CMD, sector, fsbi->sector_per_cluster, (unsigned char *)buffer))
        {
            kdelete(buffer, fsbi->bytes_per_cluster);
            kdelete(fld, sizeof(FAT32_longdirectory_t));
            // 注意此处失败 还应该 回收inode, dentry, 占用的动态内存
            color_printk(RED, BLACK, "FAT32 inode(Create) write disk ERROR!!!!\n");
            retval = -EIO;
            break;
        }

        entry = (struct FAT32_Directory *)buffer;
        for (i = 0; i <= fsbi->bytes_per_cluster / FAT_DENRY_SIZE; i++, entry++)
            if (entry->DIR_Name[0] == 0)
            { // 成功找到
                memcpy(fld, buffer + FAT_DENRY_SIZE * i, dir_entry_size_total);
                IDE_device_operation.transfer(ATA_WRITE_CMD, sector, fsbi->sector_per_cluster, (unsigned char *)buffer);
                finode->dentry_location = cluster;
                finode->dentry_position = FAT_DENRY_SIZE * i;
                retval = flag_finish = 0;
                break;
            }

        next_cluster = DISK1_FAT32_read_FAT_Entry(fsbi, cluster);
        if (next_cluster >= EOC && flag_finish)
        {
            next_cluster = FAT32_find_available_cluster(fsbi);
            if (!next_cluster)
            {
                kdelete(fld, sizeof(FAT32_longdirectory_t));
                kdelete(buffer, fsbi->bytes_per_cluster);
                return -ENOSPC;
            }

            // 簇号的连接就是链表
            DISK1_FAT32_write_FAT_Entry(fsbi, cluster, next_cluster);
            DISK1_FAT32_write_FAT_Entry(fsbi, next_cluster, EOC); // 写入结束标志
        }

        cluster = next_cluster;
    } while (flag_finish);

    return retval;
}

/**
 * @brief 负责从目录项中搜索出子目录项
 *
 * @param parent_inode 父目录的inode
 * @param dest_dentry  把找出来的目录项, 记录在dest_dentry结构中(传出参数
 * @return struct dir_entry* 返回dest_dentry结构
 */
struct dir_entry *FAT32_lookup(struct index_node *parent_inode, struct dir_entry *dest_dentry)
{
    // 得到FAT32的基础信息结构体
    struct FAT32_inode_info *finode = parent_inode->private_index_info;
    struct FAT32_sb_info *fsbi = parent_inode->sb->private_sb_info;

    unsigned int cluster = 0;  // 簇号
    unsigned long sector = 0;  // 扇区号
    unsigned char *buf = NULL; // 保存硬盘数据的缓冲区
    int i = 0, j = 0, x = 0;
    struct FAT32_Directory *tmpdentry = NULL;
    struct FAT32_LongDirectory *tmpldentry = NULL;
    struct index_node *p = NULL;

    buf = knew(fsbi->bytes_per_cluster, 0); // 申请缓冲区
    cluster = finode->first_cluster;           // 根目录的簇号
next_cluster:
    sector = fsbi->Data_firstsector + (cluster - 2) * fsbi->sector_per_cluster; // 计算要读取的扇区号
    // color_printk(BLUE, BLACK, "lookup cluster:%#010x, sector: %#018lx\n", cluster, sector);
    if (!IDE_device_operation.transfer(ATA_READ_CMD, sector, fsbi->sector_per_cluster, (unsigned char *)buf))
    { // 读目录的数据块
        color_printk(RED, BLACK, "FAT32 FS(lookup) read disk ERROR!!!!!!!\n");
        kdelete(buf, fsbi->bytes_per_cluster);
        return NULL;
    }

    tmpdentry = (struct FAT32_Directory *)buf;
    for (i = 0; i < fsbi->bytes_per_cluster; i += 32, tmpdentry++)
    {
        if (tmpdentry->DIR_Attr == ATTR_LONG_NAME) // 跳过长目录项
            continue;
        // 跳过空闲目录项
        if (tmpdentry->DIR_Name[0] == 0xe5 || tmpdentry->DIR_Name[0] == 0x00 || tmpdentry->DIR_Name[0] == 0x05)
            continue;

        // 长目录项排列的时候，在短目录项之前，而且长/短目录项都是32B，32字节.
        // so 可以用下面的方式得到长目录项
        tmpldentry = (struct FAT32_LongDirectory *)tmpdentry - 1;
        j = 0;

        //  long file/dir name compare, 若长目录下匹配成功 则不会进行短目录项的匹配
        //  逐次匹配多个长目录项
        while (((struct FAT32_Directory *)tmpldentry)->DIR_Attr == ATTR_LONG_NAME && tmpldentry->LDIR_Ord != 0xe5)
        {
            // 依次匹配长目录项名字成员的三部分
            for (x = 0; x < 5; x++)
            {
                if (j > dest_dentry->name_length && tmpldentry->LDIR_Name1[x] == 0xffff)
                    continue;
                // LDIR_Name1使用的编码方式是Unicode编码，而name[]使用的是ACSii码, 但由于Unicode编码发展与ACSII码, 所以可以进行比较
                // 这里的unsigned short强转，是把char类型转换成unsigned short.所以本系统目前不支持汉字文件名
                else if (j > dest_dentry->name_length || tmpldentry->LDIR_Name1[x] != (unsigned short)(dest_dentry->name[j++]))
                    goto continue_cmp_fail;
            }
            for (x = 0; x < 6; x++)
            {
                if (j > dest_dentry->name_length && tmpldentry->LDIR_Name2[x] == 0xffff)
                    continue;
                else if (j > dest_dentry->name_length || tmpldentry->LDIR_Name2[x] != (unsigned short)(dest_dentry->name[j++]))
                    goto continue_cmp_fail;
            }
            for (x = 0; x < 2; x++)
            {
                if (j > dest_dentry->name_length && tmpldentry->LDIR_Name3[x] == 0xffff)
                    continue;
                else if (j > dest_dentry->name_length || tmpldentry->LDIR_Name3[x] != (unsigned short)(dest_dentry->name[j++]))
                    goto continue_cmp_fail;
            }

            // 找到了确定的目录项
            if (j >= dest_dentry->name_length)
                goto find_lookup_success;

            // 匹配多个目录项
            tmpldentry--;
        }

        // short file/dir base name compare
        j = 0;
        for (x = 0; x < 8; x++)
        {
            switch (tmpdentry->DIR_Name[x])
            {
                // 短目录项的前八的名字的空位都是' '
            case ' ':
                if (!(tmpdentry->DIR_Attr & ATTR_DIRECTORY))
                { // 本文件不是目录
                    if (dest_dentry->name[j] == '.')
                        continue; // 同步字符串
                    else if (tmpdentry->DIR_Name[x] == dest_dentry->name[j])
                    {
                        j++;
                        break;
                    }
                    else
                        goto continue_cmp_fail;
                }
                else
                { // 当前文件是目录
                    if (j < dest_dentry->name_length && tmpdentry->DIR_Name[x] == dest_dentry->name[j])
                    {
                        j++;
                        break;
                    }
                    else if (j == dest_dentry->name_length)
                        continue;
                    else
                        goto continue_cmp_fail;
                }
            case 'A' ... 'Z':
            case 'a' ... 'z':
                if (!(tmpdentry->DIR_NTRes & LOWERCASE_BASE)) // 这个LOWERCASE_BASE是为了兼容Windows操作系统，才准备的
                {                                             // 这一段代码是什么意思？
                    if (j < dest_dentry->name_length && tmpdentry->DIR_Name[x] + 32 == dest_dentry->name[j])
                    {
                        j++;
                        break;
                    }
                    else
                        goto continue_cmp_fail;
                }
                else
                {
                    if (j < dest_dentry->name_length && tmpdentry->DIR_Name[x] == dest_dentry->name[j])
                    {
                        j++;
                        break;
                    }
                    else
                        goto continue_cmp_fail;
                }
            case '0' ... '9':
            default:
                if (j < dest_dentry->name_length && tmpdentry->DIR_Name[x] == dest_dentry->name[j])
                {
                    j++;
                    break;
                }
                else
                    goto continue_cmp_fail;
                break;
            }
        }
        // short file ext name compare
        if (!(tmpdentry->DIR_Attr & ATTR_DIRECTORY))
        { // 不是目录，目录没有扩展名
            j++;
            for (x = 8; x < 11; x++)
            {
                switch (tmpdentry->DIR_Name[x])
                {
                case 'A' ... 'Z':
                case 'a' ... 'z':
                    if (tmpdentry->DIR_NTRes & LOWERCASE_EXT)
                        if (tmpdentry->DIR_Name[x] + 32 == dest_dentry->name[j])
                        {
                            j++;
                            break;
                        }
                        else
                            goto continue_cmp_fail;
                    else
                    {
                        if (tmpdentry->DIR_Name[x] == dest_dentry->name[j])
                        {
                            j++;
                            break;
                        }
                        else
                            goto continue_cmp_fail;
                    }

                case '0' ... '9':
                    if (tmpdentry->DIR_Name[x] == dest_dentry->name[j])
                    {
                        j++;
                        break;
                    }
                    else
                        goto continue_cmp_fail;

                case ' ':
                    if (tmpdentry->DIR_Name[x] == dest_dentry->name[j])
                    {
                        j++;
                        break;
                    }
                    else
                        goto continue_cmp_fail;

                default:
                    goto continue_cmp_fail;
                }
            }
        }

        goto find_lookup_success;
    continue_cmp_fail:;
    }

    // 得到目录本文件的下一个簇号
    cluster = DISK1_FAT32_read_FAT_Entry(fsbi, cluster);
    if (cluster < 0x0ffffff7) // 判断簇号是否为结束簇or坏簇。如果是正常簇号，就继续读下个簇
        goto next_cluster;

    kdelete(buf, fsbi->bytes_per_cluster);
    return NULL; // 寻找失败返回空

find_lookup_success:
    /*寻找成功后，创建本文件的index_node，通过读取到的物理目录项，获得该文件的全部信息*/
    p = (struct index_node *)knew(sizeof(struct index_node), 0);
    memset(p, 0, sizeof(struct index_node));
    // 文件普遍都有的信息
    p->file_size = tmpdentry->DIR_FileSize;

    // 这一句不太能理解
    p->blocks = (p->file_size + fsbi->bytes_per_cluster - 1) / fsbi->bytes_per_sector;

    p->attribute = (tmpdentry->DIR_Attr & ATTR_DIRECTORY) ? FS_ATTR_DIR : FS_ATTR_FILE;
    p->sb = parent_inode->sb;
    p->f_ops = &FAT32_file_ops;
    p->inode_ops = &FAT32_inode_ops;

    // FAT32 特有的
    p->private_index_info = (struct FAT32_inode_info *)knew(sizeof(struct FAT32_inode_info), 0);
    memset(p->private_index_info, 0, sizeof(struct FAT32_inode_info));
    finode = p->private_index_info;

    // 得到文件数据的第一个簇号
    finode->first_cluster = (tmpdentry->DIR_FstClusHI << 16 | tmpdentry->DIR_FstClusLO) & 0x0fffffff;

    // 文件对应目录项的位置信息
    finode->dentry_location = cluster;
    finode->dentry_position = tmpdentry - (struct FAT32_Directory *)buf;

    finode->create_date = tmpdentry->DIR_CrtDate;
    finode->create_time = tmpdentry->DIR_CrtTime;
    finode->write_date = tmpdentry->DIR_WrtDate;
    finode->write_time = tmpdentry->DIR_WrtTime;

    if ((tmpdentry->DIR_FstClusHI >> 12) && (p->attribute & FS_ATTR_FILE))
        p->attribute |= FS_ATTR_DEVICE_KEYBOARD;

    dest_dentry->dir_inode = p;
    kdelete(buf, fsbi->bytes_per_cluster);
    return dest_dentry;
}
long FAT32_mkdir(struct index_node *inode, struct dir_entry *dentry, int mode) { return 0; }
long FAT32_rmdir(struct index_node *inode, struct dir_entry *dentry) { return 0;}
long FAT32_rename(struct index_node *old_inode, struct dir_entry *old_dentry, struct index_node *new_inode, struct dir_entry *new_dentry) { return 0; }
long FAT32_getattr(struct dir_entry *dentry, unsigned long *attr) { return 0;}
long FAT32_setattr(struct dir_entry *dentry, unsigned long *attr) { return 0;}

struct index_node_operations FAT32_inode_ops =
    {
        .create = FAT32_create,
        .lookup = FAT32_lookup,
        .mkdir = FAT32_mkdir,
        .rmdir = FAT32_rmdir,
        .rename = FAT32_rename,
        .getattr = FAT32_getattr,
        .setattr = FAT32_setattr,
        .unlink = NULL,
};

//// these operation need cache and list - 为缓存目录项提供操作方法
long FAT32_compare(struct dir_entry *parent_dentry, char *source_filename, char *destination_filename) { return 0; }
long FAT32_hash(struct dir_entry *dentry, char *filename) { return 0; }
long FAT32_release(struct dir_entry *dentry) { return 0; }                        // 释放目录项
long FAT32_iput(struct dir_entry *dentry, struct index_node *inode) { return 0; } // 释放inode索引
struct dir_entry_operations FAT32_dentry_ops =
    {
        .compare = FAT32_compare,
        .hash = FAT32_hash,
        .release = FAT32_release,
        .iput = FAT32_iput,
        .d_delete = NULL,
};

// 修改超级块？
void fat32_write_superblock(struct super_block *sb) {}

// 用于释放superblock结构，dentry结构，根目录inode结构，文件系统的特有结构
void fat32_put_superblock(struct super_block *sb)
{
    kdelete(sb->private_sb_info, sizeof(FAT32_sb_info_t));
    kdelete(sb->root->dir_inode->private_index_info, sizeof(FAT32_inode_info_t));
    kdelete(sb->root->dir_inode, sizeof(inode_t));
    kdelete(sb->root, sizeof(dir_entry_t));
    kdelete(sb, sizeof(super_t));
}

// 用于把inode文件对应的目录项写到硬盘中- 缓存目录项
void fat32_write_inode(struct index_node *inode)
{
    struct FAT32_Directory *fdentry = NULL;
    struct FAT32_Directory *buf = NULL;
    struct FAT32_inode_info *finode = inode->private_index_info;
    struct FAT32_sb_info *fsbi = inode->sb->private_sb_info;
    unsigned long sector = 0;
    if (finode->dentry_location == 0)
    {
        color_printk(RED, BLACK, "FS ERROR:write root inode!\n");
        return;
    }

    sector = fsbi->Data_firstsector + (finode->dentry_location - 2) * fsbi->sector_per_cluster;
    buf = (struct FAT32_Directory *)knew(fsbi->bytes_per_cluster, 0);
    memset(buf, 0, fsbi->bytes_per_cluster);
    IDE_device_operation.transfer(ATA_READ_CMD, sector, fsbi->sector_per_cluster, (unsigned char *)buf);
    fdentry = buf + finode->dentry_position;

    //// alert fat32 dentry data, 这里没有更新时间等信息
    fdentry->DIR_FileSize = inode->file_size;
    fdentry->DIR_FstClusLO = finode->first_cluster & 0xffff;
    fdentry->DIR_FstClusHI = (fdentry->DIR_FstClusHI & 0xf000) | (finode->first_cluster >> 16);
    IDE_device_operation.transfer(ATA_WRITE_CMD, sector, fsbi->sector_per_cluster, (unsigned char *)buf);

    kdelete(buf, sizeof(struct FAT32_Directory));
}

// 提供了操作超级块和写inode结点的方法
struct super_block_operations FAT32_sb_ops =
    {
        .write_superblock = fat32_write_superblock,
        .put_superblock = fat32_put_superblock,

        .write_inode = fat32_write_inode,
};

/**
 * @brief 为fat32文件系统编写的引导扇区解析方法, 主要工作是建立并初始化超级块
 *
 * @param DPTE MBR的分区表
 * @param buf fat32文件系统的引导扇区
 * @return struct super_block* 超级块结构体
 */
struct super_block *fat32_read_superblock(struct Disk_Partition_Table_Entry *DPTE, void *buf)
{
    struct super_block *sbp = NULL;
    struct FAT32_inode_info *finode = NULL;
    struct FAT32_BootSector *fbs = NULL;
    struct FAT32_sb_info *fsbi = NULL;

    // =============================== 建立 super block =====================================
    sbp = (struct super_block *)knew(sizeof(struct super_block), 0);
    memset(sbp, 0, sizeof(struct super_block));
    sbp->type = FS_TYPE_FAT32;
    sbp->sb_ops = &FAT32_sb_ops;
    sbp->private_sb_info = (struct FAT32_sb_info *)knew(sizeof(struct FAT32_sb_info), 0);
    memset(sbp->private_sb_info, 0, sizeof(struct FAT32_sb_info));

    // fat32 boot sector 根据FAT32引导扇区，记录一些数据提供给VFS使用
    fbs = (struct FAT32_BootSector *)buf;
    fsbi = sbp->private_sb_info;

    fsbi->start_sector = DPTE->start_LBA;
    fsbi->sector_count = DPTE->sectors_limit;
    fsbi->sector_per_cluster = fbs->BPB_SecPerClus;
    // 每簇的字节数 = 每簇的扇区数 * 每扇区的字节数
    // 簇是fat系统操作的最小单元
    fsbi->bytes_per_cluster = fbs->BPB_SecPerClus * fbs->BPB_BytesPerSec;
    fsbi->bytes_per_sector = fbs->BPB_BytesPerSec;
    // 第一个数据扇区 = 本文件系统的起始扇区 + fat32系统的保留扇区 + 文件系统的fat表数 * fat表的大小。
    fsbi->Data_firstsector = DPTE->start_LBA + fbs->BPB_RsvdSecCnt + fbs->BPB_NumFATs * fbs->BPB_FATSz32;
    // 第一个fat表在文件系统的保留扇区之后
    fsbi->FAT1_firstsector = DPTE->start_LBA + fbs->BPB_RsvdSecCnt;
    fsbi->sector_per_FAT = fbs->BPB_FATSz32;
    fsbi->NumFATs = fbs->BPB_NumFATs;
    fsbi->fsinfo_sector_infat = fbs->BPB_FSInfo;
    fsbi->bootsector_bk_infat = fbs->BPB_BkBootSec;

    #if ENABLE_FAT32_DEBUG
    // BPB - 引导扇区参数块 - BIOS Parameter Block ? - Boot Parameter Block
    color_printk(ORANGE, BLACK, "FAT32 Boot Sector\nBPB_FSInfo:%#018lx\tFAT1_firstsector:%#018lx\tBPB_TotSec32:%#018lx\n", fbs->BPB_FSInfo, fsbi->FAT1_firstsector, fbs->BPB_TotSec32);
    color_printk(ORANGE, BLACK, "BPB_SecPerClus:%#018lx\tBPB_NumFATs:%#018lx\tBPB_FATSz32:%#018lx\t\n", fbs->BPB_SecPerClus, fbs->BPB_NumFATs, fbs->BPB_FATSz32);
    color_printk(ORANGE, BLACK, "BPB_firstsector:%#018lx\tBPB_RootClus:%#018lx\tBPB_BytesPerSec:%#018lx\n", fsbi->Data_firstsector, fbs->BPB_RootClus, fbs->BPB_BytesPerSec);
    
    unsigned int bbuf[128], i;
    IDE_device_operation.transfer(ATA_READ_CMD, fsbi->FAT1_firstsector, 1, (unsigned char *)bbuf);
    for (i = 0; i < 128; i++)
    {
        color_printk(ORANGE, BLACK, "%lx    ", bbuf[i]);
        if (i && i % 16 == 0)
            color_printk(ORANGE, BLACK, "\n");
    }

    #endif
    
    // fat32 fsinfo sector
    fsbi->fat_fsinfo = (struct FAT32_FSInfo *)knew(sizeof(struct FAT32_FSInfo), 0);
    memset(fsbi->fat_fsinfo, 0, sizeof(struct FAT32_FSInfo));
    IDE_device_operation.transfer(ATA_READ_CMD, DPTE->start_LBA + fbs->BPB_FSInfo, 1, (unsigned char *)fsbi->fat_fsinfo);
    color_printk(ORANGE, BLACK, "FAT32 FSInfo\nFSI_LeadSig:%#018lx\tFSI_StrucSig:%#018lx\tFSI_Free_Count:%#018lx\n", fsbi->fat_fsinfo->FSI_LeadSig, fsbi->fat_fsinfo->FSI_StrucSig, fsbi->fat_fsinfo->FSI_Free_Count);

    // ================================== 创建根目录 =====================================
    // directory entry
    sbp->root = (struct dir_entry *)knew(sizeof(dir_entry_t), 0);

    sbp->root->parent = sbp->root; // 根目录的父目录是自己
    sbp->root->dir_ops = &FAT32_dentry_ops;
    sbp->root->name = (char *)knew(2, 0);
    sbp->root->name[0] = '/';
    sbp->root->name_length = 1;
    sbp->root->d_sb = sbp;

    // 根目录的 index node
    sbp->root->dir_inode = (struct index_node *)knew(sizeof(struct index_node), 0);
    memset(sbp->root->dir_inode, 0, sizeof(struct index_node));
    sbp->root->dir_inode->inode_ops = &FAT32_inode_ops;
    sbp->root->dir_inode->f_ops = &FAT32_file_ops;
    sbp->root->dir_inode->file_size = 0;
    sbp->root->dir_inode->blocks = (sbp->root->dir_inode->file_size + fsbi->bytes_per_cluster - 1) / fsbi->bytes_per_sector;
    sbp->root->dir_inode->attribute = FS_ATTR_DIR;
    sbp->root->dir_inode->sb = sbp;
    list_init(&sbp->root->child_node);
    list_init(&sbp->root->subdirs_list);
    
    // fat32 root inode's private_index_info成员
    sbp->root->dir_inode->private_index_info = (struct FAT32_inode_info *)knew(sizeof(struct FAT32_inode_info), 0);
    memset(sbp->root->dir_inode->private_index_info, 0, sizeof(struct FAT32_inode_info));
    finode = (struct FAT32_inode_info *)sbp->root->dir_inode->private_index_info;
    finode->first_cluster = fbs->BPB_RootClus;
    finode->dentry_location = 0;
    finode->dentry_position = 0;
    finode->create_date = 0;
    finode->create_time = 0;
    finode->write_date = 0;
    finode->write_time = 0;
    
    sbp->s_flags = false; // 标记挂载

    return sbp;
}

struct file_system_type FAT32_fs_type =
    {
        .name = "FAT32",
        .fs_flags = 0,
        .read_superblock = fat32_read_superblock,
        .next = NULL,
};


