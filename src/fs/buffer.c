/* 
    2024-6-22 16:21
    ��Ȼ�������������רҵ�ģ����ҵļܹ����������ã��Ҳ���֪��Ӧ�ðѸ��߻����������ڲ���ϵͳ���Ǹ���ź���
    ��Ӧ���Ķ������Դ�룬ѧϰ���ģʽ��
    ��Ŀǰ�����������㣬������ʱֹͣ�˸��ٻ������ı�д

*/
#include "buffer.h"
#include "memory.h"
#include "VFS.h"
#include "lib.h"
#include "types.h"
// �������Ӧ���޸ĳ�Ϊһ�����飬����¼ϵͳ�����˵��ļ�ϵͳ
extern struct super_block *root_sb; 


#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define BUFFER_DESC_NR 3 // ���������� 512 1024 4096

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



// Ϊ���豸����������
void buffer(void) {

}

// �豸������ȷ��ʹ��ʲô���������Ҹ�ÿһ���������ļ�ϵͳ��������һ��������
buffer_t *bread(unsigned long dev, long cmd, unsigned long block, long size, unsigned char *buffer) {

}
