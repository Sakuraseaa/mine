#include "mmkit.h"
#include "kernelkit.h"
extern mmgro_t glomm;
void kvmemcboxmgr_t_init(kvmemcboxmgr_t* init);
void knl_count_kvmemcbox(kvmemcbox_t* kmbox);
bool_t knl_put_kvmemcbox(kvmemcbox_t* kmbox);
bool_t vma_del_usermsa(mmdsc_t *mm, kvmemcbox_t *kmbox, msadsc_t *msa, adr_t phyadr);
kvirmemadrs_t krlvirmemadrs;

void teststc_t_init(teststc_t *initp)
{
	list_init(&initp->tst_list);
	initp->tst_vadr = 0;
	initp->tst_vsiz = 0;
	initp->tst_type = 0;
	initp->tst_lime = 0;
	return;
}

teststc_t *new_teststc()
{
	teststc_t *t = (teststc_t *)kmsob_new(sizeof(teststc_t));
	if (nullptr == t)
	{
		return nullptr;
	}
	teststc_t_init(t);
	return t;
}

void del_teststc(teststc_t *delstc)
{

	if ((nullptr != delstc))
	{
		teststc_t_init(delstc);
		if (TRUE == kmsob_delete((void *)delstc, sizeof(teststc_t)))
		{
			return;
		}
	}
	system_error("del_teststc err\n");
	return;
}

void add_new_teststc(adr_t vadr, size_t vsiz)
{
	if (NULL == vadr || 1 > vsiz)
	{
		system_error("add_new_teststc parm err\n");
	}
	teststc_t *t = nullptr;
	t = new_teststc();
	if (nullptr == t)
	{
		system_error("add_new_teststc new_teststc nullptr\n");
	}
	t->tst_vadr = vadr;
	t->tst_vsiz = vsiz;
	list_add_to_behind(&krlvirmemadrs.kvs_testhead, &t->tst_list);
	krlvirmemadrs.kvs_tstcnr++;
	return;
}

void vaslknode_t_init(vaslknode_t *initp)
{
	if (nullptr == initp)
	{
		system_error("vaslknode_t_init pram err\n");
	}
	initp->vln_color = 0;
	initp->vln_flags = 0;
	initp->vln_left = nullptr;
	initp->vln_right = nullptr;
	initp->vln_prev = nullptr;
	initp->vln_next = nullptr;
	return;
}

void pgtabpage_t_init(pgtabpage_t *initp)
{
	spin_init(&initp->ptp_lock);
	list_init(&initp->ptp_msalist);
	initp->ptp_msanr = 0;
	return;
}

void virmemadrs_t_init(virmemadrs_t *initp)
{
	if (nullptr == initp) {
		return;
	}
	spin_init(&initp->vs_lock);
	initp->vs_resalin = 0;
	list_init(&initp->vs_list);
	initp->vs_flgs = 0;
	initp->vs_kmvdscnr = 0;
	initp->vs_mm = nullptr;
	initp->vs_startkmvdsc = 0;
	initp->vs_endkmvdsc = nullptr;
	initp->vs_currkmvdsc = nullptr;
	initp->vs_krlmapdsc = nullptr;
	initp->vs_krlhwmdsc = nullptr;
	initp->vs_krlolddsc = nullptr;
	initp->vs_isalcstart = 0;
	initp->vs_isalcend = 0;
	initp->vs_privte = 0;
	initp->vs_ext = 0;
	return;
}

void kmvarsdsc_t_init(kmvarsdsc_t *initp)
{
	if (nullptr == initp)
	{
		system_error("kmvarsdsc_t_init pram err\n");
	}
	spin_init(&initp->kva_lock);
	initp->kva_maptype = 0;
	list_init(&initp->kva_list);
	initp->kva_flgs = 0;
	initp->kva_limits = 0;
	vaslknode_t_init(&initp->kva_lknode);
	initp->kva_mcstruct = nullptr;
	initp->kva_start = 0;
	initp->kva_end = 0;
	initp->kva_kvmbox = nullptr;
	initp->kva_kvmcobj = nullptr;
	initp->kva_vir2file = nullptr;
	return;
}
void kvmcobjmgr_t_init(kvmcobjmgr_t* initp)
{
	if(nullptr==initp)
	{
		system_error("kvmcobjmgr_t_init parm nullptr\n");
	}
	// knl_spinlock_init(&initp->kom_lock);
	initp->kom_flgs=0;
	initp->kom_kvmcobjnr=0;
	list_init(&initp->kom_kvmcohead);
	initp->kom_kvmcocahenr=0;
	list_init(&initp->kom_kvmcocahe);
	initp->kom_kvmcodelnr=0;
	list_init(&initp->kom_kvmcodelhead);
	return;
}

