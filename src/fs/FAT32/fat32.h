#ifndef __FAT32_H__
#define __FAT32_H__

// BPB = BIOS Parameter Block
// FAT32文件系统的引导扇区
typedef struct FAT32_BootSector
{
    unsigned char BS_jmpBoot[3];    // 跳转指令
    unsigned char BS_OEMName[8];    // 生产厂商名
    unsigned short BPB_BytesPerSec; // 每扇区字节数
    unsigned char BPB_SecPerClus;   // 每簇扇区数
    unsigned short BPB_RsvdSecCnt;  // 保留扇区数
    unsigned char BPB_NumFATs;      // 共有多少FAT表
    unsigned short BPB_RootEntCnt;  // 根目录文件数最大值
    unsigned short BPB_TotSec16;    // 16位扇区总数
    unsigned char BPB_Media;        // 介质描述符
    unsigned short BPB_FATSz16;     // FAT12/16每FAT扇区数
    unsigned short BPB_SecPerTrk;   // 每磁道扇区数
    unsigned short BPB_NumHeads;    // 磁头数
    unsigned int BPB_HiddSec;       // 隐藏扇区数
    unsigned int BPB_TotSec32;      // 32位扇区总数
    unsigned int BPB_FATSz32;       // FAT32每FAT扇区数
    unsigned short BPB_ExtFlags;    // 扩展标志
    unsigned short BPB_FSVer;       // FAT32文件系统版本号
    unsigned int BPB_RootClus;      // 根目录起始簇号
    unsigned short BPB_FSInfo;      // FSInfo结构体的扇区号
    unsigned short BPB_BkBootSec;   // 引导扇区的备份扇区号
    unsigned char BPB_Reserved[12]; // 保留使用
    unsigned char BS_DrvNum;        // int 13h的驱动器号
    unsigned char BS_Reserved1;     // 未使用
    unsigned char BS_BootSig;       // 扩展引导标记
    unsigned int BS_VolID;          // 卷序列号
    unsigned char BS_VolLab[11];    // 卷标
    unsigned char BS_FilSysType[8]; // 文件系统类型

    unsigned char BootCode[420]; // 引导代码
    unsigned short BS_TraiSig;   // 结束标记
} __attribute__((packed)) FAT32_bootsector_t;

// FAT32文件系统的FSInfo扇区, 主要的是FSI_Free_Count 和 FSI_Nxt_Free成员，
typedef struct FAT32_FSInfo
{
    unsigned int FSI_LeadSig;         // FSInfo扇区标识符
    unsigned char FSI_Reserved1[480]; // 保留使用
    unsigned int FSI_StrucSig;        // 第二个标识符-0x61417272
    unsigned int FSI_Free_Count;      // 上一次记录的空闲簇的数量，参考值不精确
    unsigned int FSI_Nxt_Free;        // 空闲簇的起始搜索位置，参考值不精确
    unsigned char Reserved2[12];      // 保留使用，全置为0
    unsigned int FSI_TrailSig;        // 结束标志
} __attribute__((packed)) FAT32_fsinfo_t;

// fat32文件系统超级块信息结构体,描述出一个FAT32文件系统元数据
typedef struct FAT32_sb_info
{
    unsigned long start_sector; // 文件系统起始扇区
    unsigned long sector_count; // 文件系统扇区总数

    long sector_per_cluster; // 每簇多少扇区
    long bytes_per_cluster;  // 每簇多少字节
    long bytes_per_sector;   // 每扇区多少字节

    unsigned long Data_firstsector; // 数据区起始扇区
    unsigned long FAT1_firstsector; // FAT1起始扇区
    unsigned long sector_per_FAT;   // FAT占用扇区数
    unsigned long NumFATs;          // FAT数 （file alloction table 文件分配表）

    unsigned long fsinfo_sector_infat; // FSinfo结构体的扇区号
    unsigned long bootsector_bk_infat; // 引导扇区的备份扇区号

    struct FAT32_FSInfo *fat_fsinfo;
}FAT32_sb_info_t;
// 为 VFS系统 提供建立fat32文件系统的数据
typedef struct FAT32_inode_info
{
    unsigned long first_cluster;   // 本文件在fat32系统中的起始簇号
    unsigned long dentry_location; // dentry struct in cluster(0 is root, 1 is invalid),本文件的目录项在文件系统中对应的簇号
    unsigned long dentry_position; // dentry struct offset in cluster,本文件目录项在文件系统对应的簇中偏移

    unsigned short create_date; // 创建日期
    unsigned short create_time; // 创建时间
    unsigned short write_date;  // 创建日期
    unsigned short write_time;  // 创建时间
}FAT32_inode_info_t;

#define ATTR_READ_ONLY (1 << 0) // 只读
#define ATTR_HIDDEN (1 << 1)    // 隐藏
#define ATTR_SYSTEM (1 << 2)    // 系统文件
#define ATTR_VOLUME_ID (1 << 3) // 卷标
#define ATTR_DIRECTORY (1 << 4) // 目录
#define ATTR_ARCHIVE (1 << 5)   // 存档
// 长文件名
#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

// 目录项
struct FAT32_Directory
{
    unsigned char DIR_Name[11];     // 文件名, DIR_NAME[0]为0xE5, 0x00,0x05时，表明此目录项为无效目录项和空闲目录项
    unsigned char DIR_Attr;         // 文件属性
    unsigned char DIR_NTRes;        // 保留使用,EXT|BASE => 8(BASE).3(EXT)
                                    // BASE:LowerCase(8),UpperCase(0)
                                    // EXT:LowerCase(16),UpperCase(0)
    unsigned char DIR_CrtTimeTenth; // 文件创建的毫秒级时间戳
    unsigned short DIR_CrtTime;     // 文件创建时间
    unsigned short DIR_CrtDate;     // 文件创建日期
    unsigned short DIR_LastAccDate; // 最后访问日期
    unsigned short DIR_FstClusHI;   // 起始簇号(高字)
    unsigned short DIR_WrtTime;     // 最后写入时间
    unsigned short DIR_WrtDate;     // 最后写入日期
    unsigned short DIR_FstClusLO;   // 起始簇号(低字)
    unsigned int DIR_FileSize;      // 文件大小
} __attribute__((packed));

#define LOWERCASE_BASE (8)
#define LOWERCASE_EXT (16)

// 长目录项
struct FAT32_LongDirectory
{
    unsigned char LDIR_Ord;        // 长目录项序号
    unsigned short LDIR_Name1[5];  // 长文件名的第1 ~ 5字符，每个字符占2B
    unsigned char LDIR_Attr;       // 文件属性，必须等于ATTR_LONG_NAME
    unsigned char LDIR_Type;       // 如果为0，则说明是长目录项的子项
    unsigned char LDIR_Chksum;     // 短文件名的校验和
    unsigned short LDIR_Name2[6];  // 长文件名的第6 - 11个字符
    unsigned short LDIR_FstClusLO; // 必须为0
    unsigned short LDIR_Name3[2];  // 长文件名的第12 ~ 13 个字符
} __attribute__((packed));

typedef struct FAT32_Directory FAT32_directory_t;
typedef struct FAT32_LongDirectory FAT32_longdirectory_t;

extern struct index_node_operations FAT32_inode_ops;
extern struct file_operations FAT32_file_ops;
extern struct dir_entry_operations FAT32_dentry_ops;
extern struct super_block_operations FAT32_sb_ops;

#endif
