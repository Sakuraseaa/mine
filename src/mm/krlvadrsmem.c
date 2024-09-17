#include "krlvadrsmem_t.h"
#include "halplatform_t.h"
#include "printk.h"
#include "memory.h"
#include "kmsob_t.h"
#include "lib.h"
#include "basetype.h"
// void teststc_t_init(teststc_t *initp)
// {
// 	list_init(&initp->tst_list);
// 	initp->tst_vadr = 0;
// 	initp->tst_vsiz = 0;
// 	initp->tst_type = 0;
// 	initp->tst_lime = 0;
// 	return;
// }

// teststc_t *new_teststc()
// {
// 	teststc_t *t = (teststc_t *)kmsob_new(sizeof(teststc_t));
// 	if (NULL == t)
// 	{
// 		return NULL;
// 	}
// 	teststc_t_init(t);
// 	return t;
// }

// void del_teststc(teststc_t *delstc)
// {

// 	if ((NULL != delstc))
// 	{
// 		teststc_t_init(delstc);
// 		if (TRUE == kmsob_delete((void *)delstc, sizeof(teststc_t)))
// 		{
// 			return;
// 		}
// 	}
// 	system_error("del_teststc err\n");
// 	return;
// }

// void add_new_teststc(adr_t vadr, size_t vsiz)
// {
// 	if (NULL == vadr || 1 > vsiz)
// 	{
// 		system_error("add_new_teststc parm err\n");
// 	}
// 	teststc_t *t = NULL;
// 	t = new_teststc();
// 	if (NULL == t)
// 	{
// 		system_error("add_new_teststc new_teststc NULL\n");
// 	}
// 	t->tst_vadr = vadr;
// 	t->tst_vsiz = vsiz;
// 	list_add(&t->tst_list, &krlvirmemadrs.kvs_testhead);
// 	krlvirmemadrs.kvs_tstcnr++;
// 	return;
// }

void vaslknode_t_init(vaslknode_t *initp)
{
	if (NULL == initp)
	{
		system_error("vaslknode_t_init pram err\n");
	}
	initp->vln_color = 0;
	initp->vln_flags = 0;
	initp->vln_left = NULL;
	initp->vln_right = NULL;
	initp->vln_prev = NULL;
	initp->vln_next = NULL;
	return;
}

// void pgtabpage_t_init(pgtabpage_t *initp)
// {
// 	knl_spinlock_init(&initp->ptp_lock);
// 	list_init(&initp->ptp_msalist);
// 	initp->ptp_msanr = 0;
// 	return;
// }

void virmemadrs_t_init(virmemadrs_t *initp)
{
	if (NULL == initp)
	{
		return;
	}
	spin_init(&initp->vs_lock);
	initp->vs_resalin = 0;
	list_init(&initp->vs_list);
	initp->vs_flgs = 0;
	initp->vs_kmvdscnr = 0;
	initp->vs_mm = NULL;
	initp->vs_startkmvdsc = 0;
	initp->vs_endkmvdsc = NULL;
	initp->vs_currkmvdsc = NULL;
	initp->vs_krlmapdsc = NULL;
	initp->vs_krlhwmdsc = NULL;
	initp->vs_krlolddsc = NULL;
	initp->vs_isalcstart = 0;
	initp->vs_isalcend = 0;
	initp->vs_privte = 0;
	initp->vs_ext = 0;
	return;
}

void kmvarsdsc_t_init(kmvarsdsc_t *initp)
{
	if (NULL == initp)
	{
		system_error("kmvarsdsc_t_init pram err\n");
	}
	spin_init(&initp->kva_lock);
	initp->kva_maptype = 0;
	list_init(&initp->kva_list);
	initp->kva_flgs = 0;
	initp->kva_limits = 0;
	vaslknode_t_init(&initp->kva_lknode);
	initp->kva_mcstruct = NULL;
	initp->kva_start = 0;
	initp->kva_end = 0;
	initp->kva_kvmbox = NULL;
	initp->kva_kvmcobj = NULL;
	return;
}

