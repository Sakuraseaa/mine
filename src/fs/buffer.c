/* 
    2024-6-22 16:21
    虽然我们是软件工程专业的，但我的架构能力并不好，我并不知道应该把告诉缓冲区设置在操作系统的那个层才合理
    我应该阅读更多的源码，学习设计模式？
    我目前编码能力不足，所以暂时停止了高速缓冲区的编写

*/
#include "buffer.h"
#include "memory.h"
#include "VFS.h"
#include "lib.h"
#include "types.h"
// 这个变量应该修改成为一个数组，来记录系统挂载了的文件系统
extern struct super_block *root_sb; 


#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define BUFFER_DESC_NR 3 // 描述符数量 512 1024 4096

typedef struct block_buf{
    Slab_cache_t* pool;                  // 缓冲池
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
#define hash(block) ((block) % HASH_COUNT)

static void* init_buffer(void* Vaddress, unsigned long arg) {

}
static void* del_buffer(void* Vaddress, unsigned long block) {

}



// 为块设备创建缓冲区
void buffer(void) {

}

// 设备号用来确定使用什么缓冲区，我给每一个分区，文件系统都设置了一个缓冲区
buffer_t *bread(unsigned long dev, long cmd, unsigned long block, long size, unsigned char *buffer) {

}
