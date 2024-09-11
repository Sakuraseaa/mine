#ifndef _MEMAREA_T_H
#define _MEMAREA_T_H

#include "lib.h"
#include "basetype.h"
#include "msadsc_t.h"
#define MMSTUS_ERR (0)
#define MMSTUS_OK (1)


// 内存区域中的辅助结构，用于管理LRU（最近最少使用）算法的链表。
typedef struct s_ARCLST
{
	list_h_t al_lru1;
	list_h_t al_lru2;
	uint_t al_lru1nr;
	uint_t al_lru2nr;
}arclst_t; // allocation replacement cache list

// 用于内存分配结果的结构体，包含了分配的起始地址、大小、物理地址等信息。
typedef struct s_MMAFRETS
{
	msadsc_t* mat_fist;
	uint_t mat_sz;
	uint_t mat_phyadr;
	u16_t mat_runmode;
	u16_t mat_gen;
	u32_t mat_mask;
}__attribute__((packed)) mmafrets_t;


// 内存分配函数对象，定义了一组函数指针，用于不同内存区域的操作，如初始化、释放、分配、回收
struct s_MEMAREA;
typedef struct s_MAFUNCOBJS
{
	mmstus_t (*mafo_init)(struct s_MEMAREA* memarea,void* valp,uint_t val);
	mmstus_t (*mafo_exit)(struct s_MEMAREA* memarea);
	mmstus_t (*mafo_aloc)(struct s_MEMAREA* memarea,mmafrets_t* mafrspack,void* valp,uint_t val);
	mmstus_t (*mafo_free)(struct s_MEMAREA* memarea,mmafrets_t* mafrspack,void* valp,uint_t val);
	mmstus_t (*mafo_recy)(struct s_MEMAREA* memarea,mmafrets_t* mafrspack,void* valp,uint_t val);
}mafuncobjs_t;

#define BAFH_STUS_INIT 0
#define BAFH_STUS_ONEM 1
#define BAFH_STUS_DIVP 2
#define BAFH_STUS_DIVM 3
typedef struct s_BAFHLST
{
	spinlock_t af_lock;
	u32_t af_stus;
	uint_t af_oder;		 	// 页面数的位移量 order
	uint_t af_oderpnr;		// 页面数, 该bafh list每一个节点挂载的页面数, 如果oder为2，那这个数就是 1<<2=4  order-page-number
	uint_t af_fobjnr;		// 多少个空闲msadsc_t结构，即空闲页面 free-object-number
	//uint_t af_aobjnr;
	uint_t af_mobjnr; 		// 此结构的msadsc_t结构总数，即此结构总页面 msadsc_t object number
	uint_t af_alcindx; 		// 此结构的分配计数 
	uint_t af_freindx;		// 此结构的释放计数
	list_h_t af_frelst; 	// 挂载此结构的空闲msadsc_t结构 free-list
	list_h_t af_alclst; 	// 挂载此结构已经分配的msadsc_t结构 allocated-list
	list_h_t af_ovelst;
}bafhlst_t; // block alloc free head list


#define MDIVMER_ARR_LMAX 52
#define MDIVMER_ARR_BMAX 11
#define MDIVMER_ARR_OMAX 9
typedef struct s_MEMDIVMER
{
	spinlock_t dm_lock;
	u32_t dm_stus;
	uint_t dm_dmmaxindx;
	uint_t dm_phydmindx;
	uint_t dm_predmindx;
	uint_t dm_divnr;	 //内存分配次数 - 分配内存
	uint_t dm_mernr;	 //内存合并次数 - 释放内存
	//bafhlst_t dm_mdmonelst[MDIVMER_ARR_OMAX];
	//bafhlst_t dm_mdmblklst[MDIVMER_ARR_BMAX];
	bafhlst_t dm_mdmlielst[MDIVMER_ARR_LMAX];
	bafhlst_t dm_onemsalst;
}memdivmer_t; // 内存分配结构体, 管理内存分配和合并操作的结构体


#define MA_TYPE_INIT 0
#define MA_TYPE_HWAD 1
#define MA_TYPE_KRNL 2
#define MA_TYPE_PROC 3
#define MA_TYPE_SHAR 4
#define MEMAREA_MAX 4
#define MA_HWAD_LSTART 0
#define MA_HWAD_LSZ 0x2000000
#define MA_HWAD_LEND (MA_HWAD_LSTART + MA_HWAD_LSZ-1)
#define MA_KRNL_LSTART 0x2000000
#define MA_KRNL_LSZ (0x10000000 - MA_HWAD_LSZ)
#define MA_KRNL_LEND (MA_KRNL_LSTART + MA_KRNL_LSZ-1)
#define MA_PROC_LSTART 0x10000000
#define MA_PROC_LSZ (0xffffffffffffffff - MA_PROC_LSTART)
#define MA_PROC_LEND (MA_PROC_LSTART + MA_PROC_LSZ)
//0x400000000  0x40000000
typedef struct s_MEMAREA
{
	list_h_t ma_list;			//内存区自身的链表
	spinlock_t ma_lock;			//保护内存区的自旋锁
	uint_t ma_stus;			 	//内存区的状态
	uint_t ma_flgs;  		
	uint_t ma_type;				//内存区的类型
	// sem_t ma_sem;    		//内存区的信号量
	// wait_l_head_t ma_waitlst;//内存区的等待队列
	uint_t ma_maxpages;			//内存区总的页面数
	uint_t ma_allocpages;  
	uint_t ma_freepages;		//内存区空闲的页面数
	uint_t ma_resvpages;		//内存区保留的页面数
	uint_t ma_horizline;		//内存区分配时的水位线
	adr_t ma_logicstart;		//内存区开始地址
	adr_t ma_logicend;			//内存区结束地址
	uint_t ma_logicsz;			//内存区大小 logitcal end
	adr_t ma_effectstart;		// effective start
	adr_t ma_effectend;			// effective - end
	uint_t ma_effectsz;	 		// effective - size
	list_h_t ma_allmsadsclst; 	// all msadsc list 挂载此内存区的物理页
	uint_t ma_allmsadscnr;
	arclst_t ma_arcpglst;		// page list
	mafuncobjs_t ma_funcobj; 	// function objcts 功能对象
	memdivmer_t ma_mdmdata;		// memory divider merge data
	void* ma_privp; 			// memory area private pointer
}memarea_t;
	/*
	*这个结构至少占用一个页面，当然
	*也可以是多个连续的的页面，但是
	*该结构从第一个页面的首地址开始
	*存放，后面的空间用于存放实现分
	*配算法的数据结构，这样每个区可
	*方便的实现不同的分配策略，或者
	*有天你觉得我的分配算法是渣渣，
	*完全可以替换mafuncobjs_t结构
	*中的指针，指向你的函数。
	*/

void init_memarea();
void init_merlove_mem();
#endif