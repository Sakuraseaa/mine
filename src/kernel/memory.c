/**
 * @file memory.c
 * @author your name (you@domain.com)
 * @brief 其中Slab技术针对特定的数据体的管理代码，没有使用到，请你找地方应用一下
 * ->（slab_create, slab_malloc, slab_destory, slab_free）
 * @version 0.1
 * @date 2024-04-09
 *
 * @copyright Copyright (c) 2024
 * 
 */
#include "memory.h"
#include "lib.h"
#include "printk.h"
#include "errno.h"
#include "assert.h"

// 给page结构体的属性成员赋值, 增加引用
unsigned long page_init(struct Page *page, unsigned long flags)
{
    page->attribute |= flags;

    if (!page->reference_count || (page->attribute & PG_Shared))
    {
        page->reference_count++;
        page->zone_struct->total_pages_link++;
    }
    return 1;
}

// 递减page页面的引用, 引用为0->属性删除
unsigned long page_clean(struct Page *page)
{
    page->reference_count--;
    page->zone_struct->total_pages_link--;
    if (!page->reference_count)
    { // 若页面的被引用数是0， 那么只保留物理页的被映射属性
        page->attribute &= PG_PTable_Maped;
    }
    return 1;
}

// 得到页面属性
unsigned long get_page_attribute(struct Page *page)
{
    if (!page)
    {
        color_printk(RED, BLACK, "get_page_sttribute() ERROR: page == NULL\n");
        return 0;
    }
    else
        return page->attribute;
}

// 设置页面属性
unsigned long set_page_attribute(struct Page *page, unsigned long flags)
{
    if (page == NULL)
    {
        color_printk(RED, BLACK, "set_page_attribute() ERROR: page == NULL\n");
        return 0;
    }
    else
    {
        page->attribute = flags;
        return 1;
    }
}

/**
 * @brief 创建一个Slab结构体, 并且初始化Struct Slab
 *
 * @param size  该2M物理页被分成多个size大小的内存块
 * @return struct Slab*  返回创建好的指针（该结构体需要被一次手动free）
 */
static struct Slab *init_Slab(unsigned long size)
{
    // a. 为tmp_slab结构体申请内存
    struct Slab *tmp_slab = (struct Slab *)kmalloc(sizeof(struct Slab), 0);
    if (tmp_slab == NULL)
    {
        color_printk(RED, BLACK, "init_Slab Fail: tmp_slab == NULL\n");
        return NULL;
    }

    memset(tmp_slab, 0, sizeof(struct Slab));
    list_init(&tmp_slab->list);

    // b. 申请出一个2M物理页给Page结构体, 并且初始化page
    tmp_slab->page = alloc_pages(ZONE_NORMAL, 1, 0);
    if (tmp_slab->page == NULL)
    {
        color_printk(RED, BLACK, "init_Slab Fail:tmp_slab->page == NULL\n");
        kfree(tmp_slab);
        return NULL;
    }
    page_init(tmp_slab->page, PG_Kernel);

    // c. 给Slab结构体的成员赋初始化
    tmp_slab->using_count = 0; // 这里我觉得应该赋值为0
    tmp_slab->free_count = PAGE_2M_SIZE / size;
    tmp_slab->Vaddress = Phy_To_Virt(tmp_slab->page->PHY_address);

    // d. 初始化本物理页中，管理许多个size的位图
    // 位图中有效位数
    tmp_slab->color_count = tmp_slab->free_count;
    // 位图长度/(字节)
    tmp_slab->color_length = ((tmp_slab->color_count + sizeof(unsigned long) * 8 - 1) >> 6) << 3;
    // 为位图申请内存
    tmp_slab->color_map = (unsigned long *)kmalloc(tmp_slab->color_length, 0);
    if (tmp_slab->color_map == NULL)
    {
        color_printk(RED, BLACK, "slab_malloc()->kmalloc()=>tmp_slab->color_map == NULL\n");
        free_pages(tmp_slab->page, 1);
        kfree(tmp_slab);
        return NULL;
    }

    memset(tmp_slab->color_map, 0xff, tmp_slab->color_length);
    int i = 0;
    for (; i < tmp_slab->color_count; i++)
        *(tmp_slab->color_map + (i >> 6)) ^= (1UL << i % 64);

    return tmp_slab;
}

/* number: number < 64, 要申请的页面数, 申请出的物理页是连续的
 * zone_select: zone select from dma, mapped in pagetable, unmapped int pagetable
 * page_flags: struct Page flages
 *  它最多可从DMA区域空间，已映射页表区域空间，未映射页表区域空间里，一次申请64个连续的物理页
 *  并设置这些物理页对应的struct page属性
 */
struct Page *alloc_pages(int zone_select, int number, unsigned long page_flags)
{
    int i;
    unsigned long page = 0;
    unsigned long attribute = 0;

    int zone_start = 0;
    int zone_end = 0;

    if (number >= 64 || number <= 0)
    {
        color_printk(RED, BLACK, "alloc_pages() ERROR: number is invaild\n");
        return 0;
    }

    // 选出需要的内存段
    switch (zone_select)
    {
    case ZONE_DMA:
        zone_start = 0;
        zone_end = ZONE_DMA_INDEX;
        attribute = PG_PTable_Maped;
        break;
    case ZONE_NORMAL:
        zone_start = ZONE_DMA_INDEX;
        zone_end = ZONE_NORMAL_INDEX;
        attribute = PG_PTable_Maped;
        break;
    case ZONE_UNMAPED:
        zone_start = ZONE_UNMAPED_INDEX;
        zone_end = memory_management_struct.zones_size - 1;
        attribute = 0;
        break;
    default:
        color_printk(RED, BLACK, "alloc_pages error zone_select index\n");
        return NULL;
        break;
    }

