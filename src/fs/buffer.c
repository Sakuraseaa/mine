/* 
    2024-6-22 16:21
    虽然我们是软件工程专业的，但我的架构能力并不好，我并不知道应该把告诉缓冲区设置在操作系统的那个层才合理
    我应该阅读更多的源码，学习设计模式？
    我目前编码能力不足，所以暂时停止了高速缓冲区的编写
    2024-6-26 23:49
    高速缓冲区应该在文件系统之下，虚拟设备层之上
*/
#include "buffer.h"
#include "memory.h"
#include "VFS.h"
#include "lib.h"
#include "types.h"
#include "device.h"
#include "debug.h"
#include "assert.h"
#include "semaphore.h"
#include "waitqueue.h"
#include "errno.h"
// 这个变量应该修改成为一个数组，来记录系统挂载了的文件系统
extern struct super_block *root_sb; 



#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define BUFFER_DESC_NR 4 // 描述符数量 512 1024 2048 4096

typedef struct block_buf{
    
    u64 count;                          // 缓冲块数量
    u64 size;                           // 缓冲块尺寸

    list_t free_list;                    // 空闲缓冲块队列
    list_t idle_list;                    // 暂存链表，被释放的块
    wait_queue_T wait_list;              // 等待缓冲块的进程队列
    list_t hash_table[HASH_COUNT];       // 暂存哈希表
}bdesc_t;

typedef struct _buffer_
{
    char *data;            // 数据区 
    bdesc_t *desc;        // 描述符指针
    dev_t dev;            // 设备号
    idx_t block;          // 逻辑块号
    int refer_count;      // 缓冲块被引用的数量
    semaphore_T lock;     // 缓冲区锁
    bool dirty;           // 是否与磁盘不一致
    bool valid;           // 是否有效
    list_t hnode;         // 哈希表拉链节点
    list_t rnode;         // 缓冲节点
} buffer_t;

static bdesc_t bdescs[BUFFER_DESC_NR];
#define hash(dev, block) ( ((dev)^(block)) % HASH_COUNT )


// 从哈希表中查找 buffer
static buffer_t *get_from_hash_table(bdesc_t *desc, dev_t dev, idx_t block) {
    u64 idx = hash(dev, block)
    list_t list = &desc->hash_table[idx];

    buffer_t *buf = NULL;
    for () {

    }
}

static err_t buffer_alloc(bdesc_t *desc) {
    // here allocated memory is too small, i think 16KB for suitable
    char* addr = (char*)kmalloc(4096, 0); // 申请一块大内存
    
    buffer_t* buf = NULL;
    for(char* i = addr; i < addr + 4096; i += desc->size) { // 划分为小内存
        buf = (buffer_t*)kmalloc(sizeof(buffer_t));
        
        buf->data = i;
        buf->block = 0;
        buf->desc = desc;
        buf->dirty = false;
        buf->valid = false;
        buf->refer_count = 0;
        buf->dev = 0;
        // 一元信号量就是锁
        semaphore_init(&buf->lock, 1);

        list_add_to_behind(&desc->free_list, &buf->rnode);

        desc->count++;
    }

    LOGK("buffer desciptor update:: size %d count %d\n", desc->size, desc->count);
    return
}

static bdesc_t *get_desc(size_t sz) {

    assert(sz >= 0);

    for(u8 i = 0; i < BUFFER_DESC_NR; i++)
        if(bdescs[i].size == sz)
            return &bdescs[i];
    
    panic("buffer_size %d has no buffer\n", sz);
    
    return NULL;
}

// get free buffer_t
static buffer_t* get_free_buffer(bdesc_t* desc) {

    if(desc->count < MAX_NR_ZONES && list_is_empty(desc->free_list)) {
        buffer_alloc(desc);
    }

}

// 获得设备 dev，第 block 对应的缓冲
static buffer_t *getblk(bdesc_t* desc, dev_t dev, idx_t block) {
    
    buffer_t* buf = get_from_hash_table(desc, dev, block);
    if(buf) {
        return buf;
    }

    buf = get_free_buffer(desc);

}

buffer_t *bread(unsigned long dev, unsigned long block, unsigned long size) {

        bdesc_t* m_desc = get_desc(size);

        buffer_t* buf = getblk(m_desc, dev, block);

        if(buf->valid) {
            return buf;
        }

        // device_read(dev, 0, bs, block, 0);
}


// 为块设备创建缓冲区
void buffer_init(void) {
    LOGK("buffer_t size is %d\n", sizeof(buffer_t));

    size_t sz = 512;
    for(size_t i = 0; i < BUFFER_DESC_NR; i++, sz <<= 1) {
        bdesc_t* desc = &bdescs[i];

        // 内存池为空
        desc->count = 0;
        desc->size = sz;

        // 初始化空闲链表
        list_init(&desc->free_list);
        // 初始化缓冲链表
        list_init(&desc->idle_list);
        // 初始化等待进程链表
        list_init(&desc->wait_list);

        // 初始化哈希表
        for (size_t i = 0; i < HASH_COUNT; i++)
        {
            list_init(&desc->hash_table[i]);
        }
    }
}
