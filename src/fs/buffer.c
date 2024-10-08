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
    if(!list_is_empty(list)) {
        for(list_t* node = list->next; node != list; node = node->next) {
            buffer_t* ptr = container_of(node, buffer_t, hnode);
        
            if(ptr->dev == dev && ptr->block == block) {
                buf = ptr;
                break;
            }
        }
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
    u8* addr = (u8*)knew(PAGE_4K_SIZE, 0); 
    
    buffer_t* buf = NULL;
    for(u8* i = addr; i < addr + 4096; i += desc->size) { 
        
        buf = (buffer_t*)knew(sizeof(buffer_t), 0);
        
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

    list_t* free_node = NULL;
    /* free_list 只由malloc能生产 */
    if(desc->count < MAX_BUF_COUNT && list_is_empty(&desc->free_list)) {
        buffer_alloc(desc);
    }

    if(!list_is_empty(&desc->free_list)) {
    // free_list 只由此处消费
        free_node = list_prev(&desc->free_list);
        buffer_t* buf = container_of(free_node, buffer_t, rnode);
        list_del(free_node);
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
    free_node = list_prev(&desc->idle_list);
    buffer_t* buf = container_of(free_node, buffer_t, rnode);
    hash_remove(desc, buf);
    list_del(free_node);
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
err_t bwrite(buffer_t *buf){

    if (!buf->dirty)
        return EOK;

    bdesc_t *desc = buf->desc;

    u64 block_size = desc->size;
    u64 sector_size =  512;   //设备扇区大小, 此处应该改进为从设备获取，该设备的扇区大小
    u64 bs = block_size / sector_size; // 读取的块数
    int ret = device_write(buf->dev, buf->data, bs, buf->block * bs, 0);

    buf->dirty = false;
    buf->valid = true;
    return ret;
}

// 释放缓冲
err_t brelse(buffer_t *buf) {
    
    if(buf == NULL) return -1;

    int ret = bwrite(buf); // 我觉得此处同步有点勉强


    buf->refer_count--;
    if(buf->refer_count) 
        return ret;

    // 该节点没有连接在free_list链表上
    assert(list_search(&buf->desc->free_list, &buf->rnode) == 0);

    bdesc_t* desc = buf->desc;
    list_add_to_behind(&desc->idle_list, &buf->rnode); // 此处可没卸载 哈希表上挂的节点

    if(!wait_queue_is_empty(&desc->wait_list))
        wakeup(&desc->wait_list, TASK_UNINTERRUPTIBLE);
    
    return 0;
}

// 缓冲读
buffer_t *bread(unsigned long dev, unsigned long block, unsigned long size) {

    bdesc_t* m_desc = get_desc(size);

    buffer_t* buf = getblk(m_desc, dev, block);

    assert(buf != NULL);
    if(buf->valid) 
        return buf;
    semaphore_down(&buf->lock);
        
    u64 block_size = m_desc->size; // 缓冲块大小
    u64 sector_size =  512;   //设备扇区大小, 此处应该改进为从设备获取，该设备的扇区大小
    u64 bs = block_size / sector_size; // 读取的块数


    device_read(dev, buf->data, bs, block * bs, 0);
    buf->valid = true;
    buf->dirty = false;
        
    semaphore_up(&buf->lock);
    return buf;
}

/**
 * @brief 把高速缓冲区和硬盘进行同步
 *      （比较耗时，3重循环）
 */
void sync(void) {
    
    list_t* head = NULL, *element = NULL;
    size_t i = 0, j = 0;
    buffer_t* buf = NULL;

    for(i = 0; i < BUFFER_DESC_NR; i++) { // 遍历缓冲池
        if(bdescs[i].count == 0)
            continue;

        for(j = 0; j < HASH_COUNT; j++) {  // 遍历缓冲池中的哈希表
            head = &bdescs[i].hash_table[j];
            if(list_is_empty(head))
                continue;

            for(element = head->next; element != head; element = element->next){ // 遍历哈希表链
                buf = container_of(element, buffer_t, hnode);
                bwrite(buf);
            }
        
        }
    }
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
        wait_queue_init(&desc->wait_list, NULL);

        for (size_t i = 0; i < HASH_COUNT; i++)
        {
            list_init(&desc->hash_table[i]);
        }
    }
}
