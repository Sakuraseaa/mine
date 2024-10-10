#ifndef __BLOCK_H__
#define __BLOCK_H__

// 驱动程序采用的是IDE设备接口的PIO操作模式
typedef struct block_device_operationa
{
    s64_t (*open)();
    s64_t (*close)();
    s64_t (*ioctl)(s64_t cmd, s64_t arg);
    s64_t (*transfer)(s64_t cmd, u64_t blocks, s64_t count, u8_t *buffer);
}block_dev_opt_t;

#endif