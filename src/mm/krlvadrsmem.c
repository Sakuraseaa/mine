#include "krlvadrsmem_t.h"
#include "halplatform_t.h"
#include "printk.h"
#include "memory.h"
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
	list_add_tail(&kmvdc->kva_list, &vma->vs_list);
	list_add_tail(&stackkmvdc->kva_list, &vma->vs_list);
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

// void test_vadr()
// {
// 	adr_t vadr = vma_new_vadrs(&initmmadrsdsc, NULL, 0x1000, 0, 0);
// 	if(NULL == vadr)
// 	{
// 		kprint("分配虚拟地址空间失败\n");
// 	}
// 	kprint("分配虚拟地址空间地址:%x\n", vadr);
// 	kprint("开始写入分配虚拟地址空间\n");
// 	hal_memset((void*)vadr, 0, 0x1000);
// 	kprint("结束写入分配虚拟地址空间\n");
// 	return;
// }

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