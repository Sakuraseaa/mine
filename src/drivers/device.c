#include "fskit.h"
#include "devkit.h"

static Device_t devices[DEVICE_NR]; // 设备数组

// 获取空设备
static Device_t *get_null_device()
{
    for (size_t i = 1; i < DEVICE_NR; i++)
    {
        Device_t *device = &devices[i];
        if (device->type == DEV_NULL)
            return device;
    }
    return nullptr;
}

s32_t device_ioctl(dev_t dev, s32_t cmd, void *args, s32_t flags)
{
    Device_t *device = device_get(dev);
    if (device->type == DEV_BLOCK && device->subtype == DEV_IDE_PART)
    {
        block_dev_opt_t* bdo = (block_dev_opt_t*)device->device_ops;
        return  bdo->ioctl(GET_IDENTIFY_DISK_CMD, 0);
    }
    FAILK("ioctl of device %d not implemented!!!\n", dev);
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
 * @return s32_t 
 */
s32_t device_read(dev_t dev, void *buf, size_t count, idx_t lba, s32_t flags)
{
    Device_t *device = device_get(dev);
    if (device->type == DEV_BLOCK && device->subtype == DEV_IDE_PART) {
        block_dev_opt_t* bdo = (block_dev_opt_t*)device->device_ops;
        ide_part_t* ipt = (ide_part_t*)device->ptr;
        return  bdo->transfer(ATA_READ_CMD, ipt->start + lba, count, buf);
    }else if(1) {
    }
    FAILK("read of device %d not implemented!!!\n", dev);
    return -ENOSYS;
}

s32_t device_write(dev_t dev, void *buf, size_t count, idx_t lba, s32_t flags)
{
    Device_t *device = device_get(dev);
    if (device->type == DEV_BLOCK && device->subtype == DEV_IDE_PART) {
        block_dev_opt_t* bdo = (block_dev_opt_t*)device->device_ops;
        ide_part_t* ipt = (ide_part_t*)device->ptr;
        return  bdo->transfer(ATA_WRITE_CMD, ipt->start + lba, count, buf);
    }else if(1) {
    }
    FAILK("write of device %d not implemented!!!\n", dev);
    return -ENOSYS;
}

// 安装设备
dev_t device_install(
    s32_t type, s32_t subtype,
    void *ptr, str_t name, dev_t parent,
    void *ops)
{
    Device_t *device = get_null_device();
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
        Device_t *device = &devices[i];
        strcpy((str_t)device->name, "null");
        device->type = DEV_NULL;
        device->subtype = DEV_NULL;
        device->dev = i;
        device->parent = 0;
        device->device_ops = nullptr;
    }
}

Device_t *device_find(s32_t subtype, idx_t idx)
{
    idx_t nr = 0;
    for (size_t i = 0; i < DEVICE_NR; i++)
    {
        Device_t *device = &devices[i];
        if (device->subtype != subtype)
            continue;
        if (nr == idx)
            return device;
        nr++;
    }
    return nullptr;
}

Device_t *device_get(dev_t dev)
{
    assert(dev < DEVICE_NR);
    Device_t *device = &devices[dev];
    assert(device->type != DEV_NULL);
    return device;
}

