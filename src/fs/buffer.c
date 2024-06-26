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
// �������Ӧ���޸ĳ�Ϊһ�����飬����¼ϵͳ�����˵��ļ�ϵͳ
extern struct super_block *root_sb; 



#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define BUFFER_DESC_NR 4 // ���������� 512 1024 2048 4096

typedef struct block_buf{
    Slab_cache_t* pool;                  // �����
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
#define hash(block) ((block) % HASH_COUNT)

static void* init_buffer(void* Vaddress, unsigned long arg) {

}
static void* del_buffer(void* Vaddress, unsigned long block) {

}

static err_t buffer_alloc(bdesc_t *desc) {
    if(desc->pool == NULL) {
        desc->pool = slab_create(desc.)
    }
}

// ��ȡ����buf
static buffer_t* get_free_buffer(bdesc_t* desc) {
    
    if((desc->pool->total_free == 0)) {
        buffer_alloc(desc);
    }

}

// �豸������ȷ��ʹ��ʲô���������Ҹ�ÿһ���������ļ�ϵͳ��������һ��������
buffer_t *bread(unsigned long dev, long cmd, unsigned long block, long size, unsigned char *buffer) {

}


// Ϊ���豸����������
void buffer_init(void) {
    LOGK("buffer_t size is %d\n", sizeof(buffer_t));

    size_t sz = 512;
    for(size_t i = 0; i < BUFFER_DESC_NR; i++, sz <<= 1) {
        bdesc_t* desc = &bdescs[i];

        // �ڴ��Ϊ��
        desc->pool = slab_create(sz, init_buffer, del_buffer, 0);
        
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
