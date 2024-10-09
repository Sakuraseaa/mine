#ifndef _DEVICE_F_H_
#define _DEVICE_F_H_

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

#endif // _DEVICE_F_H_