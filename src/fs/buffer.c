/* 
    2024-6-22 16:21
    ��Ȼ�������������רҵ�ģ����ҵļܹ����������ã��Ҳ���֪��Ӧ�ðѸ��߻����������ڲ���ϵͳ���Ǹ���ź���
    ��Ӧ���Ķ������Դ�룬ѧϰ���ģʽ��
    ��Ŀǰ�����������㣬������ʱֹͣ�˸��ٻ������ı�д
    2024-6-26 23:49
    ���ٻ�����Ӧ�����ļ�ϵͳ֮�£������豸��֮��
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
// �������Ӧ���޸ĳ�Ϊһ�����飬����¼ϵͳ�����˵��ļ�ϵͳ
extern struct super_block *root_sb; 



#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define BUFFER_DESC_NR 4 // ���������� 512 1024 2048 4096

typedef struct block_buf{
    
    u64 count;                          // ���������
    u64 size;                           // �����ߴ�

    list_t free_list;                    // ���л�������
    list_t idle_list;                    // �ݴ��������ͷŵĿ�
    wait_queue_T wait_list;              // �ȴ������Ľ��̶���
    list_t hash_table[HASH_COUNT];       // �ݴ��ϣ��
}bdesc_t;

typedef struct _buffer_
{
    char *data;            // ������ 
    bdesc_t *desc;        // ������ָ��
    dev_t dev;            // �豸��
    idx_t block;          // �߼����
    int refer_count;      // ����鱻���õ�����
    semaphore_T lock;     // ��������
    bool dirty;           // �Ƿ�����̲�һ��
    bool valid;           // �Ƿ���Ч
    list_t hnode;         // ��ϣ�������ڵ�
    list_t rnode;         // ����ڵ�
} buffer_t;

static bdesc_t bdescs[BUFFER_DESC_NR];
#define hash(dev, block) ( ((dev)^(block)) % HASH_COUNT )


// �ӹ�ϣ���в��� buffer
static buffer_t *get_from_hash_table(bdesc_t *desc, dev_t dev, idx_t block) {
    u64 idx = hash(dev, block)
    list_t list = &desc->hash_table[idx];

    buffer_t *buf = NULL;
    for () {

    }
}

static err_t buffer_alloc(bdesc_t *desc) {
    // here allocated memory is too small, i think 16KB for suitable
    char* addr = (char*)kmalloc(4096, 0); // ����һ����ڴ�
    
    buffer_t* buf = NULL;
    for(char* i = addr; i < addr + 4096; i += desc->size) { // ����ΪС�ڴ�
        buf = (buffer_t*)kmalloc(sizeof(buffer_t));
        
        buf->data = i;
        buf->block = 0;
        buf->desc = desc;
        buf->dirty = false;
        buf->valid = false;
        buf->refer_count = 0;
        buf->dev = 0;
        // һԪ�ź���������
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

// ����豸 dev���� block ��Ӧ�Ļ���
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


// Ϊ���豸����������
void buffer_init(void) {
    LOGK("buffer_t size is %d\n", sizeof(buffer_t));

    size_t sz = 512;
    for(size_t i = 0; i < BUFFER_DESC_NR; i++, sz <<= 1) {
        bdesc_t* desc = &bdescs[i];

        // �ڴ��Ϊ��
        desc->count = 0;
        desc->size = sz;

        // ��ʼ����������
        list_init(&desc->free_list);
        // ��ʼ����������
        list_init(&desc->idle_list);
        // ��ʼ���ȴ���������
        list_init(&desc->wait_list);

        // ��ʼ����ϣ��
        for (size_t i = 0; i < HASH_COUNT; i++)
        {
            list_init(&desc->hash_table[i]);
        }
    }
}
