#include "buffer.h"
#include "memory.h"
#include "VFS.h"
#include "lib.h"
// �������Ӧ���޸ĳ�Ϊһ�����飬����¼ϵͳ�����˵��ļ�ϵͳ
extern struct super_block *root_sb; 

struct block_buf{
    Slab_cache_t* pool;            // �����
    list_t free_list;                    // ���л�������
    wait_queue_T wait_list;              // �ȴ������Ľ��̶���
    list_t hash_table[HASH_COUNT]; 
}block_buf_t;

typedef struct _buffer_
{
    char *data;           // ������ 
    unsigned long block;  // �߼����
    int refer_count;      // ����鱻���õ�����
    semaphore_T lock;     // ��������
    bool dirty;           // �Ƿ�����̲�һ��
    bool valid;           // �Ƿ���Ч
    list_t hash_list_node; // �ýڵ�λ�ڹ�ϣ���Ͱ��
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
// Ϊ���豸����������
void buffer(void) {
    // Ϊ���п��豸 ����������
    int i;

    struct FAT32_sb_info* sb = (struct FAT32_sb_info*)root_sb->private_sb_info;
    blobuf[0]->pool =  slab_create(sb->sector_per_cluster * sb->bytes_per_sector, NULL, NULL, 0);
    list_init(&blkbuf[0]->free_list);
    wait_queue_init(&blkbuf[0]->wait_list, NULL);
    for(i = 0; i < HASH_COUNT; i++)
        list_init(&blkbuf[0]->hash_table[i]);
}

// �豸������ȷ��ʹ��ʲô���������Ҹ�ÿһ���������ļ�ϵͳ��������һ��������
buffer_t *bread(unsigned long dev, unsigned long block, unsigned long size) {
    
    list_t head = blkbuf[dev]->hash_table[hash(block)];

    if(list_is_empty(&head))
    {
        // Ϊ�������뻺���
        buffer_t* bf = (buffer_t*)slab_malloc(blkbuf[dev], block);
    } else {
        //��Ϊ��������Ƿ��з��ϵĻ����
        list_t head = blkbuf[dev].hash_table[hash(arg)];
        do {
            head = head.next;
            buffer_t* bf = container_of(&head), buffer_t, hash_list_node);
        while(bf->block == block);
    }

}
