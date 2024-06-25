#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "types.h"

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

typedef struct device_t
{
    char name[NAMELEN];  // 设备名
    int type;            // 设备类型
    int subtype;         // 设备子类型
    dev_t dev;           // 设备号
    dev_t parent;        // 父设备号
    void *ptr;           // 设备指针

    void *device_ops;    // 设备操作指针
} device_t;

// 安装设备
dev_t device_install(
    int type, int subtype,
    void *ptr, char *name, dev_t parent,
    void *ops);

// 根据子类型查找设备
device_t *device_find(int type, idx_t idx);

// 根据设备号查找设备
device_t *device_get(dev_t dev);

// 控制设备
int device_ioctl(dev_t dev, int cmd, void *args, int flags);

// 读设备
int device_read(dev_t dev, void *buf, size_t count, idx_t lba, int flags);

// 写设备
int device_write(dev_t dev, void *buf, size_t count, idx_t lba, int flags);
void device_init();
#endif