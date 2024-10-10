#ifndef _DEVICE_F_H_
#define _DEVICE_F_H_

// 安装设备
dev_t device_install(
    s32_t type, s32_t subtype,
    void *ptr, char *name, dev_t parent,
    void *ops);

// 根据子类型查找设备
device_t *device_find(s32_t type, idx_t idx);

// 根据设备号查找设备
device_t *device_get(dev_t dev);

// 控制设备
s32_t device_ioctl(dev_t dev, s32_t cmd, void *args, s32_t flags);

// 读设备
s32_t device_read(dev_t dev, void *buf, size_t count, idx_t lba, s32_t flags);

// 写设备
s32_t device_write(dev_t dev, void *buf, size_t count, idx_t lba, s32_t flags);
void device_init();

#endif // _DEVICE_F_H_