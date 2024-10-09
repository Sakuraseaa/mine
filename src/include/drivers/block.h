#ifndef __BLOCK_H__
#define __BLOCK_H__

// 驱动程序采用的是IDE设备接口的PIO操作模式
typedef struct block_device_operation
{
    long (*open)();
    long (*close)();
    long (*ioctl)(long cmd, long arg);
    long (*transfer)(long cmd, u64_t blocks, long count, unsigned char *buffer);
}block_dev_opt_t;

#endif