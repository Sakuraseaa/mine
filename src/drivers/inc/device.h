#ifndef __DEVICE_H__
#define __DEVICE_H__

#define DEVICE_NR 64 // 设备数量
#define NAMELEN 16

// 设备类型
enum device_type_t
{
    DEV_NULL,  // 空设备
    DEV_CHAR,  // 字符设备
    DEV_BLOCK, // 块设备
    DEV_NET,   // 网络设备
};

// 设备子类型
enum device_subtype_t
{
    DEV_CONSOLE = 1, // 控制台
    DEV_KEYBOARD,    // 键盘
    DEV_SERIAL,      // 串口
    DEV_TTY,         // TTY 设备
    DEV_SB16,        // 声霸卡
    DEV_IDE_DISK,    // IDE 磁盘
    DEV_IDE_PART,    // IDE 磁盘分区
    DEV_IDE_CD,      // IDE 光盘
    DEV_RAMDISK,     // 虚拟磁盘
    DEV_FLOPPY,      // 软盘
    DEV_NETIF,       // 网卡
};

// typedef struct device_t
// {
//     char_t name[NAMELEN];  // 设备名
//     s32_t type;            // 设备类型
//     s32_t subtype;         // 设备子类型
//     dev_t dev;           // 设备号
//     dev_t parent;        // 父设备号
//     void *ptr;           // 设备指针

//     void *device_ops;    // 设备操作指针
// } device_t;

#endif