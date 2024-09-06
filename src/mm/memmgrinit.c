#include "basetype.h"
#include "msadsc_t.h"
#include "marea_t.h"
#include "memmgrob.h"
#include "memdivmer_t.h"

memmgrob_t glomm;

void init_memmgr()
{	
	uint_t need = 0;
	// init 物理页 结构
	init_msadsc();
	
	// init 内存区 结构
	init_memarea();
	
    // init_copy_pagesfvm();
	// 处理内存占用
	init_search_krloccupymm();
	
    // 合并内存页到内存区中
    init_merlove_mem();
	
	mm_division_pages(&glomm, 4, &need, MA_TYPE_HWAD, 1);
	
	return;
}