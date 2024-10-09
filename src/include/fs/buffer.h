#ifndef __BUFFER_H__
#define __BUFFER_H__
#include "memory.h"
#include "lib.h"
#include "waitqueue.h"
#include "semaphore.h"

#define HASH_COUNT 63      //
#define MAX_BUF_COUNT 4096

typedef struct block_buf{
    
    u64_t count;                     // 缓冲池中 缓冲块的数量
    u64_t size;                     

    list_t free_list;                   // 已经申请但没使用的块
    list_t idle_list;                   // 使用过，但被释放的块
    wait_queue_t wait_list;             // 等待进程队列
    list_t hash_table[HASH_COUNT];      // 缓冲哈希表
}bdesc_t;

typedef struct buffer
{
    u8_t *data;          // 数据区
    bdesc_t *desc;       // 描述符指针
    dev_t dev;           // 设备号
    idx_t block;         // 块号
    int refer_count;     // 引用计数
    semaphore_t lock;    // 锁
    bool dirty;          // 是否与磁盘不一致
    bool valid;          // 缓冲数据是否有效
    list_t hnode;        // 哈希表拉链节点
    list_t rnode;        // 缓冲节点
} buffer_t;



err_t bwrite(buffer_t *buf);
// 释放缓冲
err_t brelse(buffer_t *buf);

// 缓冲读
buffer_t *bread(unsigned long dev, unsigned long block, unsigned long size);



void buffer_init(void);
#endif