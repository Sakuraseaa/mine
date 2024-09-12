#include "basetype.h"
#include "msadsc_t.h"
#include "marea_t.h"
#include "memmgrob.h"
#include "memdivmer_t.h"

memmgrob_t glomm;

void init_memmgr()
{	
	msadsc_t* msa;
	uint_t need = 0;
	// init 物理页 结构
	init_msadsc();
	
	// init 内存区 结构
	init_memarea();
	
    // init_copy_pagesfvm();
	// 处理内存占用
	init_search_krloccupymm();
	
    // 合并内存页到内存区中
    init_merlove_mem();	// Error have a bug
	
	init_kmsob();

	// 测试代码需要迁移到映射之后
	// kmsob_new(34);
	
	// kmsob_new(38);
	
	#if 0
	// 这里申请了9kb,需要占用3个页面,也就是12kb,得到是16kb, 16 - 12 = 4;
	// 可以尝试把 多余的个页面，挂载到小的链表头上
	msa = mm_division_pages(&glomm, 9, &need, MA_TYPE_HWAD, 0);
	
	// 4, 8, 16，13 = 4KB整数页 这些2的整数倍的页，直接归还尝试合并就好
	// 9, 17 这些非整数倍的页，可以根据得到的12kb(16kb, 4kb)，20kb(32kb， 12kb),直接分解12kb,16kb挂载到空闲链表上
	// 然后进行一次内存整合操作
	mm_merge_pages(&glomm, msa, need);
	#endif

	return;
}