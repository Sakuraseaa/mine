#ifndef _KRLVADRSMEM_T_H_
#define _KRLVADRSMEM_T_H_

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
	list_h_t kbm_kmbhead;	// 挂载kvmemcbox_t结构的链表
	uint_t kbm_cachenr;		// 缓存空闲kvmemcbox_t结构的个数
	uint_t kbm_cachemax;	// 最大缓存个数，超过了就释放
	uint_t kbm_cachemin;	// 最小缓存个数
	list_h_t kbm_cachehead;	// 缓存kvmemcbox_t结构的链表
	void* kbm_ext;			// 扩展数据指针
}kvmemcboxmgr_t; // 页面盒子管理头, 管理所有的页面盒子




// 页面盒子的头，用于挂载kvmemcbox_t结构, 全局的数据结构, 管理所有的kvmemcbox_t

// 每段虚拟地址区间，在用到的时候都会映射对应的物理页面
//
//一般虚拟地址区间是和文件对应的数据相关联的。
// 比如把一个文件映射到进程的虚拟地址空间中，只需要在内存页面中保留一份共享文件，多个程序就都可以共享它。
//
//常规操作就是把同一个物理内存页面映射到不同的虚拟地址区间，由此连接物理页面和虚拟区间的结构体应运而生
// kvmemcbox_t Kernel Virtual Memory Container Box
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
}kvmemcbox_t;	// 页面盒子, Kernel Virtual Memory Container Box

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

// 段权限
typedef struct VMA_TO_FILE_FLAGS_CORE
{
		u64_t read:1;
		u64_t write:1;
		u64_t execute:1;
		u64_t rsv:61;
} __attribute__((packed)) vtfflags_core_t;

typedef struct VMA_TO_FILE_FLAGS
{
    union
    {
        vtfflags_core_t flags;
        u64_t entry;
    } __attribute__((packed));
} __attribute__((packed)) vtfflags_t;

typedef struct VMA_TO_FILE{
    u64_t vtf_position; // 要加载的数据在文件中的起始地址
    u64_t vtf_size;	// 要加载的数据尺寸
	u64_t vtf_alread_load_size; // 本段已经有多少个字节被加载到了正确位置？
    void* vtf_file;	//file_t指针
    vtfflags_t vtf_flag;
} vma_to_file_t;

typedef struct KMVARSDSC
{
	spinlock_t kva_lock;
	u32_t  kva_maptype;  // 映射类型
	list_n_t kva_list;
	u64_t  kva_flgs;	// 0普通内存, 1栈内存，2堆内存，默认普通内存
	u64_t  kva_limits;
	vaslknode_t kva_lknode;
	void*  kva_mcstruct;        // 指向它的上层结构
	adr_t  kva_start;           // 虚拟地址的开始
	adr_t  kva_end;             // 虚拟地址的结束
	kvmemcbox_t* kva_kvmbox;    // 管理这段 虚拟区间 已经被映射的物理页面 
	void*  kva_kvmcobj;
	vma_to_file_t* kva_vir2file;	// 完成虚拟区间和文件之间的关联
}kmvarsdsc_t; // 虚拟地址区间 kernel memory virtual address descriptor, 该结构类似与 vm_area_struct virtual memory area struct

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

typedef struct s_MMADRSDSC mmdsc_t;

typedef struct s_VIRMEMADRS
{
	spinlock_t vs_lock;
	u32_t  vs_resalin;
	list_h_t vs_list;			// 链接虚拟地址区间, 虚拟区间一个一个地挂载这条链表上，区间挂载的前后顺序
								// 一定是虚拟区间所管理地址从小到大排列的, 这是一条顺序有关列表，维护的时候需要特别注意
	uint_t vs_flgs;
	uint_t vs_kmvdscnr;         // 多少个虚拟地址区间
	mmdsc_t* vs_mm;         // 指向它的上层的数据结构
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
	mmudsc_t msd_mmu;   // 管理 MMU相关信息, 其中记载了页表，已经页表占用的内存。
	virmemadrs_t msd_virmemadrs;    // 虚拟地址空间

	adr_t start_code;	// 应用的指令区的开始，结束地址
	adr_t end_code;
	adr_t start_data;	// 应用的数据区的开始，结束地址
	adr_t end_data;
	adr_t start_rodata;	// 应用的数据区的开始，结束地址
	adr_t end_rodata;
	adr_t start_bss;
	adr_t end_bss;
	adr_t start_brk;		// 应用的堆区的开始，结束地址
	adr_t end_brk;
	adr_t start_stack, stack_length; /* 应用栈的开始 和 长度*/

}mmdsc_t; // 管理进程的虚拟地址, memory address descriptor, 类似与Linux中的 struct mm

#define VADSZ_ALIGN(x) ALIGN(x,0x1000) /* 4Kb 对齐 */
#define KVMCOBJ_FLG_DELLPAGE (1)
#define KVMCOBJ_FLG_UDELPAGE (2)


#endif // _KRLVADRSMEM_T_H_