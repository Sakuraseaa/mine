#ifndef _KRLVADRSMEM_T_H_
#define _KRLVADRSMEM_T_H_

#include "basetype.h"
#include "lib.h"
#include "atomic.h"
#include "spinlock.h"

#define RET4PT_PML4EVDR (1)
#define RET4PT_PDPTEVDR (2)
#define RET4PT_PDETEVDR (3)
#define RET4PT_PTETEVDR (4)
#define RET4PT_PFAMEPDR (5)

#define VMAP_MIN_SIZE (MSA_SIZE)

#define KMBOX_CACHE_MAX (0x1000)
#define KMBOX_CACHE_MIN (0x40)
typedef struct KVMCOBJMGR
{
	spinlock_t kom_lock;
	u32_t kom_flgs;
	uint_t kom_kvmcobjnr;
	list_h_t kom_kvmcohead;
	uint_t kom_kvmcocahenr;
	list_h_t kom_kvmcocahe;
	uint_t kom_kvmcodelnr;
	list_h_t kom_kvmcodelhead;
}kvmcobjmgr_t;

typedef struct KVMEMCOBJ
{
	list_h_t kco_list;
	spinlock_t kco_lock;
	u32_t kco_cont;
	u32_t kco_flgs;
	u32_t kco_type;
	uint_t kco_msadnr;
	list_h_t kco_msadlst;
	void* kco_filenode;
	void* kco_pager;
	void* kco_extp;
}kvmemcobj_t;


typedef struct KVMEMCBOXMGR 
{
	list_h_t kbm_list;
	spinlock_t kbm_lock;
	u64_t kbm_flgs;
	u64_t kbm_stus;	
	uint_t kbm_kmbnr;		// kvmemcbox_t结构个数
	list_h_t kbm_kmbhead;
	uint_t kbm_cachenr;
	uint_t kbm_cachemax;
	uint_t kbm_cachemin;
	list_h_t kbm_cachehead;
	void* kbm_ext;
}kvmemcboxmgr_t;
// 页面盒子的头，用于挂载kvmemcbox_t结构, 全局的数据结构, 管理所有的kvmemcbox_t

typedef struct KVMEMCBOX 
{
	list_n_t kmb_list;
	spinlock_t kmb_lock;
	refcount_t kmb_cont;		// 共享计数器
	u64_t kmb_flgs;
	u64_t kmb_stus;
	u64_t kmb_type;
	uint_t kmb_msanr;			// 多少个msadsc_t
	list_h_t kmb_msalist;		// 挂载msadsc_t结构的链表
	kvmemcboxmgr_t* kmb_mgr;	// 指向上层结构
	void* kmb_filenode;			// 指向文件节点描述符
	void* kmb_pager;			// 指向分页器
	void* kmb_ext;				// 自身扩展数据指针
}kvmemcbox_t;	// 页面盒子

typedef struct VASLKNODE
{
	u32_t  vln_color;
	u32_t  vln_flags;
	void*  vln_left;
	void*  vln_right;
	void*  vln_prev;
	void*  vln_next;
}vaslknode_t;

typedef struct TESTSTC
{
	list_h_t tst_list;
	adr_t    tst_vadr;
	size_t   tst_vsiz;
	uint_t   tst_type;
	uint_t   tst_lime;
}teststc_t;

typedef struct PGTABPAGE
{
	spinlock_t ptp_lock;
	list_h_t   ptp_msalist;
	uint_t     ptp_msanr;
}pgtabpage_t; // page table page

typedef struct KMVARSDSC
{
	spinlock_t kva_lock;
	u32_t  kva_maptype;  // 映射类型
	list_h_t kva_list;
	u64_t  kva_flgs;
	u64_t  kva_limits;
	vaslknode_t kva_lknode;
	void*  kva_mcstruct;        // 指向它的上层结构
	adr_t  kva_start;           // 虚拟地址的开始
	adr_t  kva_end;             // 虚拟地址的结束
	kvmemcbox_t* kva_kvmbox;    // 管理这个结构映射的物理页面
	void*  kva_kvmcobj;
}kmvarsdsc_t; // 虚拟地址区间 kernel memory virtual address descriptor

typedef struct KVIRMEMADRS
{
	spinlock_t kvs_lock;
	u64_t  kvs_flgs;
	uint_t kvs_kmvdscnr;
	kmvarsdsc_t* kvs_startkmvdsc;
	kmvarsdsc_t* kvs_endkmvdsc;	
	kmvarsdsc_t* kvs_krlmapdsc;
	kmvarsdsc_t* kvs_krlhwmdsc;
	kmvarsdsc_t* kvs_krlolddsc;
	adr_t kvs_isalcstart;
	adr_t kvs_isalcend;
	void* kvs_privte;
	void* kvs_ext;
	list_h_t kvs_testhead;
	uint_t   kvs_tstcnr;
	uint_t   kvs_randnext;
	pgtabpage_t kvs_ptabpgcs;
	kvmcobjmgr_t kvs_kvmcomgr;
	kvmemcboxmgr_t kvs_kvmemcboxmgr;
}kvirmemadrs_t; // kernel virtual memory address 管理整个虚拟地址空间的kmvarsdsc_t结构

typedef struct s_MMADRSDSC mmadrsdsc_t;

typedef struct s_VIRMEMADRS
{
	spinlock_t vs_lock;
	u32_t  vs_resalin;
	list_h_t vs_list;			// 链接虚拟地址区间
	uint_t vs_flgs;
	uint_t vs_kmvdscnr;         // 多少个虚拟地址区间
	mmadrsdsc_t* vs_mm;         // 指向它的上层的数据结构
	kmvarsdsc_t* vs_startkmvdsc; // 开始的虚拟地址区间
	kmvarsdsc_t* vs_endkmvdsc;   // 结束的虚拟地址区间
	kmvarsdsc_t* vs_currkmvdsc;  // 当前的虚拟地址区间
	kmvarsdsc_t* vs_krlmapdsc;
	kmvarsdsc_t* vs_krlhwmdsc;
	kmvarsdsc_t* vs_krlolddsc;
	adr_t vs_isalcstart;         // 能分配的开始虚拟地址
	adr_t vs_isalcend;           // 能分配的结束虚拟地址
	void* vs_privte;             // 私有数据指针
	void* vs_ext;				 // 扩展数据指针
}virmemadrs_t; // virtual memory address

// 常规操作就是把同一个物理内存页面映射到不同的虚拟地址区间
typedef struct s_MMADRSDSC
{
	spinlock_t msd_lock;
	list_h_t msd_list;
	uint_t msd_flag;
	uint_t msd_stus;
	uint_t msd_scount;  // 计数，该结构可能被共享
	// sem_t  msd_sem;     // 信号量
	// mmudsc_t msd_mmu;   // 管理 MMU相关信息
	virmemadrs_t msd_virmemadrs;    // 虚拟地址空间

	adr_t msd_stext;	// 应用的指令区的开始，结束地址
	adr_t msd_etext;
	adr_t msd_sdata;	// 应用的数据区的开始，结束地址
	adr_t msd_edata;
	adr_t msd_sbss;
	adr_t msd_ebss;
	adr_t msd_sbrk;		// 应用的堆区的开始，结束地址
	adr_t msd_ebrk;
}mmadrsdsc_t; // 管理进程的虚拟地址, memory address descriptor


#define VADSZ_ALIGN(x) ALIGN(x,0x1000)
#define KVMCOBJ_FLG_DELLPAGE (1)
#define KVMCOBJ_FLG_UDELPAGE (2)

// #endif


#endif // _KRLVADRSMEM_T_H_