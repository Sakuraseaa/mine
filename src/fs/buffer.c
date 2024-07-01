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

extern struct super_block *root_sb; 

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define BUFFER_DESC_NR 4 // 512 1024 2048 4096

static bdesc_t bdescs[BUFFER_DESC_NR];
#define hash(dev, block) ( ((dev)^(block)) % HASH_COUNT )

static bdesc_t *get_desc(size_t sz) {

    assert(sz >= 0);

    for(u8 i = 0; i < BUFFER_DESC_NR; i++)
        if(bdescs[i].size == sz)
            return &bdescs[i];
    
    // Run here. 证明程序出错了
    
    return NULL;
}

// 把 缓冲块 放入哈希表
static void hash_locate(bdesc_t* desc, buffer_t* buf) {
    
    u64 idx = hash(buf->dev, buf->block);
    list_t* list = &desc->hash_table[idx];

    assert(!list_search(list, &buf->hnode));

    list_add_to_behind(list, &buf->hnode);
}

static void hash_remove(bdesc_t *desc, buffer_t* buf) {

    u64 idx = hash(buf->dev, buf->block);
    list_t* list = &desc->hash_table[idx];

    // 有 才 删除
    if(list_search(list, &buf->hnode))
        list_del(&buf->hnode);
}

static buffer_t *get_from_hash_table(bdesc_t *desc, dev_t dev, idx_t block) {
    
    u64 idx = hash(dev, block);
    list_t* list = &desc->hash_table[idx];

    buffer_t *buf = NULL;
    while(!list_is_empty(list)) {
        
        buffer_t* ptr = container_of(list_next(list), buffer_t, hnode);
        
        if(ptr->dev == dev && ptr->block == block) {
            buf = ptr;
            break;
        }

        list = list->next;
    }
    
    if(buf == NULL)
        return NULL;

    // 这种情况出现于: 缓冲块被释放后，缓冲块加入到了idle_list, 但没有从哈希表链中移除.
    // 此时该缓冲块又被命中，所以此处尝试从idle_list中移除 rnode
    // 如果 buf 在空闲列表中，则移除
    if(list_search(&desc->idle_list, &buf->rnode)) 
        list_del(&buf->rnode);

    return buf;
}

static err_t buffer_alloc(bdesc_t *desc) {
    // here allocated memory is too small, i think 16KB for suitable
    char* addr = (char*)kmalloc(4096, 0); 
    
    buffer_t* buf = NULL;
    for(char* i = addr; i < addr + 4096; i += desc->size) { 
        
        buf = (buffer_t*)kmalloc(sizeof(buffer_t), 0);
        
        buf->data = i;
        buf->block = 0;
        buf->desc = desc;
        buf->dirty = false;
        buf->valid = false;
        buf->refer_count = 0;
        buf->dev = 0;
        
        list_init(&buf->hnode);
        list_init(&buf->rnode);
        
        semaphore_init(&buf->lock, 1);

        list_add_to_behind(&desc->free_list, &buf->rnode);

        desc->count++; // 增加空闲块计数
    }

    LOGK("buffer desciptor update:: size %d count %d\n", desc->size, desc->count);
    return 0;
}

/* get free buffer_t */
static buffer_t* get_free_buffer(bdesc_t* desc) {

    /* free_list 只由malloc能生产 */
    if(desc->count < MAX_BUF_COUNT && list_is_empty(&desc->free_list)) {
        buffer_alloc(desc);
    }

    if(!list_is_empty(&desc->free_list)) {
    // free_list 只由此处消费
        buffer_t* buf = container_of(list_prev(&desc->free_list), buffer_t, rnode);
        hash_remove(desc, buf);
        buf->valid = false;
        return buf;
    }

    // 执行到此处说明：没有空闲缓冲块了，不能再申请空闲缓冲块了
    // 只能等待 有缓冲块释放 才去使用
    while (list_is_empty(&desc->idle_list)) {
        sleep_on(&desc->wait_list);
    }

    assert(!list_is_empty(&desc->idle_list));
    buffer_t* buf = container_of(list_prev(&desc->idle_list), buffer_t, rnode);
    hash_remove(desc, buf);
    buf->valid = false;
    return buf;

}

static buffer_t *getblk(bdesc_t* desc, dev_t dev, idx_t block) {
    
    buffer_t* buf = get_from_hash_table(desc, dev, block);
    if(buf) {
        buf->refer_count++;
        return buf;
    }

    buf = get_free_buffer(desc);
    assert(buf->refer_count == 0);
    assert(buf->dirty == 0);

    buf->refer_count = 1;
    buf->dev = dev;
    buf->block = block;
    hash_locate(desc, buf); // 只有此处才会把 buf 放入哈希表链中
    
    return buf;
}

// 写缓冲
err_t bwrite(buffer_t *buf){}
// 释放缓冲
err_t brelse(buffer_t *buf) {}



// 缓冲读
buffer_t *bread(unsigned long dev, unsigned long block, unsigned long size) {

    bdesc_t* m_desc = get_desc(size);

    buffer_t* buf = getblk(m_desc, dev, block);

    assert(buf != NULL);
    if(buf->valid) 
        return buf;
    semaphore_down(&buf->lock);
        
    u64 block_size = m_desc->size; // 缓冲块大小
    u64 sector_size =  1024;   //设备扇区大小
    u64 bs = block_size / sector_size; // 读取的块数


    device_read(dev, buf->data, bs, block * bs, 0);
    buf->valid = true;
    buf->dirty = false;
        
    semaphore_up(&buf->lock);
    return buf;

rollback:
    semaphore_up(&buf->lock);
    brelse(buf);
    return NULL;
}

void buffer_init(void) {
    LOGK("buffer_t size is %d\n", sizeof(buffer_t));

    size_t sz = 512;
    for(size_t i = 0; i < BUFFER_DESC_NR; i++, sz <<= 1) {
        bdesc_t* desc = &bdescs[i];

        desc->count = 0;
        desc->size = sz;

        list_init(&desc->free_list);
        list_init(&desc->idle_list);
        list_init(&desc->wait_list);

        for (size_t i = 0; i < HASH_COUNT; i++)
        {
            list_init(&desc->hash_table[i]);
        }
    }
}