// void kvirmemadrs_t_init(kvirmemadrs_t *initp)
// {
// 	if (NULL == initp)
// 	{
// 		system_error("kvirmemadrs_t_init pram err\n");
// 	}
// 	spin_init(&initp->kvs_lock);
// 	initp->kvs_flgs = 0;
// 	initp->kvs_kmvdscnr = 0;
// 	initp->kvs_startkmvdsc = NULL;
// 	initp->kvs_endkmvdsc = NULL;
// 	initp->kvs_krlmapdsc = NULL;
// 	initp->kvs_krlhwmdsc = NULL;
// 	initp->kvs_krlolddsc = NULL;
// 	initp->kvs_isalcstart = 0;
// 	initp->kvs_isalcend = 0;
// 	initp->kvs_privte = NULL;
// 	initp->kvs_ext = NULL;
// 	list_init(&initp->kvs_testhead);
// 	initp->kvs_tstcnr = 0;
// 	initp->kvs_randnext = 1;
// 	pgtabpage_t_init(&initp->kvs_ptabpgcs);
// 	kvmcobjmgr_t_init(&initp->kvs_kvmcomgr);
// 	kvmemcboxmgr_t_init(&initp->kvs_kvmemcboxmgr);
// 	return;
// }

kmvarsdsc_t *new_kmvarsdsc()
{
	kmvarsdsc_t *kmvdc = NULL;
	kmvdc = (kmvarsdsc_t *)kmsob_new(sizeof(kmvarsdsc_t));
	if (NULL == kmvdc)
	{
		return NULL;
	}
	kmvarsdsc_t_init(kmvdc);
	return kmvdc;
}

bool_t del_kmvarsdsc(kmvarsdsc_t *delkmvd)
{
	if (NULL == delkmvd)
	{
		return FALSE;
	}
	return kmsob_delete((void *)delkmvd, sizeof(kmvarsdsc_t));
}

// virmemadrs_t *new_virmemadrs()
// {
// 	virmemadrs_t *vmdsc = NULL;
// 	vmdsc = (virmemadrs_t *)kmsob_new(sizeof(virmemadrs_t));
// 	if (NULL == vmdsc)
// 	{
// 		return NULL;
// 	}
// 	virmemadrs_t_init(vmdsc);
// 	return vmdsc;
// }

// bool_t del_virmemadrs(virmemadrs_t *vmdsc)
// {
// 	if (NULL == vmdsc)
// 	{
// 		return FALSE;
// 	}
// 	return kmsob_delete((void *)vmdsc, sizeof(virmemadrs_t));
// }

// void kvma_seting_kvirmemadrs(kvirmemadrs_t *kvma)
// {
// 	kmvarsdsc_t *kmvdc = NULL;
// 	if (NULL == kvma)
// 	{
// 		system_error("kvma_seting_kvirmemadrs parm err\n");
// 	}
// 	kmvdc = new_kmvarsdsc();
// 	if (NULL == kmvdc)
// 	{
// 		system_error("kvma_seting_kvirmemadrs nomem err\n");
// 	}
// 	kvma->kvs_isalcstart = KRNL_VIRTUAL_ADDRESS_START + KRNL_MAP_VIRTADDRESS_SIZE;
// 	kvma->kvs_isalcend = KRNL_VIRTUAL_ADDRESS_END;
// 	kmvdc->kva_start = KRNL_VIRTUAL_ADDRESS_START;
// 	kmvdc->kva_end = KRNL_VIRTUAL_ADDRESS_START + KRNL_MAP_VIRTADDRESS_SIZE;
// 	kmvdc->kva_mcstruct = kvma;
// 	kvma->kvs_startkmvdsc = kmvdc;
// 	kvma->kvs_endkmvdsc = kmvdc;
// 	kvma->kvs_krlmapdsc = kmvdc;
// 	kvma->kvs_kmvdscnr++;
// 	return;
// }

