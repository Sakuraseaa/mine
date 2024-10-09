#ifndef __MEMORY_H
#define __MEMORY_H
/* 这里定义的结构体 已经几乎废弃 */
/*ARDS, Address Range Descriptor Structure, 每个结构20字节*/
struct Memory_E820_Formate
{
    u32_t address1; // 基地址低32位
    u32_t address2; // 基地址高32位
    u32_t length1;  // 内存长度低32位
    u32_t length2;  // 内存长度高32位
    u32_t type;     // 本段内存的类型
};
u64_t *Global_CR3 = NULL;
// E820 是 Memory_E820_Formate 的简单整合
struct E820
{
    u64_t address;
    u64_t length;
    u32_t type;
} __attribute__((packed));
// 修饰结构体不会生成对齐空间，改用紧凑格式

// page管理每一个物理内存页
struct Page
{
    struct Zone *zone_struct;      // 指向本页所属的区域结构体
    u64_t PHY_address;     // 页的物理地址
    u64_t attribute;       // 页的属性
    u64_t reference_count; // 记录该页的引用次数
    u64_t age;             // 该页的创建时间
};

// 描述 各个可用物理内存区域(可用的物理内存段)
struct Zone
{
    struct Page *pages_group;                    // struct page结构体数组指针
    u64_t pages_length;                  // 本区域包含的struct page结构体数量
    u64_t zone_start_address;            // 本区域的起始页对齐地址
    u64_t zone_end_address;              // 本区域的结束页对齐地址
    u64_t zone_length;                   // 本区域经过页对齐后的地址长度
    u64_t attribute;                     // 本区域空间的属性
    struct Global_Memory_Descriptor *GMD_struct; // 指向全局结构体 Global_Memory_Descriptor
    u64_t page_using_count;              // 本区域已使用物理内存页数量
    u64_t page_free_count;               // 本区域空闲物理内存页数量
    u64_t total_pages_link;              // 本区域物理页被引用次数
};

struct Global_Memory_Descriptor
{
    struct E820 e820[32];      // 物理内存段结构数组
    u64_t e820_length; // 物理内存段结构数组长度

    u64_t *bits_map;   // 物理地址空间页映射位图
    u64_t bits_size;   // 物理地址空间页数量
    u64_t bits_length; // 物理地址空间页映射位图长度

    struct Page *pages_struct;  // 全局struct page结构体数组指针
    u64_t pages_size;   // struct page结构体总数
    u64_t pages_length; // struct page结构体占用的字节数

    struct Zone *zones_struct;  // 全局struct zone结构体数组的指针
    u64_t zones_size;   // struct zone结构体数量
    u64_t zones_length; // struct zone结构体占用的字节数

    u64_t start_code; // 内核程序的起始代码段地址
    u64_t end_code;   // 内核程序的结束代码段地址
    u64_t end_data;   // 内核程序的结束数据段地址
    u64_t start_brk;  // 内核程序的结束地址

    u64_t end_rodata;

    u64_t end_of_struct; // 内存页管理结构的结尾地址
};

// 管理每个以物理页为单位的内存空间
// 每个物理页中包含着若干待分配的对象
typedef struct Slab
{
    struct List list;  // 连接其他的Slab结构体
    struct Page *page; // 记录所使用页面的page成员变量

    u64_t using_count; // 本物理页正在使用的块数
    u64_t free_count;  // 本物理页空闲的块数

    void *Vaddress; // 记录当前页面所在线性地址

    // 管理内存对象使用情况
    u64_t color_length;
    u64_t color_count; //// 本物理页中的小内存块数
    u64_t *color_map;
}Slab_t;

// 抽象内存池
typedef struct Slab_cache
{
    u64_t size;
    u64_t total_using;                               // 本内存池正在使用的内存块数
    u64_t total_free;                                // 本内存池空闲的内存块数
    struct Slab *cache_pool;                                 // 管理Slab
    struct Slab *cache_dma_pool;                             // 用于索引DMA内存池存储空间结构
    void *(*constructor)(void *Vaddress, u64_t arg); // 内存池构造函数
    void *(*destructor)(void *vaddress, u64_t arg);  // 内存池析构函数
}Slab_cache_t;