// 初始化虚拟地址空间结构体
void kvirmemadrs_t_init(kvirmemadrs_t *initp)
{
	if (nullptr == initp) {
		system_error("kvirmemadrs_t_init pram err\n");
	}
	
	spin_init(&initp->kvs_lock);
	initp->kvs_flgs = 0;
	initp->kvs_kmvdscnr = 0;
	initp->kvs_startkmvdsc = nullptr;
	initp->kvs_endkmvdsc = nullptr;
	initp->kvs_krlmapdsc = nullptr;
	initp->kvs_krlhwmdsc = nullptr;
	initp->kvs_krlolddsc = nullptr;
	initp->kvs_isalcstart = 0;
	initp->kvs_isalcend = 0;
	initp->kvs_privte = nullptr;
	initp->kvs_ext = nullptr;
	list_init(&initp->kvs_testhead);
	initp->kvs_tstcnr = 0;
	initp->kvs_randnext = 1;
	pgtabpage_t_init(&initp->kvs_ptabpgcs);
	kvmcobjmgr_t_init(&initp->kvs_kvmcomgr);
	kvmemcboxmgr_t_init(&initp->kvs_kvmemcboxmgr);
	return;
}

/**
 * @brief 申请一个虚拟区间管理结构体，初始化并返回
 * 
 * @return kmvarsdsc_t* 
 */
kmvarsdsc_t *new_kmvarsdsc()
{
	kmvarsdsc_t *kmvdc = nullptr;
	kmvdc = (kmvarsdsc_t *)kmsob_new(sizeof(kmvarsdsc_t));
	if (nullptr == kmvdc) {
		return nullptr;
	}

	kmvarsdsc_t_init(kmvdc);
	return kmvdc;
}

bool_t del_kmvarsdsc(kmvarsdsc_t *delkmvd)
{
	if (nullptr == delkmvd) {
		return FALSE;
	}

	return kmsob_delete((void *)delkmvd, sizeof(kmvarsdsc_t));
}

virmemadrs_t *new_virmemadrs()
{
	virmemadrs_t *vmdsc = nullptr;
	vmdsc = (virmemadrs_t *)kmsob_new(sizeof(virmemadrs_t));
	if (nullptr == vmdsc)
	{
		return nullptr;
	}
	virmemadrs_t_init(vmdsc);
	return vmdsc;
}

bool_t del_virmemadrs(virmemadrs_t *vmdsc)
{
	if (nullptr == vmdsc)
	{
		return FALSE;
	}
	return kmsob_delete((void *)vmdsc, sizeof(virmemadrs_t));
}

void kvma_seting_kvirmemadrs(kvirmemadrs_t *kvma)
{
	kmvarsdsc_t *kmvdc = nullptr;
	if (nullptr == kvma) {
		system_error("kvma_seting_kvirmemadrs parm err\n");
	}
	kmvdc = new_kmvarsdsc();
	if (nullptr == kmvdc) {
		system_error("kvma_seting_kvirmemadrs nomem err\n");
	}
	kvma->kvs_isalcstart = KRNL_VIRTUAL_ADDRESS_START + KRNL_MAP_VIRTADDRESS_SIZE;
	kvma->kvs_isalcend = KRNL_VIRTUAL_ADDRESS_END;
	kmvdc->kva_start = KRNL_VIRTUAL_ADDRESS_START;
	kmvdc->kva_end = KRNL_VIRTUAL_ADDRESS_START + KRNL_MAP_VIRTADDRESS_SIZE;
	kmvdc->kva_mcstruct = kvma;
	kvma->kvs_startkmvdsc = kmvdc;
	kvma->kvs_endkmvdsc = kmvdc;
	kvma->kvs_krlmapdsc = kmvdc;
	kvma->kvs_kmvdscnr++;
	return;
}

