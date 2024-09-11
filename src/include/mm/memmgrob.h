#ifndef _MEMMGROB_H_
#define _MEMMGROB_H_

#include "basetype.h"
#include "msadsc_t.h"
#include "marea_t.h"
#include "kmsob_t.h"

typedef struct s_MEMMGROB
{
    list_h_t mo_list;
    spinlock_t mo_lock;        //保护自身自旋锁
    uint_t mo_stus;            //状态 status
    uint_t mo_flgs;            //标志 flags
    u64_t mo_memsz;            //内存大小
    u64_t mo_maxpages;         //内存最大页面数
    u64_t mo_freepages;        //内存最大空闲页面数
    u64_t mo_alocpages;        //内存最大分配页面数
    u64_t mo_resvpages;        //内存保留页面数 resever
    u64_t mo_horizline;        //内存分配水位线 horizon line
    // phymmarge_t* mo_pmagestat; //内存空间布局结构指针
    // u64_t mo_pmagenr;
    msadsc_t* mo_msadscstat;   //内存页面结构指针 - 内存空间地址描述符起始地址
    u64_t mo_msanr;
    memarea_t* mo_mareastat;   //内存区结构指针 
    u64_t mo_mareanr;

	kmsobmgrhed_t mo_kmsobmgr;  // 统筹所有内存池的结构，内存池用于分配小内存
}memmgrob_t; // memory management global resource object


#endif // _MEMMGROB_H_