extern struct Global_Memory_Descriptor memory_management_struct;
#define Virt_To_2M_Page(kaddr) (memory_management_struct.pages_struct + (Virt_To_Phy(kaddr) >> PAGE_2M_SHIFT))
#define Phy_to_2M_Page(kaddr) (memory_management_struct.pages_struct + ((u64_t)(kaddr) >> PAGE_2M_SHIFT))

//// each zone index
int ZONE_DMA_INDEX = 0;
int ZONE_NORMAL_INDEX = 0;  // low 1GB RAM ,was mapped in pagetable
int ZONE_UNMAPED_INDEX = 0; // above 1GB RAM,unmapped in pagetable

/*
    kmalloc`s struct
*/
struct Slab_cache kmalloc_cache_size[16] =
    {
        {32, 0, 0, NULL, NULL, NULL, NULL},
        {64, 0, 0, NULL, NULL, NULL, NULL},
        {128, 0, 0, NULL, NULL, NULL, NULL},
        {256, 0, 0, NULL, NULL, NULL, NULL},
        {512, 0, 0, NULL, NULL, NULL, NULL},
        {1024, 0, 0, NULL, NULL, NULL, NULL}, // 1KB
        {2048, 0, 0, NULL, NULL, NULL, NULL},
        {4096, 0, 0, NULL, NULL, NULL, NULL}, // 4KB
        {8192, 0, 0, NULL, NULL, NULL, NULL},
        {16384, 0, 0, NULL, NULL, NULL, NULL},
        {32768, 0, 0, NULL, NULL, NULL, NULL},
        {65536, 0, 0, NULL, NULL, NULL, NULL},  // 64KB
        {131072, 0, 0, NULL, NULL, NULL, NULL}, // 128KB
        {262144, 0, 0, NULL, NULL, NULL, NULL},
        {524288, 0, 0, NULL, NULL, NULL, NULL},
        {1048576, 0, 0, NULL, NULL, NULL, NULL}, // 1MB
};

#define MAX_NR_ZONES 10 // max zone
#define SIZEOF_LONG_ALIGN(size) ((size + sizeof(long) - 1) & ~(sizeof(long) - 1))
#define SIZEOF_INT_ALIGN(size) ((size + sizeof(int) - 1) & ~(sizeof(int) - 1))

u64_t page_init(struct Page *page, u64_t flags);
u64_t page_clean(struct Page *page);
u64_t get_page_attribute(struct Page *page);
u64_t set_page_attribute(struct Page *page, u64_t flags);

struct Page *alloc_pages(int zone_select, int number, u64_t page_flags);
void free_pages(struct Page *page, int number);

/*
    return virtual kernel address
*/
void *kmalloc(u64_t size, u64_t flags);
struct Slab *kmalloc_create(u64_t size);
u64_t kfree(void *address);

struct Slab_cache *slab_create(u64_t size, void *(*constructor)(void *Vaddress, u64_t arg), void *(*destructor)(void *Vaddress, u64_t arg), u64_t arg);
u64_t slab_destroy(struct Slab_cache *slab_cache);

u64_t slab_init();
void *slab_malloc(struct Slab_cache *slab_cache, u64_t arg);
u64_t slab_free(struct Slab_cache *slab_cache, void *address, u64_t arg);
u64_t do_brk(u64_t addr, u64_t len);
void init_memory();
u64_t* pde_ptr(u64_t vaddr);
u64_t* pml4e_ptr(u64_t vaddr);
u64_t* pdpe_ptr(u64_t vaddr);
u64_t* pte_ptr(u64_t vaddr);
u64_t do_wp_page(u64_t virtual_address);
s64_t do_no_page(u64_t virtual_address);
void pagetable_4K_init();

void kdelete(void* address, u64_t size);
void *knew(u64_t size, u64_t gfp_flags);

adr_t viradr_to_phyadr(adr_t kviradr);
adr_t phyadr_to_viradr(adr_t kphyadr);

#endif