bool_t kvma_inituserspace_virmemadrs(virmemadrs_t *vma)
{
	kmvarsdsc_t *kmvdc = NULL, *stackkmvdc = NULL;
	if (NULL == vma) {
		return FALSE;
	}

	kmvdc = new_kmvarsdsc(); // 分配一个 new_kmvarsdsc(); 
	if (NULL == kmvdc) {
		return FALSE;
	}

	stackkmvdc = new_kmvarsdsc(); // 申请一个栈的虚拟内存控制结构
	if (NULL == stackkmvdc) {
		del_kmvarsdsc(kmvdc);
		return FALSE;
	}

	// 虚拟区间开始地址0x1000
	kmvdc->kva_start = USER_VIRTUAL_ADDRESS_START + 0x1000;
	kmvdc->kva_end = kmvdc->kva_start + 0x4000;
	kmvdc->kva_mcstruct = vma;

	// 栈虚拟区间开始地址 
	stackkmvdc->kva_start = PAGE_4K_ALIGN(USER_VIRTUAL_ADDRESS_END - 0x40000000);
	stackkmvdc->kva_end = USER_VIRTUAL_ADDRESS_END;
	stackkmvdc->kva_mcstruct = vma;

	// knl_spinlock(&vma->vs_lock);
	vma->vs_isalcstart = USER_VIRTUAL_ADDRESS_START;
	vma->vs_isalcend = USER_VIRTUAL_ADDRESS_END;
	vma->vs_startkmvdsc = kmvdc;		// 设置虚拟地址空间的开始区间为kmvdc
	vma->vs_endkmvdsc = stackkmvdc; // 设置虚拟空间结束地址为栈区
	list_add(&vma->vs_list, &kmvdc->kva_list);
	list_add(&vma->vs_list, &stackkmvdc->kva_list);
	vma->vs_kmvdscnr += 2;
	// knl_spinunlock(&vma->vs_lock);
	return TRUE;
}
void mmadrsdsc_t_init(mmadrsdsc_t* initp)
{
	if(NULL == initp)
	{
		return;
	}
	spin_init(&initp->msd_lock);
	list_init(&initp->msd_list);
	initp->msd_flag = 0;
	initp->msd_stus = 0;
	initp->msd_scount = 0;
	// krlsem_t_init(&initp->msd_sem);
	// krlsem_set_sem(&initp->msd_sem, SEM_FLG_MUTEX, SEM_MUTEX_ONE_LOCK);
	// mmudsc_t_init(&initp->msd_mmu);
	virmemadrs_t_init(&initp->msd_virmemadrs);
	initp->msd_stext = 0;
	initp->msd_etext = 0;
	initp->msd_sdata = 0;
	initp->msd_edata = 0;
	initp->msd_sbss = 0;
	initp->msd_ebss = 0;
	initp->msd_sbrk = 0;
	initp->msd_ebrk = 0;
	return; 
}

mmadrsdsc_t initmmadrsdsc;

void init_kvirmemadrs()
{
	mmadrsdsc_t_init(&initmmadrsdsc);
	// kvirmemadrs_t_init(&krlvirmemadrs);
	// kvma_seting_kvirmemadrs(&krlvirmemadrs);
	kvma_inituserspace_virmemadrs(&initmmadrsdsc.msd_virmemadrs);
	// hal_mmu_init(&initmmadrsdsc.msd_mmu);
	// hal_mmu_load(&initmmadrsdsc.msd_mmu);
	// test_vadr();
	return;
}

// D
kmvarsdsc_t *vma_find_kmvarsdsc_is_ok(virmemadrs_t *vmalocked, kmvarsdsc_t *curr, adr_t start, size_t vassize)
{
	kmvarsdsc_t *nextkmvd = NULL;
	adr_t newend = start + (adr_t)vassize;
	
	if (list_is_last(&curr->kva_list, &vmalocked->vs_list) == FALSE)
	{
		nextkmvd = list_next_entry(curr, kmvarsdsc_t, kva_list);
		if (NULL == start) { 
		// 1、如果没有指定起始地址，则判断当前kmvarsdsc_t与下一个kmvarsdsc_t之间，是否有未分配的虚拟地址，长度满足要求
			if ((curr->kva_end + (adr_t)vassize) <= nextkmvd->kva_start) {
				return curr;
			}
		} else {
		// 2、如果制定了起始地址，则判断当前kmvarsdsc_t与 下一个kmvarsdsc_t之间，是否有未分配的虚拟地址，起始地址和长度都满足要求
			if ((curr->kva_end <= start) && (newend <= nextkmvd->kva_start)) {
				return curr;
			}
		}
	}
	else
	{
		if (NULL == start) // 如果 start 是 NULL，则表示从当前区间的结尾开始分配
		{
			if ((curr->kva_end + (adr_t)vassize) < vmalocked->vs_isalcend) {
				return curr;
			}
		}
		else
		{	// 如果指定了 start，判断当前区间是否有足够空间
			if ((curr->kva_end <= start) && (newend < vmalocked->vs_isalcend))
			{
				return curr;
			}
		}
	}
	return NULL;
}