    // 遍历内存区域结构体, 找出符合申请条件的struct page结构体
    for (i = zone_start; i <= zone_end; i++)
    {
        struct Zone *z;
        unsigned long j;
        unsigned long start, end;
        unsigned long tmp;

        // 检测第i个数据空间是否有空足够空闲页，可提供
        if ((memory_management_struct.zones_struct + i)->page_free_count < number)
            continue;

        z = memory_management_struct.zones_struct + i;  // 选择出对应的内存区域
        start = z->zone_start_address >> PAGE_2M_SHIFT; // start是在位图中，该物理块对应的第几位数
        end = z->zone_end_address >> PAGE_2M_SHIFT;

        tmp = 64 - start % 64;
        // 在内存区域zone中，遍历对应的物理页，按照UNSIGNED LONG类型作为步进长度(这里的处理很绝，值得学习)
        for (j = start; j < end; j += j % 64 ? tmp : 64)
        {
            unsigned long *p = memory_management_struct.bits_map + (j >> 6); // 定位到位图中的long
            unsigned long shift = j % 64;                                    // 在long中的偏移
            unsigned long k = 0;
            unsigned long num = (1UL << number) - 1;

            for (k = shift; k < 64; k++) // 每次遍历64个
            {
                //  (*p >> k) | (*(p + 1) << (64 - k))将后一个UNSIGNED LONG变量的低位补齐到正在检索的变量中
                //  这样申请出的物理页是连续的
                unsigned long z1 = (*p >> k) | (*(p + 1) << (64 - k));
                if (!((k ? (z1) : *p) & (num)))
                {
                    // 找到了连续的number个物理页面, 初始化这个页面
                    unsigned long l;
                    page = j + k - shift;
                    for (l = 0; l < number; l++) // 分配每一个页
                    {
                        struct Page *x = memory_management_struct.pages_struct + page + l;
                        // 位图的改变标志着内存的分配于释放
                        *(memory_management_struct.bits_map + ((x->PHY_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (x->PHY_address >> PAGE_2M_SHIFT) % 64;

                        z->page_using_count++;
                        z->page_free_count--;
                        page_init(x, page_flags);
                    }
                    goto find_free_pages;
                }
            }
        }
    }
    return NULL;
find_free_pages:
    return (struct Page *)(memory_management_struct.pages_struct + page);
}

/* page: free page start from this pointer
    number : number < 64
*/
void free_pages(struct Page *page, int number)
{
    int i = 0;
    if (page == NULL)
    {
        color_printk(RED, BLACK, "free_pages() ERROR: page is invalid\n");
        return;
    }

    if (number >= 64 || number <= 0)
    {
        color_printk(RED, BLACK, "free_pages() ERROR:number is invalid\n");
        return;
    }

    for (i = 0; i < number; i++, page++)
    {
        *(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) &= ~(1UL << (page->PHY_address >> PAGE_2M_SHIFT) % 64);
        page->zone_struct->page_using_count--;
        page->zone_struct->page_free_count++;
        page->attribute = 0;
    }
}

// 在具体的SLAB内存池申请对象
void *slab_malloc(struct Slab_cache *Slab_cache, unsigned long arg)
{
    struct Slab *slab_p = Slab_cache->cache_pool;
    struct Slab *tmp_slab = NULL;
    int j = 0;


    // a.内存池中没有Slab可用了, 申请一个物理页，加入内存池
    if (Slab_cache->total_free == 0 || slab_p == NULL)
    {
        tmp_slab = init_Slab(Slab_cache->size);
        if (tmp_slab == NULL)
        {
            color_printk(RED, BLACK, "slab_malloc::init_Slab ERROR: can't alloc");
            return NULL;
        }

        if(slab_p == NULL)
            Slab_cache->cache_pool = tmp_slab;
        else
            list_add_to_behind(&Slab_cache->cache_pool->list, &tmp_slab->list);
        
        Slab_cache->total_free += tmp_slab->color_count;

        // b.根据Slab结构体中的内存位图，寻址对应虚拟地址返回给调用者
        for (j = 0; j <= tmp_slab->color_count; j++)
        {
            if ((*(tmp_slab->color_map + (j >> 6)) & (1UL << (j % 64))) == 0)
            { // 检测哪一位是否为0
                *(tmp_slab->color_map + (j >> 6)) |= (1UL << (j % 64));

                tmp_slab->using_count++; // 本物理页正在使用的块数
                tmp_slab->free_count--;  // 本物理页空闲的块数

                Slab_cache->total_using++; // 本内存池正在使用的块数
                Slab_cache->total_free--;  // 本内存池空闲的块数

                if (Slab_cache->constructor != NULL)
                    return Slab_cache->constructor((char *)tmp_slab->Vaddress + Slab_cache->size * j, arg);
                else
                    return (void *)((char *)tmp_slab->Vaddress + Slab_cache->size * j);
            }
        }
    }
    else
    {
        do
        {
            if (slab_p->free_count == 0)
            { // 本物理页没有空余的结构体，那么递增到下一个物理页结构体
                slab_p = container_of(list_next(&slab_p->list), struct Slab, list);
                continue;
            }

            for (j = 0; j < slab_p->color_count; j++)
            {
                if (*(slab_p->color_map + (j >> 6)) == 0xffffffffffffffffUL)
                { // 如果本long里面没有空位, 递增 j
                    j += 63;
                    continue;
                }

                if ((*(slab_p->color_map + (j >> 6)) & (1UL << (j % 64))) == 0)
                {
                    // 检测哪一位是否为0
                    *(slab_p->color_map + (j >> 6)) |= (1UL << (j % 64));

                    slab_p->using_count++; // 本物理页正在使用的块数
                    slab_p->free_count--;  // 本物理页空闲的块数

                    Slab_cache->total_using++; // 本内存池正在使用的块数
                    Slab_cache->total_free--;  // 本内存池空闲的块数

                    if (Slab_cache->constructor != NULL)
                        return Slab_cache->constructor((char *)slab_p->Vaddress + Slab_cache->size * j, arg);
                    else
                        return (void *)((char *)slab_p->Vaddress + Slab_cache->size * j);
                }
            }
        } while (slab_p != Slab_cache->cache_pool);
    }

    color_printk(RED, BLACK, "slab_malloc ERROR: can't alloc\n");
    return NULL;
}

// 释放具体的SLAB内存池中的对象
/**
 * @brief a. 复位颜色位图color_map对应的索引位
 *        b. 调整内存池的相关计数器, total_using, total_free, using_count, free_count
 *        c. 调用内存池的自定义析构功能
 *        d. 如果目标Slab结构中的内存对象全部空闲，并且内存池的空闲内存对象数量超1.5倍的slab_p->color_count
 *           则将这个struct Slab结构体释放以降低系统内存使用率
 *
 * @param slab_cache    内存池
 * @param addres    需要释放的虚拟地址
 * @param arg       析构功能的参数
 * @return unsigned long  成功返回 1， 不成功返回 0
 */
unsigned long slab_free(struct Slab_cache *slab_cache, void *address, unsigned long arg)
{
    struct Slab *slab_p = slab_cache->cache_pool;
    assert(slab_p != NULL);
    int index = 0;
    do
    {
        // 先通过内存对象(由调用者传入)的虚拟地址判断其所在的struct Slba结构体
        if (slab_p->Vaddress <= address && address < slab_p->Vaddress + PAGE_2M_SIZE)
        {
            index = (address - slab_p->Vaddress) / slab_cache->size;
            *(slab_p->color_map + (index >> 6)) ^= 1UL << (index % 64);

            slab_p->free_count++;
            slab_p->using_count--;

            slab_cache->total_free++;
            slab_cache->total_using--;

            if (slab_cache->destructor)
            {
                slab_cache->destructor((char *)address, arg);
                // slab_cache->destructor((char *)slab_p->Vaddress + slab_cache->size * index, arg);
            }

            // 这个物理页全是空的
            if ((slab_p->using_count == 0) && (slab_cache->total_free >= slab_p->color_count * 3 / 2))
            {
                list_del(&slab_p->list);
                slab_cache->total_free -= slab_p->color_count;

                kfree(slab_p->color_map);
                page_clean(slab_p->page);
                free_pages(slab_p->page, 1);
                kfree(slab_p);
            }

            return 1;
        }
        else
        {
            // 该虚拟地址对应的内存Slab, 物理页不属于该SLab结构体。递增到下一个Slab，下一个物理页
            slab_p = container_of(list_next(&slab_p->list), struct Slab, list);
            continue;
        }
    } while (slab_p != slab_cache->cache_pool);

    // 如果执行到这里, 说明程序出错
    color_printk(RED, BLACK, "slab_free() ERROR: address not int slab\n");
    return 0;
}


/**
 * @brief SLAB内存池的创建
 * 
 * @param size 每个内存块的大小
 * @param constructor  内存块初始化函数
 * @param destructor   内存块销毁函数
 * @param arg  暂时没有用到
 * @return struct Slab_cache* 内存池指针
 */
struct Slab_cache *slab_create(unsigned long size, void *(*constructor)(void *Vaddress, unsigned long arg),
                               void *(*destructor)(void *Vaddress, unsigned long arg), unsigned long arg)
{
    // 1. 为Slab_cache申请内存
    struct Slab_cache *slab_cache = NULL;
    slab_cache = (struct Slab_cache *)kmalloc(sizeof(struct Slab_cache), 0);
    if (slab_cache == NULL)
    {
        color_printk(RED, BLACK, "slab_create()->kmalloc()=>slab_cache == NULL \n");
        return NULL;
    }
    memset(slab_cache, 0, sizeof(struct Slab_cache));

    // 2. 初始化Slab_cache的成员
    slab_cache->size = SIZEOF_LONG_ALIGN(size);
    slab_cache->total_using = 0;
    slab_cache->cache_dma_pool = NULL;
    slab_cache->constructor = constructor;
    slab_cache->destructor = destructor;

    slab_cache->cache_pool = NULL;

    return slab_cache;
}

// SLAB内存池的销毁
unsigned long slab_destroy(struct Slab_cache *slab_cache)
{
    struct Slab *slab_p = slab_cache->cache_pool;
    struct Slab *tmp_slab = NULL;

    // 确保没有使用的内存块
    if (slab_cache->total_using != 0)
    {
        color_printk(RED, BLACK, "slab_cache->total_using != 0\n");
        return 0;
    }

    if(slab_cache->cache_pool == NULL)
        goto NO_SLAB_MEM;
    

    // 销毁Slab_cache里的每一个Slab
    while (!list_is_empty(&slab_p->list))
    {
        tmp_slab = slab_p;
        slab_p = container_of(list_next(&slab_p->list), struct Slab, list);

        list_del(&tmp_slab->list);
        kfree(tmp_slab->color_map);

        page_clean(tmp_slab->page);
        free_pages(tmp_slab->page, 1); // 释放页
        kfree(tmp_slab);
    }

    // 销毁最后一个Slab
    kfree(slab_p->color_map);

    page_clean(slab_p->page);
    free_pages(slab_p->page, 1);
    kfree(slab_p);

NO_SLAB_MEM:
    // 销毁Slab_cache
    kfree(slab_cache);

    return 1;
}

// 初始化不同规格的内存池, 给每个内存池暂时分配一个slab, page物理页
// 此处slab占用的空间是我们静态申请使用的。
unsigned long slab_init()
{
    struct Page *page = NULL;
    // get a free page and set to empty page table and return the virtual address
    unsigned long *virtual = NULL;
    unsigned long i, j;

    unsigned long tmp_address = memory_management_struct.end_of_struct;
    // 给16种内存池，分配Slab结构体
    for (i = 0; i < 16; i++)
    {
        // 给内存池创建Slab结构体(递增内核边界)
        kmalloc_cache_size[i].cache_pool = (struct Slab *)memory_management_struct.end_of_struct;
        memory_management_struct.end_of_struct = memory_management_struct.end_of_struct + sizeof(struct Slab) + sizeof(long) * 10;

        /////////////////// 初始化必要的Slab结构体的成员
        list_init(&kmalloc_cache_size[i].cache_pool->list);
        kmalloc_cache_size[i].cache_pool->using_count = 0;
        kmalloc_cache_size[i].cache_pool->free_count = PAGE_2M_SIZE / kmalloc_cache_size[i].size; // 空闲块数
        // 位图长度 /字节
        kmalloc_cache_size[i].cache_pool->color_length = ((PAGE_2M_SIZE / kmalloc_cache_size[i].size + sizeof(unsigned long) * 8 - 1) >> 6) << 3;
        // 本Slab里面有多少个可用块数
        kmalloc_cache_size[i].cache_pool->color_count = kmalloc_cache_size[i].cache_pool->free_count;

        // 建立位图, 递增内核边界, 把所有位图置位
        kmalloc_cache_size[i].cache_pool->color_map = (unsigned long *)memory_management_struct.end_of_struct;
        memory_management_struct.end_of_struct = (unsigned long)(memory_management_struct.end_of_struct + kmalloc_cache_size[i].cache_pool->color_length + sizeof(long) * 10) & (~(sizeof(long) - 1));
        memset(kmalloc_cache_size[i].cache_pool->color_map, 0xff, kmalloc_cache_size[i].cache_pool->color_length);

        // 把位图中该恢复的位，恢复, 异或
        for (j = 0; j < kmalloc_cache_size[i].cache_pool->color_count; j++)
            *(kmalloc_cache_size[i].cache_pool->color_map + (j >> 6)) ^= 1UL << j % 64;

        kmalloc_cache_size[i].total_free = kmalloc_cache_size[i].cache_pool->color_count;
        kmalloc_cache_size[i].total_using = 0;
    }

    // 配置扩展空间对应的struct Page结构体以表示此内核内存空间已被使用
    // init page for kernel code and memory management struct
    // j 使用向上对其的原因是，第一个物理页已经被我们收到分配并使用了
    // 而对于 i 没有使用向上对其，那么对于i页面也是要初始化的，所以for循环要使用小于等于号
    i = Virt_To_Phy(memory_management_struct.end_of_struct) >> PAGE_2M_SHIFT;
    for (j = PAGE_2M_ALIGN(Virt_To_Phy(tmp_address)) >> PAGE_2M_SHIFT; j <= i; j++)
    {
        page = memory_management_struct.pages_struct + j;
        *(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (page->PHY_address >> PAGE_2M_SHIFT) % 64;
        page->zone_struct->page_using_count++;
        page->zone_struct->page_free_count--;
        page_init(page, PG_PTable_Maped | PG_Kernel_Init | PG_Kernel);
    }

    // color_printk(ORANGE, BLACK, "2.memory_management_struct.bits_map:%#018lx\tzone_struct->page_using_count:%d\t end_of_struct:%#018lx\n", *memory_management_struct.bits_map, memory_management_struct.zones_struct->page_using_count, memory_management_struct.end_of_struct);

    // 给每个内存池 分配1个物理页
    for (i = 0; i < 16; i++)
    {
        // a. 计算物理页地址, 向上2MB对齐
        virtual = (unsigned long *)((memory_management_struct.end_of_struct + PAGE_2M_SIZE * i + PAGE_2M_SIZE - 1) & PAGE_2M_MASK);

        // b. 根据虚拟页计算(已经分配好的)strcut Page结构体的位置
        page = Virt_To_2M_Page(virtual);

        // c. 分配物理页
        *(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (page->PHY_address >> PAGE_2M_SHIFT) % 64;

        // d. 更新zone_struct中的计数
        page->zone_struct->page_using_count++;
        page->zone_struct->page_free_count--;

        // e. 更新物理页属性
        page_init(page, PG_PTable_Maped | PG_Kernel_Init | PG_Kernel);

        // f.为内存池 i 初始化Slab
        kmalloc_cache_size[i].cache_pool->page = page;
        kmalloc_cache_size[i].cache_pool->Vaddress = virtual;
    }

    color_printk(ORANGE, BLACK, "3.memory_management_struct.bits_map:%#018lx\tzone_struct->page_using_count:%d\tzone_struct->page_free_count:%d\n", *memory_management_struct.bits_map, memory_management_struct.zones_struct->page_using_count, memory_management_struct.zones_struct->page_free_count);
    color_printk(ORANGE, BLACK, "start_code:%#018lx,end_code:%#018lx,end_data:%#018lx,start_brk:%#018lx,end_of_struct:%#018lx\n", memory_management_struct.start_code, memory_management_struct.end_code, memory_management_struct.end_data, memory_management_struct.start_brk, memory_management_struct.end_of_struct);
    return 1;
}

// 内存池资源不足时，使用该函数创建一个Slab结构体，申请一个物理页，加入到内存池
struct Slab *kmalloc_create(unsigned long size)
{
    int i;
    struct Slab *slab = NULL;
    struct Page *page = NULL;
    unsigned long *vaddress = NULL;
    long structsize = 0; // 记录 Slab 和 位图 的大小

    // 申请一个物理页
    page = alloc_pages(ZONE_NORMAL, 1, 0);
    if (page == NULL)
    {
        color_printk(RED, BLACK, "kmalloc_create()->alloc_pages()=>page == NULL\n");
        return NULL;
    }
    page_init(page, PG_Kernel);

    switch (size)
    {
    ////////////////////slab + map in 2M page
    case 32:
    case 64:
    case 128:
    case 256:
    case 512:
        // 对于范围在32B ~ 512B的小尺寸内存对象。虽然这些内存对象的尺寸比较小
        // 但他们的颜色位图却占用了较大的存储空间，这里将struct Slab结构体和数据存储空间放在同一个内存页内
        vaddress = Phy_To_Virt(page->PHY_address);
        structsize = sizeof(struct Slab) + PAGE_2M_SIZE / size / 8; // Slab 和位图占用的字节数

        // 这是Slab结构体 和本物理页的位图
        slab = (struct Slab *)((unsigned char *)vaddress + PAGE_2M_SIZE - structsize);
        slab->color_map = (unsigned long *)((unsigned char *)slab + sizeof(struct Slab));

        slab->free_count = (PAGE_2M_SIZE - structsize) / size;
        slab->using_count = 0;
        slab->color_count = slab->free_count;
        slab->Vaddress = vaddress;
        slab->page = page;
        list_init(&slab->list);

        // 位图长度/字节，向上对齐64字节
        slab->color_length = ((slab->color_count + sizeof(unsigned long) * 8 - 1) >> 6) << 3;
        memset(slab->color_map, 0xff, slab->color_length);

        // 根据总可用块数，恢复位图
        for (i = 0; i < slab->color_count; i++)
            *(slab->color_map + (i >> 6)) ^= 1UL << i % 64;

        break;
    ///////////////////// kmalloc slab and map, not in 2M page anymore
    case 1024: // 1kb
    case 2048:
    case 4096: // 4kb
    case 8192:    // 8KB
    case 16384:   //16KB
        /////////////////// color_map is a very short buffer
    case 32768:  // 32 KB
    case 65536:  // 64KB
    case 131072: // 128KB
    case 262144:
    case 524288:
    case 1048576: // 1MB
        // 这里使用kmlloc函数给Slab结构体和位图申请存储空间，因为这里的内存块大，位图小，
        // 提高物理页空间的利用率
        slab = (struct Slab *)kmalloc(sizeof(struct Slab), 0);

        slab->free_count = PAGE_2M_SIZE / size;
        slab->using_count = 0;
        slab->color_count = slab->free_count;
        slab->color_length = ((slab->color_count + sizeof(unsigned long) * 8 - 1) >> 6) << 3;

        // 创建位图
        slab->color_map = (unsigned long *)kmalloc(slab->color_length, 0);
        memset(slab->color_map, 0xff, slab->color_length);

        slab->Vaddress = Phy_To_Virt(page->PHY_address);
        slab->page = page;
        list_init(&slab->list);

        // 恢复位图
        for (i = 0; i < slab->color_count; i++)
            *(slab->color_map + (i >> 6)) ^= 1UL << i % 64;

        break;
    default:
        color_printk(RED, BLACK, "kmalloc_create() ERROR: wrong size:%08d\n", size);
        free_pages(page, 1); // 释放 2MB 物理页
        return NULL;
    }

    return slab;
}

/**
 * @brief 申请size大小的内存块
 *
 * @param size
 * @param gfp_flags  the condition of get memory
 * @return void*  return virtual kernel address
 */
void *kmalloc(unsigned long size, unsigned long gfp_flags)
{
    int i, j;
    struct Slab *slab = NULL;
    if (size > 1048576)
    {
        // 如果申请的资源超过了1MB, 那么就直接返回
        color_printk(RED, BLACK, "kmalloc() ERROR: kmalloc size too long:%08d\n", size);
        return NULL;
    }
    // 寻找到合适的内存池
    for (i = 0; i < 16; i++)
        if (kmalloc_cache_size[i].size >= size)
            break;

    slab = kmalloc_cache_size[i].cache_pool;
    if (kmalloc_cache_size[i].total_free != 0)
    { // true- 内存池有空闲内存块，可分配，寻找到确切的物理页
        do
        { // 在内存池中找空闲Slab,空闲的物理页
            if (slab->free_count == 0)
                slab = container_of(list_next(&slab->list), struct Slab, list);
            else
                break;
        } while (slab != kmalloc_cache_size[i].cache_pool);
    }
    else
    { // false -内存块空没有空的内存块了，申请新的物理页加入内存池
        slab = kmalloc_create(kmalloc_cache_size[i].size);
        if (slab == NULL)
        {
            color_printk(BLUE, BLACK, "kmalloc()->kmalloc_create()=>slab==NULL\n");
            return NULL;
        }

        kmalloc_cache_size[i].total_free += slab->color_count;

        color_printk(BLUE, BLACK, "memory pool(%#010x) expended, using_page:%x, free_page:%x\n",
                     kmalloc_cache_size[i].size, kmalloc_cache_size[i].total_using, kmalloc_cache_size[i].total_free);

        list_add_to_before(&kmalloc_cache_size[i].cache_pool->list, &slab->list);
    }

    // 在物理页/(Slab)中，寻找一块内存，并返回
    for (j = 0; j < slab->color_count; j++)
    {
        if (*(slab->color_map + (j >> 6)) == 0xffffffffffffffffUL)
        {
            j += 63;
            continue;
        }

        if ((*(slab->color_map + (j >> 6)) & (1UL << (j % 64))) == 0)
        {
            *(slab->color_map + (j >> 6)) |= (1UL << (j % 64));
            slab->free_count--;
            slab->using_count++;

            kmalloc_cache_size[i].total_free--;
            kmalloc_cache_size[i].total_using++;

            return (void *)((char *)slab->Vaddress + kmalloc_cache_size[i].size * j);
        }
    }

    color_printk(BLUE, BLACK, "kmalloc() ERROR: no memory can alloc\n");
    return NULL;
}

/**
 * @brief 释放内存
 *
 * @param address 需要被释放的地址
 * @return unsigned long 1(false), 0(ture)
 */
unsigned long kfree(void *address)
{
    int i, index;
    void *page_base_address = (void *)((unsigned long)address & PAGE_2M_MASK); // 物理页虚拟基地址
    struct Slab *slab = NULL;
    // 这里内存的释放代价是否有点大了？
    // 遍历各种内存池，寻找需要操作的物理页
    for (i = 0; i < 16; i++)
    {
        slab = kmalloc_cache_size[i].cache_pool; // 遍历该内存池的Slab
        do
        {
            if (slab->Vaddress == page_base_address)
            {
                // 若物理页基址相等，则说明找到了对应的物理页

                // 在物理页中，确定address对应的内存块的偏移
                index = (address - slab->Vaddress) / kmalloc_cache_size[i].size;
                *(slab->color_map + (index >> 6)) ^= 1 << index % 64; // 修改位图，释放内存

                slab->using_count--;
                slab->free_count++;

                kmalloc_cache_size[i].total_free++;
                kmalloc_cache_size[i].total_using--;
                if ((slab->using_count == 0) && (kmalloc_cache_size[i].total_free > slab->color_count * 3 / 2) && (kmalloc_cache_size[i].cache_pool != slab))
                { // 当前Slab结构体管理的内存对象全部空闲 && Slab结构不是当初手动创建的静态存储空间
                    // && 内存池仍有超过1.5倍的slab.color_count数量的空闲对象时 => 从内存池中卸载掉该物理页
                    switch (kmalloc_cache_size[i].size)
                    {
                    //////////////////////// slab + map in 2M page
                    case 32:
                    case 64:
                    case 128:
                    case 256:
                    case 512:
                        list_del(&slab->list);
                        kmalloc_cache_size[i].total_free -= slab->free_count;

                        page_clean(slab->page);
                        free_pages(slab->page, 1); // 释放2MB物理页
                        break;
                    default:
                        list_del(&slab->list);
                        kmalloc_cache_size[i].total_free -= slab->free_count;

                        kfree(slab->color_map);

                        page_clean(slab->page);
                        free_pages(slab->page, 1);
                        kfree(slab);
                        break;
                    }
                }
                return 1;
            }
            else
                slab = container_of(list_next(&slab->list), struct Slab, list);

        } while (slab != kmalloc_cache_size[i].cache_pool);
    }

    color_printk(RED, BLACK, "kfree() ERROR: can't free memory\n");
    return 0;
}

/**
 * @brief 内核页表重新初始化，直至0-4GB(我的电脑内存小于4GB,so 我可用的内存都被映射到内核空间了2MB ~ 512MB)内的物理页
 *  全部映射到了线性地址空间(以0xffff800000000000为基址)
 *  页表的权限也全为内核级, 此处的映射采用的是一一映射。
 *  如果我的内存是16GB,那么0-4GB都是给内核空间的, 4GB~16GB都是没有映射的, 可用给用户空间
 */
void pagetable_init()
{
    unsigned long i, j;
    unsigned long *tmp = NULL;

    // Global_CR3 = Get_gdt();
    // tmp = (unsigned long *)(Phy_To_Virt((unsigned long)Global_CR3 & (~0xfffUL)) + 256);
    // color_printk(YELLOW, BLACK, "1:%#018lx, %018lx\t\t", (unsigned long)tmp, *tmp);
    // tmp = Phy_To_Virt(*tmp & (~0xfffUL));
    // color_printk(YELLOW, BLACK, "2:%#018lx, %018lx\t\t", (unsigned long)tmp, *tmp);
    // tmp = Phy_To_Virt(*tmp & (~0xfffUL));
    // color_printk(YELLOW, BLACK, "3:%#018lx, %#018lx\t\t\n", (unsigned long)tmp, *tmp);

    // 遍历内存区域
    for (i = 0; i < memory_management_struct.zones_size; i++)
    {
        if (i == ZONE_UNMAPED_INDEX && ZONE_UNMAPED_INDEX)
            break;

        struct Zone *z = memory_management_struct.zones_struct + i;
        struct Page *p = z->pages_group;

        // 遍历该内存区域的物理页
        for (j = 0; j < z->pages_length; j++, p++)
        {
            // 获取该虚拟地址对应的PML(page map level 4, 4级页表)中的页表项指针
            tmp = (unsigned long *)((unsigned long)Phy_To_Virt((unsigned long)Global_CR3 & (~0xfffUL)) +
                                    (((unsigned long)Phy_To_Virt(p->PHY_address) >> PAGE_GDT_SHIFT) & 0x1ff) * 8);

            if (*tmp == 0)
            { // 页表项为空，则分配4kbPDPT页表,填充该表项
                unsigned long *virtual_addrees = kmalloc(PAGE_4K_SIZE, 0);
                // set_mpl4t(tmp, mk_mpl4t(Virt_To_Phy(virtual_addrees), PAGE_KERNEL_GDT));
                set_mpl4t(tmp, mk_mpl4t(Virt_To_Phy(virtual_addrees), PAGE_USER_GDT));
            }
            //=======================================================================================

            // 获取该虚拟地址对应的PDPT(page directory point table)中的页表项指针
            tmp = (unsigned long *)((unsigned long)Phy_To_Virt(*tmp & (~0xfffUL)) +
                                    (((unsigned long)Phy_To_Virt(p->PHY_address) >> PAGE_1G_SHIFT) & 0x1ff) * 8);
            if (*tmp == 0)
            { // 页表项为空，则分配4kb-PDT(page directory table)页表，填充该表项
                unsigned long *virtual_address = kmalloc(PAGE_4K_SIZE, 0);
                set_pdpt(tmp, mk_pdpt(Virt_To_Phy(virtual_address), PAGE_USER_Dir));
            }

            // ========================================================================================
            // 获取该虚拟地址对应的PDT(page directory table)中的页表项指针
            tmp = (unsigned long *)((unsigned long)Phy_To_Virt(*tmp & (~0xfffUL)) +
                                    (((unsigned long)Phy_To_Virt(p->PHY_address) >> PAGE_2M_SHIFT) & 0x1ff) * 8);
            // 在页表中填写对应的物理页
            set_pdt(tmp, mk_pdt(p->PHY_address, PAGE_USER_Page));

            // if (j % 50 == 0)
            //   color_printk(GREEN, BLACK, "@:%#018lx,%#018lx\t\n", (unsigned long)tmp, *tmp);
        }
    }

    flush_tlb();
}

void init_memory()
{
    int i, j;
    unsigned long TotalMem = 0;
    struct E820 *p = NULL;

    // color_printk(BLUE, BLACK, "Display Physics Address MAP,Type(1:RAM,2:ROM or Reserved,3:ACPI Reclaim Memory,4:ACPI NVS Memory,Others:Undefine)\n");
    p = (struct E820 *)0xffff800000007e00;
    // 得到内存地址范围信息
    for (i = 0; i < 32; i++)
    {
        // 如果内存地址有效，则打印输出
        // if (p->type == 1)
        // color_printk(ORANGE, BLACK, "Address:%#018lx\tLength:%#018lx\tType:%#010x\n", p->address, p->length, p->type);

        if (p->type == 1)
            TotalMem += p->length;

        memory_management_struct.e820[i].address = p->address;

        memory_management_struct.e820[i].length = p->length;

        memory_management_struct.e820[i].type = p->type;

        if (i == 0 || memory_management_struct.e820[i].address > memory_management_struct.e820[i - 1].address)
            memory_management_struct.e820_length = i;

        p++;
        if (p->type > 4 || p->length == 0 || p->type < 1)
            break;
    }

    // color_printk(ORANGE, BLACK, "OS Can Used Total RAM:%#018lx\n", TotalMem);

    TotalMem = 0;
    // 把可操作的地址对齐到2MB, 计算有多少2MB物理页可用
    for (i = 0; i <= memory_management_struct.e820_length; i++)
    {
        unsigned long start, end;
        if (memory_management_struct.e820[i].type != 1)
            continue;

        // start向上取整，end向下取整
        start = PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
        end = ((memory_management_struct.e820[i].address + memory_management_struct.e820[i].length) >> PAGE_2M_SHIFT) << PAGE_2M_SHIFT;

        if (end <= start)
            continue;
        TotalMem += (end - start) >> PAGE_2M_SHIFT;
    }

    color_printk(ORANGE, BLACK, "OS Can Used Total 2M PAGEs:%#018lx=%018ld\n", TotalMem, TotalMem);
    // 这里计算出的TotalMem是4GB, 最大的寻址范围(此处使用4GB开始计算对系统安全吗？)
    TotalMem = memory_management_struct.e820[memory_management_struct.e820_length].address +
               memory_management_struct.e820[memory_management_struct.e820_length].length;

    // bits map construction init
    //================创建物理页位图==============================================================
    // a. 位图设置在内核之后，向上对齐4kb
    memory_management_struct.bits_map = (unsigned long *)((memory_management_struct.start_brk + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);
    // 2MB物理页面数
    memory_management_struct.bits_size = TotalMem >> PAGE_2M_SHIFT;
    // b. 位图长度(单位是字节) --- 此处的 & 符号可以理解为除法
    memory_management_struct.bits_length = (((unsigned long)(TotalMem >> PAGE_2M_SHIFT) + sizeof(long) * 8 - 1) / 8) & (~(sizeof(long) - 1));
    // c. 把位图全置位 init bits map memory
    memset(memory_management_struct.bits_map, 0xff, memory_management_struct.bits_length); // init bits map memory

    // pages construction init
    // ===========创建物理页结构体数组 - pages construction init =================================================
    // a. 物理页结构体创建在物理页位图之后，向上对齐4kb, 要求清空这段区域
    memory_management_struct.pages_struct = (struct Page *)(((unsigned long)memory_management_struct.bits_map + memory_management_struct.bits_length + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);

    memory_management_struct.pages_size = TotalMem >> PAGE_2M_SHIFT;

    memory_management_struct.pages_length = ((TotalMem >> PAGE_2M_SHIFT) * sizeof(struct Page) + sizeof(long) - 1) & (~(sizeof(long) - 1));

    memset(memory_management_struct.pages_struct, 0x00, memory_management_struct.pages_length); // init pages memory

    // zones construction init
    // ===========创建可用物理内存区域结构体 - pages consturction init ====================
    memory_management_struct.zones_struct = (struct Zone *)(((unsigned long)memory_management_struct.pages_struct + memory_management_struct.pages_length + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);

    memory_management_struct.zones_size = 0;

    memory_management_struct.zones_length = (5 * sizeof(struct Zone) + sizeof(long) - 1) & (~(sizeof(long) - 1));

    memset(memory_management_struct.zones_struct, 0x00, memory_management_struct.zones_length);
    // ================ 初始化 Zone 和 Page 结构体
    for (i = 0; i <= memory_management_struct.e820_length; i++)
    {
        unsigned long start, end;
        struct Zone *z;
        struct Page *p;
        // 不符合要求的内存区域被掠过
        if (memory_management_struct.e820[i].type != 1)
            continue;
        start = PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
        end = ((memory_management_struct.e820[i].address + memory_management_struct.e820[i].length) >> PAGE_2M_SHIFT) << PAGE_2M_SHIFT;
        if (end <= start)
            continue;

        // zone init

        z = memory_management_struct.zones_struct + memory_management_struct.zones_size;
        memory_management_struct.zones_size++;

        z->zone_start_address = start;
        z->zone_end_address = end;
        z->zone_length = end - start;

        z->page_using_count = 0;
        z->page_free_count = (end - start) >> PAGE_2M_SHIFT;

        z->total_pages_link = 0;

        z->attribute = 0;
        z->GMD_struct = &memory_management_struct;

        z->pages_length = (end - start) >> PAGE_2M_SHIFT;
        z->pages_group = (struct Page *)(memory_management_struct.pages_struct + (start >> PAGE_2M_SHIFT));

        // page init
        p = z->pages_group;
        for (j = 0; j < z->pages_length; j++, p++)
        {
            p->zone_struct = z;
            p->PHY_address = (start + PAGE_2M_SIZE * j);
            p->attribute = 0;

            p->reference_count = 0;
            // 页的创建时间
            p->age = 0;
            /*除 >>6 = 64, 把对应的物理页的位图复位*/
            *(memory_management_struct.bits_map + ((p->PHY_address >> PAGE_2M_SHIFT) >> 6)) ^= 1UL << ((p->PHY_address >> PAGE_2M_SHIFT) % 64);
        }
    }

    /////////////init address 0 to page struct 0;
    /// because the memory_management_struct.e820[0]的范围小于2MB。所以0 ~ 2MB没有被上面的循环初始化
    /// 0 ~ 2MB 包含内核程序和多个数据段, 第一个物理页结构体也没有归于第一段zones_struct
    // 这里的代码有点问题。
    memory_management_struct.pages_struct->zone_struct = memory_management_struct.zones_struct;

    memory_management_struct.pages_struct->PHY_address = 0UL;

    set_page_attribute(memory_management_struct.pages_struct, PG_PTable_Maped | PG_Kernel_Init | PG_Kernel);
    memory_management_struct.pages_struct->reference_count = 1;
    memory_management_struct.pages_struct->age = 0;

    /////////////

    memory_management_struct.zones_length = (memory_management_struct.zones_size * sizeof(struct Zone) + sizeof(long) - 1) & (~(sizeof(long) - 1));
    // 显示三种结构的信息
    color_printk(ORANGE, BLACK, "bits_map:%#018lx,bits_size:%#018lx,bits_length:%#018lx\n", memory_management_struct.bits_map, memory_management_struct.bits_size, memory_management_struct.bits_length);
    color_printk(ORANGE, BLACK, "pages_struct:%#018lx,pages_size:%#018lx,pages_length:%#018lx\n", memory_management_struct.pages_struct, memory_management_struct.pages_size, memory_management_struct.pages_length);
    // color_printk(ORANGE, BLACK, "zones_struct:%#018lx,zones_size:%#018lx,zones_length:%#018lx\n", memory_management_struct.zones_struct, memory_management_struct.zones_size, memory_management_struct.zones_length);
    //  显示ZONE结构体具体信息
    ZONE_DMA_INDEX = 0;
    ZONE_NORMAL_INDEX = 0;
    ZONE_UNMAPED_INDEX = 0;

    for (i = 0; i < memory_management_struct.zones_size; i++)
    {
        struct Zone *z = memory_management_struct.zones_struct + i;
        // color_printk(ORANGE, BLACK, "zone_start_address:%#018lx,zone_end_address:%#018lx,zone_length:%#018lx,pages_group:%#018lx,pages_length:%#018lx\n", z->zone_start_address, z->zone_end_address, z->zone_length, z->pages_group, z->pages_length);

        // 4GB
        if (z->zone_start_address >= 0x100000000 && !ZONE_UNMAPED_INDEX)
            ZONE_UNMAPED_INDEX = i;
    }

    // color_printk(ORANGE, BLACK, "ZONE_DMA_INDEX:%d\tZONE_NORMAL_INDEX:%d\tZONE_UNMAPED_INDEX:%d\n", ZONE_DMA_INDEX, ZONE_NORMAL_INDEX, ZONE_UNMAPED_INDEX);
    //  给内存管理结构尾赋值，并且预留的一段内存空间防止越界访问, 字节单位
    ////need a blank to separate memory_management_struct
    memory_management_struct.end_of_struct = (unsigned long)((unsigned long)memory_management_struct.zones_struct + memory_management_struct.zones_length + sizeof(long) * 32) & (~(sizeof(long) - 1));

    // color_printk(ORANGE, BLACK, "start_code:%#018lx,end_code:%#018lx,end_data:%#018lx,start_brk:%#018lx,end_of_struct:%#018lx\n", memory_management_struct.start_code, memory_management_struct.end_code, memory_management_struct.end_data, memory_management_struct.start_brk, memory_management_struct.end_of_struct);

    i = PAGE_2M_ALIGN(Virt_To_Phy(memory_management_struct.end_of_struct)) >> PAGE_2M_SHIFT;
    /*初始化内核目前使用到的物理页结构体, 目前内核小于2MB*/
    /*此处 j 初始化为1 是应为第一个物理页已经被我们手动初始化了
        使用小于号的原因是因为处理i的过程中使用了向上对齐*/
    for (j = 1; j < i; j++)
    {
        struct Page *tmp_page = memory_management_struct.pages_struct + j;
        page_init(tmp_page, PG_PTable_Maped | PG_Kernel_Init | PG_Kernel);
        *(memory_management_struct.bits_map + ((tmp_page->PHY_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (tmp_page->PHY_address >> PAGE_2M_SHIFT) % 64;
        tmp_page->zone_struct->page_using_count++;
        tmp_page->zone_struct->page_free_count--;
    }
    /*清空页表项, 准确来说是清理第四级页表的第一个页表项，带来的结果是线性地址0开始的内存没有被映射，无法被使用*/
    /*不过0xffff800是映射的第256个表项, 所以内核程序可以继续运行*/
    Global_CR3 = Get_gdt();

    color_printk(INDIGO, BLACK, "Global_CR3: %#018lx\t", Global_CR3);
    color_printk(INDIGO, BLACK, "*Global_CR3: %#018lx\t", *Phy_To_Virt(Global_CR3) & (~0xff));
    color_printk(PURPLE, BLACK, "**Global_CR3: %#018lx\n", *Phy_To_Virt(*Phy_To_Virt(Global_CR3) & (~0xff)) & (~0xff));

    // 此处打印的bits_maps的位图会是 01, 这里表示0 ~ 2MB已经被占用。
    // zone_struct掌握的内存是 2MB ~ 510MB
    // color_printk(ORANGE, BLACK, "memory_management_struct.bits_map:%#018lx\tzone_struct->page_using_count:%d\tzone_struct->page_free_count:%d\n", *memory_management_struct.bits_map, memory_management_struct.zones_struct->page_using_count, memory_management_struct.zones_struct->page_free_count);

    // 一旦处理器执行完flush_tlb函数, 线性地址0处的页表映射便不复存在，
    // 此后内核程序只存在于线性地址0xffff800000000000之上
    // 目前task_init()无法运行了
    for (i = 0; i < 10; i++)
        *(Phy_To_Virt(Global_CR3) + i) = 0UL;

    flush_tlb();
}

/**
 * @brief 堆函数
 *
 * @param addr 堆的当前位置
 * @param len 要扩展的堆的长度
 * @return unsigned long
 */
unsigned long do_brk(unsigned long addr, unsigned long len)
{
    unsigned long *tmp = NULL;
    unsigned long *virtual = NULL;
    struct Page *p = NULL;
    unsigned long i = 0;

    for (i = addr; i < addr + len; i += PAGE_2M_SIZE)
    {
        tmp = Phy_To_Virt((unsigned long *)((unsigned long)current->mm->pgd & (~0xfffUL)) + ((i >> PAGE_GDT_SHIFT) & 0x1ff));
        if (*tmp == 0) // 这样比较可读性不好
        {
            virtual = kmalloc(PAGE_4K_SIZE, 0);
            memset(virtual, 0, PAGE_4K_SIZE);
            set_mpl4t(tmp, mk_mpl4t(Virt_To_Phy(virtual), PAGE_USER_GDT));
        }
        tmp = Phy_To_Virt((unsigned long *)(*tmp & (~0xfffUL)) + ((i >> PAGE_1G_SHIFT) & 0x1ff));
        if (*tmp == 0)
        {
            virtual = kmalloc(PAGE_4K_SIZE, 0);
            memset(virtual, 0, PAGE_4K_SIZE);
            set_pdpt(tmp, mk_pdpt(Virt_To_Phy(virtual), PAGE_USER_Dir));
        }
        tmp = Phy_To_Virt((unsigned long *)(*tmp & (~0xfffUL)) + ((i >> PAGE_2M_SHIFT) & 0x1ff));
        if (*tmp == 0)
        {
            p = alloc_pages(ZONE_NORMAL, 1, PG_PTable_Maped);
            if (p == NULL)
                return -ENOMEM;
            set_pdt(tmp, mk_pdt(p->PHY_address, PAGE_USER_Page));
        }
    }
    current->mm->end_brk = i;
    flush_tlb();
    return i;
}

unsigned long do_wp_page(unsigned )