bool_t kvma_inituserspace_virmemadrs(virmemadrs_t *vma)
{
	kmvarsdsc_t *kmvdc = nullptr, *stackkmvdc = nullptr;
	if (nullptr == vma) {
		return FALSE;
	}

	kmvdc = new_kmvarsdsc(); // 申请一个虚拟地址区间结构体
	if (nullptr == kmvdc) {
		return FALSE;
	}

	stackkmvdc = new_kmvarsdsc(); // 申请一个栈的虚拟地址区间结构体
	if (nullptr == stackkmvdc) {
		del_kmvarsdsc(kmvdc);
		return FALSE;
	}

	// 设置开始虚拟区间
	kmvdc->kva_start = USER_VIRTUAL_ADDRESS_START + 0x1000;
	kmvdc->kva_end = kmvdc->kva_start + 0x4000;
	kmvdc->kva_mcstruct = vma;

	// 设置栈虚拟区间
	stackkmvdc->kva_start = PAGE_4K_ALIGN(USER_VIRTUAL_ADDRESS_END - 0x8000);
	stackkmvdc->kva_end = USER_VIRTUAL_ADDRESS_END;
	stackkmvdc->kva_mcstruct = vma;
	stackkmvdc->kva_flgs = 1; // 标记栈内存

	// knl_spinlock(&vma->vs_lock);
	vma->vs_isalcstart = USER_VIRTUAL_ADDRESS_START;
	vma->vs_isalcend = USER_VIRTUAL_ADDRESS_END;
	vma->vs_startkmvdsc = kmvdc;		// 设置虚拟地址空间的开始区间为kmvdc
	vma->vs_endkmvdsc = stackkmvdc; // 设置虚拟空间结束地址为栈区
	list_add_to_behind(&vma->vs_list, &kmvdc->kva_list);
	list_add_to_behind(&kmvdc->kva_list, &stackkmvdc->kva_list);
	vma->vs_kmvdscnr += 2;
	// knl_spinunlock(&vma->vs_lock);
	return TRUE;
}

void mmadrsdsc_t_init(mmdsc_t* initp)
{
	if (nullptr == initp) {
		return;
	}
	spin_init(&initp->msd_lock);
	list_init(&initp->msd_list);
	initp->msd_flag = 0;
	initp->msd_stus = 0;
	initp->msd_scount = 0;
	// krlsem_t_init(&initp->msd_sem);
	// krlsem_set_sem(&initp->msd_sem, SEM_FLG_MUTEX, SEM_MUTEX_ONE_LOCK);
	mmudsc_t_init(&initp->msd_mmu); // 初始化相关页表
	virmemadrs_t_init(&initp->msd_virmemadrs); // 初始化虚拟地址空间结构体
	initp->start_code = initp->end_code = 0;
	initp->start_data = initp->end_data = 0;
	initp->start_rodata = initp->end_rodata = 0;
	initp->start_bss = initp->end_bss = 0;
	initp->start_brk = initp->end_brk = 0;
	initp->stack_length = initp->start_stack = 0;
	return; 
}

void init_kvirmemadrs()
{
	mmadrsdsc_t_init(&initmm);
	
	kvirmemadrs_t_init(&krlvirmemadrs);
	kvma_seting_kvirmemadrs(&krlvirmemadrs);
	
	// 初始化整虚拟地址空间管理结构体
	kvma_inituserspace_virmemadrs(&initmm.msd_virmemadrs);
	
	hal_mmu_init(&initmm.msd_mmu);
	hal_mmu_load(&initmm.msd_mmu);
	return;
}

/**
 * @brief 判断在区间curr后能否开拓一块新的虚拟内存分配给上层
 * 
 * @param vmalocked 整个虚拟空间
 * @param curr 	某个虚拟区间、某个虚拟区域
 * @param start 需要申请的虚拟内存地址
 * @param vassize 虚拟内存长度
 * @return kmvarsdsc_t* 成功返回 curr, 失败返回nullptr
 */
kmvarsdsc_t *vma_find_kmvarsdsc_is_ok(virmemadrs_t *vmalocked, kmvarsdsc_t *curr, adr_t start, size_t vassize)
{
	kmvarsdsc_t *nextkmvd = nullptr;
	adr_t newend = start + (adr_t)vassize;
	
	//如果curr不是最后一个先检查当前kmvarsdsc_t结构
	if (list_is_last(&curr->kva_list, &vmalocked->vs_list) == FALSE)
	{
		nextkmvd = list_next_entry(curr, kmvarsdsc_t, kva_list);
		if (NULL == start) { 
		// 1、如果没有指定起始地址，则判断当前虚拟区间与下一个虚拟区间之间，是否有未分配的虚拟地址，长度满足要求
			if ((curr->kva_end + (adr_t)vassize) <= nextkmvd->kva_start) {
				return curr;
			}
		} else {
		// 2、如果制定了起始地址，则判断当前虚拟区间与 下一个虚拟区间 之间，是否有未分配的虚拟地址，起始地址和长度都满足要求
			if ((curr->kva_end <= start) && (newend <= nextkmvd->kva_start)) {
				return curr;
			}
		}
	}
	else
	{
		if (NULL == start) // 如果 start 是 nullptr，则表示从整个虚拟空间的最后一个虚拟区间的结尾开始分配
		{
			if ((curr->kva_end + (adr_t)vassize) < vmalocked->vs_isalcend) {
				return curr;
			}
		}
		else
		{
			if ((curr->kva_end <= start) && (newend < vmalocked->vs_isalcend))
			{
				return curr;
			}
		}
	}
	return nullptr;
}

