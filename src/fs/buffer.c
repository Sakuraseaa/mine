#include "buffer.h"
#include "memory.h"
#include "VFS.h"
#include "lib.h"
// 这个变量应该修改成为一个数组，来记录系统挂载了的文件系统
extern struct super_block *root_sb; 

struct block_buf{
    Slab_cache_t* pool;            // 缓冲池
    list_t free_list;                    // 空闲缓冲块队列
    wait_queue_T wait_list;              // 等待缓冲块的进程队列
    list_t hash_table[HASH_COUNT]; 
}block_buf_t;

typedef struct _buffer_
{
    char *data;           // 数据区 
    unsigned long block;  // 逻辑块号
    int refer_count;      // 缓冲块被引用的数量
    semaphore_T lock;     // 缓冲区锁
    bool dirty;           // 是否与磁盘不一致
    bool valid;           // 是否有效
    list_t hash_list_node; // 该节点位于哈希表的桶中
} buffer_t;

block_buf_t blkbuf[12];

#define hash(block) ((block) % HASH_COUNT)

static void* init_buffer(void* Vaddress, unsigned long arg) {
    buffer_t* bf = (buffer_t*)kmalloc(sizeof(buffer_t), 0);
    bf->data = (char*)Vaddress;
    bf->refer_count = 0;
    bf->dirty = 0;
    bf->valid = 0;
    bf->block = arg;
    semaphore_init(bf->lock, 1);
    list_add_to_behind(&blkbuf[0].hash_table[hash(arg)], &bf->hash_list_node);
    return bf;
}
static void* del_buffer(void* Vaddress, unsigned long arg) {
    list_t head = blkbuf[0].hash_table[hash(arg)];
    buffer_t* bf = container_of(list_next(&head), buffer_t, hash_list_node);
    kfree(bf);
    return NULL;
}
// 为块设备创建缓冲区
void buffer(void) {
    // 为所有块设备 创建缓冲区
    int i;

    struct FAT32_sb_info* sb = (struct FAT32_sb_info*)root_sb->private_sb_info;
    blobuf[0]->pool =  slab_create(sb->sector_per_cluster * sb->bytes_per_sector, NULL, NULL, 0);
    list_init(&blkbuf[0]->free_list);
    wait_queue_init(&blkbuf[0]->wait_list, NULL);
    for(i = 0; i < HASH_COUNT; i++)
        list_init(&blkbuf[0]->hash_table[i]);
}

// 设备号用来确定使用什么缓冲区，我给每一个分区，文件系统都设置了一个缓冲区
buffer_t *bread(unsigned long dev, unsigned long block, unsigned long size) {
    
    list_t head = blkbuf[dev]->hash_table[hash(block)];

    if(list_is_empty(&head))
    {
        // 为空则申请缓冲块
        buffer_t* bf = (buffer_t*)slab_malloc(blkbuf[dev], block);
    } else {
        //不为空则查找是否有符合的缓冲块
        list_t head = blkbuf[dev].hash_table[hash(arg)];
        do {
            head = head.next;
            buffer_t* bf = container_of(&head), buffer_t, hash_list_node);
        while(bf->block == block);
    }

}
