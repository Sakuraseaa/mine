#ifndef _MEMMGROB_H_
#define _MEMMGROB_H_

typedef struct s_MEMMGROB
{
    list_h_t mo_list;
    spinlock_t mo_lock;        //保护自身自旋锁
    uint_t mo_stus;            //状态 status
    uint_t mo_flgs;            //标志 flags
	// sem_t mo_sem;
    u64_t mo_memsz;            //内存大小
    u64_t mo_maxpages;         //内存最大页面数
    u64_t mo_freepages;        //内存最大空闲页面数
    u64_t mo_alocpages;        //内存最大分配页面数
    u64_t mo_resvpages;        //内存保留页面数 resever
    u64_t mo_horizline;        //内存分配水位线 horizon line
    // phymmarge_t* mo_pmagestat; //内存空间布局结构指针
    // u64_t mo_pmagenr;
    msadsc_t* mo_msadscstat;   // 内存页面结构指针 - 内存空间地址描述符起始地址
    u64_t mo_msanr;            // 内存空间地址描述符总数
    memarea_t* mo_mareastat;   // 内存区结构指针 
    u64_t mo_mareanr;          // 内存区总数

	kmsobmgrhed_t mo_kmsobmgr;  // 统筹所有内存池的结构，内存池用于分配小内存
	void* mo_privp;
	void* mo_extp;
}mmgro_t; // memory management global resource object


#endif // _MEMMGROB_H_