// C
kmvarsdsc_t *vma_find_kmvarsdsc(virmemadrs_t *vmalocked, adr_t start, size_t vassize)
{
	kmvarsdsc_t *kmvdcurrent = NULL, *curr = vmalocked->vs_currkmvdsc;
	adr_t newend = start + vassize;
	list_h_t *listpos = NULL;
	if (0x1000 > vassize) {	// 请求的尺寸小于4KB, 返回
		return NULL;
	}

	if (newend > vmalocked->vs_isalcend) { // 大于能分配的最大范围，返回
		return NULL;
	}

	if (NULL != curr)	//  如果当前区间存在，先检查该区间是否可以容纳请求的大小
	{
		kmvdcurrent = vma_find_kmvarsdsc_is_ok(vmalocked, curr, start, vassize);
		if (NULL != kmvdcurrent)
		{
			return kmvdcurrent;
		}
	}
	
	// 遍历virmemadrs_t中的所有的kmvarsdsc_t结构
	list_for_each(listpos, &vmalocked->vs_list)
	{
		curr = list_entry(listpos, kmvarsdsc_t, kva_list);
		// 检查每个kmvarsdsc_t结构
		kmvdcurrent = vma_find_kmvarsdsc_is_ok(vmalocked, curr, start, vassize);
		if (NULL != kmvdcurrent)
		{
			return kmvdcurrent;
		}
	}
	return NULL;
}

// B 分配虚拟地址空间核心函数
adr_t vma_new_vadrs_core(mmadrsdsc_t *mm, adr_t start, size_t vassize, u64_t vaslimits, u32_t vastype)
{
	adr_t retadrs = NULL;
	kmvarsdsc_t *newkmvd = NULL, *currkmvd = NULL;
	virmemadrs_t *vma = &mm->msd_virmemadrs;
	// knl_spinlock(&vma->vs_lock);

	// 查找虚拟地址区间
	currkmvd = vma_find_kmvarsdsc(vma, start, vassize);
	if (NULL == currkmvd) {
		retadrs = NULL;
		goto out;
	}
	// 进行虚拟地址区间进行检查看能否复用这个数据结构
	if (((NULL == start) || (start == currkmvd->kva_end)) && (vaslimits == currkmvd->kva_limits) && (vastype == currkmvd->kva_maptype)) {
		retadrs = currkmvd->kva_end;		// 能复用的话，当前虚拟地址区间的结束地址返回
		currkmvd->kva_end += vassize;		// 扩展当前虚拟地址区间的结束地址为分配虚拟地址区间的大小
		vma->vs_currkmvdsc = currkmvd;
		goto out;
	}

	newkmvd = new_kmvarsdsc(); // 建立一个新的kmvarsdsc_t虚拟地址区间结构
	if (NULL == newkmvd) {
		retadrs = NULL;
		goto out;
	}

	if (NULL == start) {
		newkmvd->kva_start = currkmvd->kva_end;
	} else {
		newkmvd->kva_start = start;
	}

	// 设置新的虚拟地址区间的结束地址
	newkmvd->kva_end = newkmvd->kva_start + vassize;
	newkmvd->kva_limits = vaslimits;
	newkmvd->kva_maptype = vastype;
	newkmvd->kva_mcstruct = vma;
	vma->vs_currkmvdsc = newkmvd;
	
	// 将新的虚拟地址区间加入到virmemadrs_t结构中
	list_add(&currkmvd->kva_list, &newkmvd->kva_list);
	// 看看新的虚拟地址区间是否是最后一个
	if (list_is_last(&newkmvd->kva_list, &vma->vs_list) == TRUE) {
		vma->vs_endkmvdsc = newkmvd;
	}
	
	// 返回新的虚拟地址区间的开始地址
	retadrs = newkmvd->kva_start;
out:
	// knl_spinunlock(&vma->vs_lock);
	return retadrs;
}

// A 分配虚拟地址空间的接口
adr_t vma_new_vadrs(mmadrsdsc_t *mm, adr_t start, size_t vassize, u64_t vaslimits, u32_t vastype)
{
	if (NULL == mm || 1 > vassize) {
		return NULL;
	}

	if (NULL != start) 
	{
		if (((start & 0xfff) != 0) || (0x1000 > start) // 开始地址是否4KB对齐？
		|| (USER_VIRTUAL_ADDRESS_END < (start + vassize))) 
		{ 	// 结束地址不能超过整个虚拟地址空间
			return NULL;
		}
	}

	return vma_new_vadrs_core(mm, start, VADSZ_ALIGN(vassize), vaslimits, vastype);
}