/**
 * @brief 在vmalocked所描述的整个虚拟空间，找一块虚拟内存
 * 
 * @param vmalocked 
 * @param start 虚拟内存的起始地址
 * @param vassize 虚拟内存的大小
 * @return kmvarsdsc_t* 成功找到返回虚拟内存起始地址之前最接近的 虚拟区间结构体，失败返回空
 */
kmvarsdsc_t *vma_find_kmvarsdsc(virmemadrs_t *vmalocked, adr_t start, size_t vassize)
{
	kmvarsdsc_t *kmvdcurrent = nullptr, *curr = vmalocked->vs_currkmvdsc;
	adr_t newend = start + vassize;
	list_h_t *listpos = nullptr;
	if (0x1000 > vassize) {	// 请求的尺寸小于4KB, 返回
		return nullptr;
	}

	if (newend > vmalocked->vs_isalcend) { // 大于能分配的最大范围，返回
		return nullptr;
	}

	if (nullptr != curr)	//  如果当前区间存在，先检查该区间是否可以容纳请求的大小
	{
		kmvdcurrent = vma_find_kmvarsdsc_is_ok(vmalocked, curr, start, vassize);
		if (nullptr != kmvdcurrent)
		{
			return kmvdcurrent;
		}
	}
	
	// 遍历虚拟空间中的所有的虚拟区间
	list_for_each(listpos, &vmalocked->vs_list)
	{
		curr = list_entry(listpos, kmvarsdsc_t, kva_list);
		// 检查每个kmvarsdsc_t结构
		kmvdcurrent = vma_find_kmvarsdsc_is_ok(vmalocked, curr, start, vassize);
		if (nullptr != kmvdcurrent)
		{
			return kmvdcurrent;
		}
	}
	
	return nullptr;
}


// B 分配虚拟地址区间核心逻辑
static adr_t vma_new_vadrs_core(mmdsc_t *mm, adr_t start, size_t vassize, vma_to_file_t* vtft, u64_t vaslimits, u32_t vastype, uint_t flags)
{
	adr_t retadrs = NULL;
	kmvarsdsc_t *newkmvd = nullptr, *currkmvd = nullptr;
	virmemadrs_t *vma = &mm->msd_virmemadrs;
	// knl_spinlock(&vma->vs_lock);

	/* 返回 虚拟内存start起始地址之前 最接近start 的 虚拟区间结构体. */
	currkmvd = vma_find_kmvarsdsc(vma, start, vassize);
	if (nullptr == currkmvd) {
		retadrs = NULL;
		goto out;
	}

	/* 进行虚拟地址区间进行检查看能否复用在这个结构之后直接划分虚拟内存，而不用申请新的虚拟区间结构体 */
	if (	((NULL == start) || (start == currkmvd->kva_end)) 
		&&  (vaslimits == currkmvd->kva_limits) 
		&&  (vastype == currkmvd->kva_maptype)
		&&  (vtft == nullptr))
	{
		retadrs = currkmvd->kva_end;		// 能复用的话，当前虚拟地址区间的结束地址返回
		currkmvd->kva_end += vassize;		// 扩展当前虚拟地址区间的结束地址为分配虚拟地址区间的大小
		vma->vs_currkmvdsc = currkmvd;
		goto out;
	}

	/* 不能复用 建立一个新的kmvarsdsc_t虚拟地址区间结构 */
	newkmvd = new_kmvarsdsc();
	if (nullptr == newkmvd) {
		retadrs = NULL;
		goto out;
	}

	if (NULL == start) {
		newkmvd->kva_start = currkmvd->kva_end;
	} else {
		newkmvd->kva_start = start;
	}

	/* 设置新的虚拟地址区间的结束地址 */
	newkmvd->kva_end = newkmvd->kva_start + vassize;
	newkmvd->kva_limits = vaslimits;
	newkmvd->kva_maptype = vastype;
	newkmvd->kva_mcstruct = vma;
	newkmvd->kva_vir2file = vtft;
	newkmvd->kva_flgs = flags;

	vma->vs_currkmvdsc = newkmvd;
	
	/* 将新的虚拟地址区间加入到virmemadrs_t结构中 */
	list_add_to_behind(&currkmvd->kva_list, &newkmvd->kva_list);

	/* 看看新的虚拟地址区间是否是最后一个 */
	if (list_is_last(&newkmvd->kva_list, &vma->vs_list) == TRUE) {
		vma->vs_endkmvdsc = newkmvd;
	}
	
	/* 返回新的虚拟地址区间的开始地址 */
	retadrs = newkmvd->kva_start;
out:
	// knl_spinunlock(&vma->vs_lock);
	return retadrs;
}

