#ifndef _MSADSC_T_H_
#define _MSADSC_T_H_


#define PAGPHYADR_SZLSHBIT (12)
#define MSAD_PAGE_MAX (8)
#define MSA_SIZE (1 << PAGPHYADR_SZLSHBIT)

#define MF_OLKTY_INIT (0)
#define MF_OLKTY_ODER (1)   /* 作为一个桶元素的开始物理块，oldlink链接着桶元素的最后一个物理块 */
#define MF_OLKTY_BAFH (2)   /* 作为一个桶元素的结束, 标志自己这块物理页没有使用odlink连接后续物理块了，自己是最后一个块，自己(oldlink)直接挂载到桶链表上. */
#define MF_OLKTY_TOBJ (3)

#define MF_LSTTY_LIST (0)
#define MF_MOCTY_FREE (0)
#define MF_MOCTY_KRNL (1)
#define MF_MOCTY_USER (2)
#define MF_MRV1_VAL (0)
#define MF_UINDX_INIT (0)
#define MF_UINDX_MAX (0xffffff)
#define MF_MARTY_INIT (0)
#define MF_MARTY_HWD (1)
#define MF_MARTY_KRL (2)
#define MF_MARTY_PRC (3)
#define MF_MARTY_SHD (4)

//内存空间地址描述符标志
typedef struct s_MSADFLGS
{   /* olkty和lstty是什么作用 */
    u64_t mf_olkty:2;     //  指明 md_odlink 变量的挂载类型
    u64_t mf_lstty:1;     //是否挂入链表 list type?
    u64_t mf_mocty:2;     //分配类型,被谁占用了,内核,应用,空闲 memory allocation type?
    u64_t mf_marty:3;     // memory area type
    u64_t mf_refcnt:56;   // reference count
}__attribute__((packed)) msadflgs_t; 

#define  PAF_NO_ALLOC (0)
#define  PAF_ALLOC (1)
#define  PAF_NO_SHARED (0)
#define  PAF_SHARED (1)
#define  PAF_NO_SWAP (0)
#define  PAF_NO_CACHE (0)
#define  PAF_NO_KMAP (0)
#define  PAF_NO_LOCK (0)
#define  PAF_NO_DIRTY (0)
#define  PAF_NO_BUSY (0)
#define  PAF_RV2_VAL (0)
#define  PAF_INIT_PADRS (0)
//物理地址和标志  
typedef struct s_PHYADRFLGS
{
    u64_t paf_alloc:1;     //分配位
    u64_t paf_shared:1;    //共享位
    u64_t paf_swap:1;      //交换位
    u64_t paf_cache:1;     //缓存位
    u64_t paf_kmap:1;      //映射位
    u64_t paf_lock:1;      //锁定位
    u64_t paf_dirty:1;     //脏位
    u64_t paf_busy:1;      //忙位
    u64_t paf_rv2:4;       //保留位
    u64_t paf_padrs:52;    //页物理地址位
}__attribute__((packed)) phyadrflgs_t;

/* 内存空间地址描述符 - Memory Space Address Descriptor */
typedef struct s_MSADSC
{
    list_n_t md_list;
    spinlock_t md_lock;         /* 保护自身的自旋锁 */
    msadflgs_t md_cntflgs;      /* 内存区描述符标志 */
    phyadrflgs_t md_phyadrs;    /* 物理地址和标志 */
    void* md_odlink;            /* 相邻且相同大小msadsc的指针 */
}msadsc_t;

#endif // _MSADSC_T_H_