kmvarsdsc_t *vma_del_find_kmvarsdsc(virmemadrs_t *vmalocked, adr_t start, size_t vassize)
{
	kmvarsdsc_t *curr = vmalocked->vs_currkmvdsc;
	adr_t newend = start + (adr_t)vassize;
	list_n_t *listpos = NULL;
	if (0x1000 > vassize)
	{
		return NULL;
	}

	if (NULL != curr)
	{
		if ((curr->kva_start) <= start && (newend <= curr->kva_end))
		{
			return curr;
		}
	}
	list_for_each(listpos, &vmalocked->vs_list)
	{
		curr = list_entry(listpos, kmvarsdsc_t, kva_list);
		if ((start >= curr->kva_start) && (newend <= curr->kva_end))
		{
			return curr;
		}
	}
	return NULL;
}

void vma_del_set_endcurrkmvd(virmemadrs_t *vmalocked, kmvarsdsc_t *del)
{
	kmvarsdsc_t *prevkmvd = NULL, *nextkmvd = NULL;
	if (list_is_last(&del->kva_list, &vmalocked->vs_list) == TRUE)
	{
		if (list_is_first(&del->kva_list, &vmalocked->vs_list) == FALSE)
		{
			prevkmvd = list_prev_entry(del, kmvarsdsc_t, kva_list);
			vmalocked->vs_endkmvdsc = prevkmvd;
			vmalocked->vs_currkmvdsc = prevkmvd;
		}
		else
		{
			vmalocked->vs_endkmvdsc = NULL;
			vmalocked->vs_currkmvdsc = NULL;
		}
	}
	else
	{
		nextkmvd = list_next_entry(del, kmvarsdsc_t, kva_list);
		vmalocked->vs_currkmvdsc = nextkmvd;
	}
	return;
}

bool_t vma_del_unmapping_phyadrs(mmadrsdsc_t *mm, kmvarsdsc_t *kmvd, adr_t start, adr_t end)
{
	// adr_t phyadrs;
	bool_t rets = TRUE;
	// mmudsc_t *mmu = &mm->msd_mmu;
	// kvmemcbox_t *kmbox = kmvd->kva_kvmbox;

	// for (adr_t vadrs = start; vadrs < end; vadrs += VMAP_MIN_SIZE)
	// {
	// 	phyadrs = hal_mmu_untransform(mmu, vadrs);
	// 	if (NULL != phyadrs && NULL != kmbox)
	// 	{
	// 		if (vma_del_usermsa(mm, kmbox, NULL, phyadrs) == FALSE)
	// 		{
	// 			rets = FALSE;
	// 		}
	// 	}
	// }

	return rets;

}

bool_t vma_del_unmapping(mmadrsdsc_t *mm, kmvarsdsc_t *kmvd, adr_t start, size_t vassize)
{
	adr_t end;

	if (NULL == mm || NULL == kmvd)
	{
		return FALSE;
	}

	end = start + (adr_t)vassize;

	return vma_del_unmapping_phyadrs(mm, kmvd, start, end);
}