// A 分配虚拟地址区间的接口
/**
 * @brief 申请一段虚拟区间
 * 
 * @param mm 进程的空间管理结构体
 * @param start 虚拟区间起始地址
 * @param vassize 虚拟区间长度
 * @param file_position 虚拟区间对应的在文件中的位置
 * @param file_size 虚拟区间对应在文件的长度
 * @param vaslimits 
 * @param vastype 
 * @return adr_t 
 */
adr_t vma_new_vadrs(mmdsc_t *mm, adr_t start, size_t vassize, vma_to_file_t* vtft, u64_t vaslimits, u32_t vastype, uint_t flags)
{
	if (nullptr == mm || 1 > vassize) {
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

	return vma_new_vadrs_core(mm, start, VADSZ_ALIGN(vassize), vtft, vaslimits, vastype, flags);
}
// 复制一个虚拟内存空间
adr_t copy_one_vma(mmdsc_t* mm, const kmvarsdsc_t* nvma)
{
	if(nvma == nullptr)
	{
		return NULL;
	}
	adr_t ret = NULL;
	virmemadrs_t *vma_family = &mm->msd_virmemadrs;
	vma_to_file_t* vtft = nvma->kva_vir2file;

	if (nvma->kva_vir2file != nullptr)
	{
		vtft = (vma_to_file_t*)knew(sizeof(vma_to_file_t), 0);
		
		vtft->vtf_file = nvma->kva_vir2file->vtf_file;
		vtft->vtf_flag = nvma->kva_vir2file->vtf_flag;
		vtft->vtf_position = nvma->kva_vir2file->vtf_position;
		vtft->vtf_size = nvma->kva_vir2file->vtf_size;
		vtft->vtf_alread_load_size = 0;
	}

	ret = vma_new_vadrs_core(mm, nvma->kva_start, (nvma->kva_end - nvma->kva_start), vtft, nvma->kva_limits, nvma->kva_maptype, nvma->kva_flgs);
	
	kmvarsdsc_t* ovma = vma_family->vs_currkmvdsc;
	
	// 共享页面盒子结构在两个进程之间共享，何时才能取消共享？
	// 自己独立创建内存，并且当所有管理的物理页面没有被共享的时候。可以取消共享，但好像没有这种必要?
	if (nvma->kva_kvmbox != nullptr && ovma->kva_kvmbox == nullptr) // 共享页面盒子
	{
		// 增加共享计数
		// knl_count_kvmemcbox(nvma->kva_kvmbox);
		// ovma->kva_kvmbox = nvma->kva_kvmbox;
	}
	return ret;
}
kmvarsdsc_t *vma_del_find_kmvarsdsc(virmemadrs_t *vmalocked, adr_t start, size_t vassize)
{
	kmvarsdsc_t *curr = vmalocked->vs_currkmvdsc;
	adr_t newend = start + (adr_t)vassize;
	list_n_t *listpos = nullptr;
	if (0x1000 > vassize)
	{
		return nullptr;
	}

	if (nullptr != curr)
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
	return nullptr;
}

void vma_del_set_endcurrkmvd(virmemadrs_t *vmalocked, kmvarsdsc_t *del)
{
	kmvarsdsc_t *prevkmvd = nullptr, *nextkmvd = nullptr;
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
			vmalocked->vs_endkmvdsc = nullptr;
			vmalocked->vs_currkmvdsc = nullptr;
		}
	}
	else
	{
		nextkmvd = list_next_entry(del, kmvarsdsc_t, kva_list);
		vmalocked->vs_currkmvdsc = nextkmvd;
	}
	return;
}

bool_t vma_del_unmapping_phyadrs(mmdsc_t *mm, kmvarsdsc_t *kmvd, adr_t start, adr_t end)
{
	adr_t phyadrs;
	bool_t rets = TRUE;
	mmudsc_t *mmu = &mm->msd_mmu;
	kvmemcbox_t *kmbox = kmvd->kva_kvmbox;

	for (adr_t vadrs = start; vadrs < end; vadrs += VMAP_MIN_SIZE)
	{
		phyadrs = hal_mmu_untransform(mmu, vadrs);
		if (NULL != phyadrs && nullptr != kmbox)
		{
			if (vma_del_usermsa(mm, kmbox, nullptr, phyadrs) == FALSE)
			{
				rets = FALSE;
			}
		}
	}

	return rets;
}

