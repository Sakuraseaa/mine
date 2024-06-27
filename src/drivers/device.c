#include "device.h"
#include "assert.h"
#include "debug.h"
#include "errno.h"
#include "block.h"
#include "disk.h"
#include "types.h"


#define LOGK(fmt, args...) DEBUGK(fmt, ##args)
static device_t devices[DEVICE_NR]; // 设备数组

// 获取空设备
static device_t *get_null_device()
{
    for (size_t i = 1; i < DEVICE_NR; i++)
    {
        device_t *device = &devices[i];
        if (device->type == DEV_NULL)
            return device;
    }
    return NULL;
}

int device_ioctl(dev_t dev, int cmd, void *args, int flags)
{
    device_t *device = device_get(dev);
    if (device->type == DEV_BLOCK && device->subtype == DEV_IDE_PART)
    {
        block_dev_opt_t* bdo = (block_dev_opt_t*)device->device_ops;
        return  bdo->ioctl(GET_IDENTIFY_DISK_CMD, args);
    }
    LOGK("ioctl of device %d not implemented!!!\n", dev);
    return -ENOSYS;
}

/**
 * @brief 
 * 
 * @param dev 
 * @param buf 缓冲区
 * @param count 需要读的扇区数
 * @param idx 硬盘逻辑块号 LBA
 * @param flags  用这个参数来 来区分 minix 和 fat32 ?
 * @return int 
 */
int device_read(dev_t dev, void *buf, size_t count, idx_t lba, int flags)
{
    device_t *device = device_get(dev);
    if (device->type == DEV_BLOCK && device->subtype == DEV_IDE_PART) {
        block_dev_opt_t* bdo = (block_dev_opt_t*)device->device_ops;
        return  bdo->transfer(ATA_READ_CMD, lba, count, buf);
    }else if(1) {
    }
    LOGK("read of device %d not implemented!!!\n", dev);
    return -ENOSYS;
}

int device_write(dev_t dev, void *buf, size_t count, idx_t lba, int flags)
{
    device_t *device = device_get(dev);
    if (device->type == DEV_BLOCK && device->subtype == DEV_IDE_PART) {
        block_dev_opt_t* bdo = (block_dev_opt_t*)device->device_ops;
        return  bdo->transfer(ATA_WRITE_CMD, lba, count, buf);
    }else if(1) {
    }
    LOGK("write of device %d not implemented!!!\n", dev);
    return -ENOSYS;
}

// 安装设备
dev_t device_install(
    int type, int subtype,
    void *ptr, char *name, dev_t parent,
    void *ops)
{
    device_t *device = get_null_device();
    device->ptr = ptr;
    device->parent = parent;
    device->type = type;
    device->subtype = subtype;
    strncpy(device->name, name, NAMELEN);
    device->device_ops = ops;
    return device->dev;
}

void device_init()
{
    for (size_t i = 0; i < DEVICE_NR; i++)
    {
        device_t *device = &devices[i];
        strcpy((char *)device->name, "null");
        device->type = DEV_NULL;
        device->subtype = DEV_NULL;
        device->dev = i;
        device->parent = 0;
        device->device_ops = NULL;
    }
}

device_t *device_find(int subtype, idx_t idx)
{
    idx_t nr = 0;
    for (size_t i = 0; i < DEVICE_NR; i++)
    {
        device_t *device = &devices[i];
        if (device->subtype != subtype)
            continue;
        if (nr == idx)
            return device;
        nr++;
    }
    return NULL;
}

device_t *device_get(dev_t dev)
{
    assert(dev < DEVICE_NR);
    device_t *device = &devices[dev];
    assert(device->type != DEV_NULL);
    return device;
}