bool_t vma_del_vadrs_core(mmadrsdsc_t *mm, adr_t start, size_t vassize)
{
	bool_t rets = FALSE;
	kmvarsdsc_t *newkmvd = NULL, *delkmvd = NULL;
	virmemadrs_t *vma = &mm->msd_virmemadrs;
	// knl_spinlock(&vma->vs_lock);

	// 查找要释放虚拟地址空间的kmvarsdsc_t结构, 
	delkmvd = vma_del_find_kmvarsdsc(vma, start, vassize);
	if (NULL == delkmvd) {
		rets = FALSE;
		goto out;
	}
	
	// delkmvd记录删除的区间
	//第一种情况要释放的虚拟地址空间正好等于查找的kmvarsdsc_t结构
	if ((delkmvd->kva_start == start) && (delkmvd->kva_end == (start + (adr_t)vassize)))
	{
		vma_del_unmapping(mm, delkmvd, start, vassize);
		vma_del_set_endcurrkmvd(vma, delkmvd);
		// knl_put_kvmemcbox(delkmvd->kva_kvmbox);
		list_del(&delkmvd->kva_list); // 脱链
		del_kmvarsdsc(delkmvd);	// 删除kmvardsc_t结构
		vma->vs_kmvdscnr--;
		rets = TRUE;
		goto out;
	}
	// 第二种情况要释放的虚拟地址空间是在查找的kmvarsdsc_t结构的下半部分
	if ((delkmvd->kva_start == start) && (delkmvd->kva_end > (start + (adr_t)vassize)))
	{//所以直接把查找的kmvarsdsc_t结构的开始地址设置为释放虚拟地址空间的结束地址
		delkmvd->kva_start = start + (adr_t)vassize; // 砍掉下半
		vma_del_unmapping(mm, delkmvd, start, vassize);
		rets = TRUE;
		goto out;
	}
	// 第三种情况要释放的虚拟地址空间是在查找的kmvarsdsc_t结构的上半部分
	if ((delkmvd->kva_start < start) && (delkmvd->kva_end == (start + (adr_t)vassize)))
	{//所以直接把查找的kmvarsdsc_t结构的结束地址设置为释放虚拟地址空间的开始地址
		delkmvd->kva_end = start; // 砍掉上半
		vma_del_unmapping(mm, delkmvd, start, vassize);
		rets = TRUE;
		goto out;
	}

	//第四种情况要释放的虚拟地址空间是在查找的kmvarsdsc_t结构的中间
	if ((delkmvd->kva_start < start) && (delkmvd->kva_end > (start + (adr_t)vassize)))
	{//所以要再新建一个kmvarsdsc_t结构来处理释放虚拟地址空间之后的下半虚拟部分地址空间
	// 砍掉中间部分，两边拆分为两个kmvarsdsc_t
		newkmvd = new_kmvarsdsc();
		if (NULL == newkmvd)
		{
			rets = FALSE;
			goto out;
		}
		
		// 让新的kmvarsdsc_t结构指向查找的kmvarsdsc_t结构的后半部分虚拟地址空间
		// 和查找到的kmvarsdsc_t结构保持一致
		newkmvd->kva_end = delkmvd->kva_end;
		newkmvd->kva_start = start + (adr_t)vassize;
		newkmvd->kva_limits = delkmvd->kva_limits;
		newkmvd->kva_maptype = delkmvd->kva_maptype;
		newkmvd->kva_mcstruct = vma;
		
		delkmvd->kva_end = start;

		// knl_count_kvmemcbox(delkmvd->kva_kvmbox);
		newkmvd->kva_kvmbox = delkmvd->kva_kvmbox;

		vma_del_unmapping(mm, delkmvd, start, vassize);

		list_add(&delkmvd->kva_list, &newkmvd->kva_list);
		vma->vs_kmvdscnr++;
		if (list_is_last(&newkmvd->kva_list, &vma->vs_list) == TRUE)
		{
			vma->vs_endkmvdsc = newkmvd;
			vma->vs_currkmvdsc = newkmvd;
		}
		else
		{
			vma->vs_currkmvdsc = newkmvd;
		}
		rets = TRUE;
		goto out;
	}

	rets = FALSE;

out:
	// knl_spinunlock(&vma->vs_lock);
	return rets;
}


// 释放虚拟地址空间的接口
bool_t vma_del_vadrs(mmadrsdsc_t *mm, adr_t start, size_t vassize)
{	
	// 对参数进行检查
	if (NULL == mm || 1 > vassize || NULL == (void*)start)
	{
		return FALSE;
	}
	// 调用核心处理函数
	return vma_del_vadrs_core(mm, start, VADSZ_ALIGN(vassize));
}


void test_vadr()
{
	adr_t vadr = vma_new_vadrs(&initmmadrsdsc, NULL, 0x1000, 0, 0);
	if (NULL == (void*)vadr) {
		color_printk(RED, BLACK, "分配虚拟地址空间失败\n");
	}
	
	int* p = (int*)vadr;
	*p = 20;
	
	return;
}
#include "errno.h"
//缺页异常处理接口
sint_t vma_map_fairvadrs(mmadrsdsc_t *mm, adr_t vadrs)
{//对参数进行检查
    if ((0x1000 > vadrs) || (USER_VIRTUAL_ADDRESS_END < vadrs) || (NULL == mm))
    {
        return -EPARAM;
    }
    //进行缺页异常的核心处理
    return vma_map_fairvadrs_core(mm, vadrs);
}

//由异常分发器调用的接口
sint_t krluserspace_accessfailed(adr_t fairvadrs)
{//这里应该获取当前进程的mm，但是现在我们没有进程，才initmmadrsdsc代替
    mmadrsdsc_t* mm = &initmmadrsdsc;
    //应用程序的虚拟地址不可能大于USER_VIRTUAL_ADDRESS_END
    if(USER_VIRTUAL_ADDRESS_END < fairvadrs)
    {
        return -EACCES;
    }
    return vma_map_fairvadrs(mm, fairvadrs);
}