bool_t vma_del_unmapping(mmdsc_t *mm, kmvarsdsc_t *kmvd, adr_t start, size_t vassize)
{
	adr_t end;

	if (nullptr == mm || nullptr == kmvd)
	{
		return FALSE;
	}

	end = start + (adr_t)vassize;

	return vma_del_unmapping_phyadrs(mm, kmvd, start, end);
}

bool_t vma_del_vadrs_core(mmdsc_t *mm, adr_t start, size_t vassize)
{
	bool_t rets = FALSE;
	kmvarsdsc_t *newkmvd = nullptr, *delkmvd = nullptr;
	virmemadrs_t *vma = &mm->msd_virmemadrs;
	// knl_spinlock(&vma->vs_lock);

	// 查找要释放虚拟地址空间的 虚拟地址区间 结构, 
	delkmvd = vma_del_find_kmvarsdsc(vma, start, vassize);
	if (nullptr == delkmvd) {
		rets = FALSE;
		goto out;
	}
	
	// delkmvd记录删除的区间
	//第一种情况要释放的虚拟地址空间正好等于查找的kmvarsdsc_t结构
	if ((delkmvd->kva_start == start) && (delkmvd->kva_end == (start + (adr_t)vassize)))
	{
		vma_del_unmapping(mm, delkmvd, start, vassize); // 取消虚拟地址的映射
		vma_del_set_endcurrkmvd(vma, delkmvd);
		knl_put_kvmemcbox(delkmvd->kva_kvmbox);
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
		if (nullptr == newkmvd)
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

		knl_count_kvmemcbox(delkmvd->kva_kvmbox);
		newkmvd->kva_kvmbox = delkmvd->kva_kvmbox;

		vma_del_unmapping(mm, delkmvd, start, vassize);

		list_add_to_behind(&delkmvd->kva_list, &newkmvd->kva_list);
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
bool_t vma_del_vadrs(mmdsc_t *mm, adr_t start, size_t vassize)
{	
	// 对参数进行检查
	if (nullptr == mm || 1 > vassize || nullptr == (void*)start)
	{
		return FALSE;
	}
	// 调用核心处理函数
	return vma_del_vadrs_core(mm, start, VADSZ_ALIGN(vassize));
}

void kvmemcbox_t_init(kvmemcbox_t* init)
{
	if (nullptr == init) {
		return;
	}
	list_init(&init->kmb_list);
	spin_init(&init->kmb_lock);
	atomic_set(&init->kmb_cont, 0);
	init->kmb_flgs = 0;
	init->kmb_stus = 0;
	init->kmb_type = 0;
	init->kmb_msanr = 0;
	list_init(&init->kmb_msalist);
	init->kmb_mgr = nullptr;
	init->kmb_filenode = nullptr;
	init->kmb_pager = nullptr;
	init->kmb_ext = nullptr;
	return;
}

void kvmemcboxmgr_t_init(kvmemcboxmgr_t* init)
{
	if (nullptr == init) {
		return;
	}

	list_init(&init->kbm_list);
	spin_init(&init->kbm_lock);
	init->kbm_flgs = 0;
	init->kbm_stus = 0;
	init->kbm_kmbnr = 0;
	list_init(&init->kbm_kmbhead);
	init->kbm_cachenr = 0;
	init->kbm_cachemax = KMBOX_CACHE_MAX;
	init->kbm_cachemin = KMBOX_CACHE_MIN;
	list_init(&init->kbm_cachehead);
	init->kbm_ext = nullptr;
	return;
}

kvmemcbox_t* new_kvmemcbox()
{
	kvmemcbox_t* kmbox = nullptr;
	
	kmbox = (kvmemcbox_t*)kmsob_new(sizeof(kvmemcbox_t));
	if(nullptr == kmbox) {
		return nullptr;
	}

	kvmemcbox_t_init(kmbox);
	return kmbox;
}

bool_t del_kvmemcbox(kvmemcbox_t* del)
{
	if (nullptr == del)
	{
		return FALSE;
	}
	
	if (del->kmb_cont.value > 1) {
		knl_decount_kvmemcbox(del);
		return TRUE;
	}

	return kmsob_delete((void*)del, sizeof(kvmemcbox_t));
}

void knl_count_kvmemcbox(kvmemcbox_t* kmbox)
{
	if(nullptr == kmbox)
	{
		return;
	}
	atomic_inc(&kmbox->kmb_cont);
	return;
}

void knl_decount_kvmemcbox(kvmemcbox_t* kmbox)
{
	if(nullptr == kmbox)
	{
		return;
	}
	atomic_dec(&kmbox->kmb_cont);
	return;
}

kvmemcbox_t* knl_get_kvmemcbox()
{
	kvmemcbox_t* kmb = nullptr;
	kvmemcboxmgr_t* kmbmgr = &krlvirmemadrs.kvs_kvmemcboxmgr; // 页面盒子头
	// knl_spinlock(&kmbmgr->kbm_lock);
	if(0 < kmbmgr->kbm_cachenr)
	{
		if(list_is_empty_careful(&kmbmgr->kbm_cachehead) == FALSE)
		{
			kmb = list_first_oneobj(&kmbmgr->kbm_cachehead, kvmemcbox_t, kmb_list);
			list_del(&kmb->kmb_list);
			kmbmgr->kbm_cachenr--;
			
			kvmemcbox_t_init(kmb);
			list_add_to_behind(&kmbmgr->kbm_kmbhead, &kmb->kmb_list);
			kmbmgr->kbm_kmbnr++;
			atomic_inc(&kmb->kmb_cont);
			kmb->kmb_mgr = kmbmgr;
			kmb = kmb;
			goto out; 
		}
	}

	kmb = new_kvmemcbox();
	if (nullptr == kmb) {
		goto out;
	}

	list_add_to_behind(&kmbmgr->kbm_kmbhead, &kmb->kmb_list);
	kmbmgr->kbm_kmbnr++;
	atomic_inc(&kmb->kmb_cont);
	kmb->kmb_mgr = kmbmgr;

out:
	// knl_spinunlock(&kmbmgr->kbm_lock);	
	return kmb;
}

bool_t knl_put_kvmemcbox(kvmemcbox_t* kmbox)
{
	kvmemcboxmgr_t* kmbmgr = &krlvirmemadrs.kvs_kvmemcboxmgr;
	bool_t rets = FALSE;
	if (nullptr == kmbox) {
		return FALSE;
	}

	// spinlock(&kmbmgr->kbm_lock);
	
	atomic_dec(&kmbox->kmb_cont);
	if (atomic_read(&kmbox->kmb_cont) >= 1) {
		rets = TRUE;
		goto out;
	}
	
	if (kmbmgr->kbm_cachenr >= kmbmgr->kbm_cachemax)
	{
		list_del(&kmbox->kmb_list);
		if(del_kvmemcbox(kmbox) == FALSE)
		{
			rets = FALSE;
			goto out;
		}
		else
		{
			kmbmgr->kbm_kmbnr--;
			rets = TRUE;
			goto out;
		}
	}

	list_move(&kmbox->kmb_list, &kmbmgr->kbm_cachehead);
	kmbmgr->kbm_cachenr++;
	kmbmgr->kbm_kmbnr--;
	
	rets = TRUE;
out:
	// knl_spinunlock(&kmbmgr->kbm_lock);
	return rets;
} 

msadsc_t* find_msa_from_pagebox(kvmemcbox_t* kbox, adr_t phyadr)
{
	msadsc_t *tmpmsa = nullptr;
	list_n_t *pos = nullptr;

	if (nullptr == kbox || NULL == phyadr) {
		return nullptr;
	}

	// knl_spinlock(&kmbox->kmb_lock);

	list_for_each(pos, &kbox->kmb_msalist)
	{
		tmpmsa = list_entry(pos, msadsc_t, md_list);
		if (msadsc_ret_addr(tmpmsa) == phyadr)
		{
			return tmpmsa;
		}
	}
	
	return nullptr;
}

bool_t vma_del_usermsa(mmdsc_t *mm, kvmemcbox_t *kmbox, msadsc_t *msa, adr_t phyadr)
{
	bool_t rets = FALSE;
	msadsc_t *tmpmsa = nullptr, *delmsa = nullptr;
	list_h_t *pos;

	if (nullptr == mm || nullptr == kmbox || NULL == phyadr) {
		return FALSE;
	}

	// knl_spinlock(&kmbox->kmb_lock);

	if (nullptr != msa)
	{
		if (msadsc_ret_addr(msa) == phyadr)
		{
			delmsa = msa;
			list_del(&msa->md_list);
			kmbox->kmb_msanr--;
			rets = TRUE;
			goto out;
		}
	}

	list_for_each(pos, &kmbox->kmb_msalist)
	{
		tmpmsa = list_entry(pos, msadsc_t, md_list);
		if (msadsc_ret_addr(tmpmsa) == phyadr)
		{
			delmsa = tmpmsa;
			list_del(&tmpmsa->md_list);
			kmbox->kmb_msanr--;
			rets = TRUE;
			goto out;
		}
	}

	delmsa = nullptr;
	rets = FALSE;

out:
	// knl_spinunlock(&kmbox->kmb_lock);

	if (nullptr != delmsa)
	{
		if (mm_merge_pages(&glomm, delmsa, onfrmsa_retn_fpagenr(delmsa)) == FALSE)
		{
			system_error("vma_del_usermsa err\n");
			return FALSE;
		}
	}

	return rets;
}

msadsc_t *vma_new_usermsa(mmdsc_t *mm, kvmemcbox_t *kmbox)
{
	u64_t pages = 1, retpnr = 0;
	msadsc_t *msa = nullptr;

	if (nullptr == mm || nullptr == kmbox) {
		return nullptr;
	}

	msa = mm_divpages_procmarea(&glomm, pages, &retpnr);
	if (nullptr == msa) {
		return nullptr;
	}

	// knl_spinlock(&kmbox->kmb_lock);

	list_add_to_behind(&kmbox->kmb_msalist, &msa->md_list);
	kmbox->kmb_msanr++;

	// knl_spinunlock(&kmbox->kmb_lock);
	return msa;
}

adr_t vma_map_msa_fault(mmdsc_t *mm, kvmemcbox_t *kmbox, adr_t vadrs, u64_t flags)
{
    msadsc_t *usermsa;
    adr_t phyadrs = NULL;

   //分配一个物理内存页面，挂载到kvmemcbox_t中，并返回对应的msadsc_t结构
    usermsa = vma_new_usermsa(mm, kmbox);
    if (nullptr == usermsa) { //没有物理内存页面返回NULL表示失败
        return NULL;
    }
    
	//获取msadsc_t对应的内存页面的物理地址
    phyadrs = msadsc_ret_addr(usermsa);
    
	// 建立MMU页表完成虚拟地址到物理地址的映射
    if (hal_mmu_transform(&mm->msd_mmu, vadrs, phyadrs, flags) == TRUE)
    {//映射成功则返回物理地址
        return phyadrs;
    }

    //映射失败就要先释放分配的物理内存页面
    vma_del_usermsa(mm, kmbox, usermsa, phyadrs);
    return NULL;
}

//接口函数
adr_t vma_map_phyadrs(mmdsc_t *mm, kmvarsdsc_t *kmvd, adr_t vadrs, u64_t flags)
{
    kvmemcbox_t *kmbox = kmvd->kva_kvmbox;
    if (nullptr == kmbox) {
        return NULL;
    }
    //调用核心函数，flags表示页表条目中的相关权限、存在、类型等位段
    return vma_map_msa_fault(mm, kmbox, vadrs, flags);
}

kmvarsdsc_t *vma_map_find_kmvarsdsc(virmemadrs_t *vmalocked, adr_t vadrs)
{
    list_h_t *pos = nullptr;
    kmvarsdsc_t *curr = vmalocked->vs_currkmvdsc;
    
	// 看看上一次刚刚被操作的kmvarsdsc_t结构
    if (nullptr != curr)
    {//虚拟地址是否落在kmvarsdsc_t结构表示的虚拟地址区间
        if ((vadrs >= curr->kva_start) && (vadrs < curr->kva_end))
        {
            return curr;
        }
    }
    
	//遍历每个kmvarsdsc_t结构
    list_for_each(pos, &vmalocked->vs_list)
    {
        curr = list_entry(pos, kmvarsdsc_t, kva_list);
        //虚拟地址是否落在kmvarsdsc_t结构表示的虚拟地址区间
        if ((vadrs >= curr->kva_start) && (vadrs < curr->kva_end)) {
            return curr;
        }
    }
    return nullptr;
}

kvmemcbox_t *vma_map_retn_kvmemcbox(kmvarsdsc_t *kmvd)
{
    kvmemcbox_t *kmbox = nullptr;
    //如果kmvarsdsc_t结构中已经存在了kvmemcbox_t结构，则直接返回
    if (nullptr != kmvd->kva_kvmbox) {
        return kmvd->kva_kvmbox;
    }
    //新建一个kvmemcbox_t结构
    kmbox = knl_get_kvmemcbox();
    if (nullptr == kmbox)
    {
        return nullptr;
    }
    //指向这个新建的kvmemcbox_t结构
    kmvd->kva_kvmbox = kmbox;
    return kmvd->kva_kvmbox;
}

void kvmemcobj_t_init(kvmemcobj_t* initp)
{
	if(NULL==initp)
	{
		system_error("kvmemcobj_t_init parm NULL\n");
	}
	list_init(&initp->kco_list);
	// knl_spinlock_init(&initp->kco_lock);
	initp->kco_cont=0;
	initp->kco_flgs=0;
	initp->kco_type=0;
	initp->kco_msadnr=0;
	list_init(&initp->kco_msadlst);
	initp->kco_filenode=NULL;
	initp->kco_pager=NULL;
	initp->kco_extp=NULL;
	return;
}