#include "mmkit.h"
#include "kernelkit.h"

KLINE sint_t retn_mscidx(uint_t pages)
{
	sint_t pbits = search_64rlbits((uint_t)pages) - 1;
	if (pages & (pages - 1)) // 检查pages是不是2的整数次幂
	{
		pbits++;
	}
	return pbits;
}

void msclst_t_init(msclst_t *initp, uint_t pnr)
{
	initp->ml_msanr = 0;
	initp->ml_ompnr = pnr;
	list_init(&initp->ml_list);
	return;
}

void msomdc_t_init(msomdc_t *initp)
{
	for (uint_t i = 0; i < MSCLST_MAX; i++)
	{
		msclst_t_init(&initp->mc_lst[i], 1UL << i);
	}
	initp->mc_msanr = 0;
	list_init(&initp->mc_list);
	list_init(&initp->mc_kmobinlst);
	initp->mc_kmobinpnr = 0;
	return;
}

void freobjh_t_init(freobjh_t *initp, uint_t stus, void *stat)
{
	list_init(&initp->oh_list);
	initp->oh_stus = stus;
	initp->oh_stat = stat;
	return;
}

void kmsob_t_init(kmsob_t *initp)
{
	list_init(&initp->so_list);
	spin_init(&initp->so_lock);
	initp->so_stus = 0;
	initp->so_flgs = 0;
	initp->so_vstat = NULL;
	initp->so_vend = NULL;
	initp->so_objsz = 0;
	initp->so_objrelsz = 0;
	initp->so_mobjnr = 0;
	initp->so_fobjnr = 0;
	list_init(&initp->so_frelst);
	list_init(&initp->so_alclst);
	list_init(&initp->so_mextlst);
	initp->so_mextnr = 0;
	msomdc_t_init(&initp->so_mc); // 初始化管理内存页面的结构体
	initp->so_privp = nullptr;
	initp->so_extdp = nullptr;
	return;
}

void kmbext_t_init(kmbext_t *initp, adr_t vstat, adr_t vend, kmsob_t *kmsp)
{
	list_init(&initp->mt_list);
	initp->mt_vstat = vstat;
	initp->mt_vend = vend;
	initp->mt_kmsb = kmsp;
	initp->mt_mobjnr = 0;
	return;
}

void koblst_t_init(koblst_t *initp, size_t koblsz)
{
	list_init(&initp->ol_emplst); // 挂载 kmsob_t结构的链表
	initp->ol_cahe = nullptr;
	initp->ol_emnr = 0;
	initp->ol_sz = koblsz;
	return;
}

void kmsobmgrhed_t_init(kmsobmgrhed_t *initp)
{
	size_t koblsz = 32;
	spin_init(&initp->ks_lock);
	list_init(&initp->ks_tclst);
	initp->ks_tcnr = 0;
	initp->ks_msobnr = 0; // kmsob_t结构的数量
	initp->ks_msobche = nullptr;  // 最近分配内存对象的 kmsob_t 结构
	
	// 这里并不是按照开始的图形分类的而是每次增加32字节，
	// 所以是32，64,96,128,160,192,224，256，...
	for (uint_t i = 0; i < KOBLST_MAX; i++) {

		koblst_t_init(&initp->ks_msoblst[i], koblsz);
		koblsz += 32;
	}
	return;
}

void init_kmsob()
{
	kmsobmgrhed_t_init(&glomm.mo_kmsobmgr);
	return;
}

void kmsob_updata_cache(kmsobmgrhed_t *kmobmgrp, koblst_t *koblp, kmsob_t *kmsp, uint_t flgs)
{
	if (KUC_NEWFLG == flgs)
	{
		kmobmgrp->ks_msobche = kmsp;
		koblp->ol_cahe = kmsp;
		return;
	}
	if (KUC_DELFLG == flgs)
	{
		kmobmgrp->ks_msobche = kmsp;
		koblp->ol_cahe = kmsp;
		return;
	}
	if (KUC_DSYFLG == flgs)
	{
		if (kmsp == kmobmgrp->ks_msobche)
		{
			kmobmgrp->ks_msobche = nullptr;
		}
		if (kmsp == koblp->ol_cahe)
		{
			koblp->ol_cahe = nullptr;
		}
		return;
	}
	return;
}

// C1
kmsob_t *scan_newkmsob_isok(kmsob_t *kmsp, size_t msz)
{
	if (nullptr == kmsp || 1 > msz)
	{
		return nullptr;
	}
	//if (msz == kmsp->so_objsz)
	if(msz <= kmsp->so_objsz)
	{
		return kmsp;
	}
	return nullptr;
}

kmsob_t *scan_delkmsob_isok(kmsob_t *kmsp, void *fadrs, size_t fsz)
{
	if (nullptr == kmsp || nullptr == fadrs || 1 > fsz) {
		return nullptr;
	}
	if ((adr_t)fadrs >= (kmsp->so_vstat + sizeof(kmsob_t)) && ((adr_t)fadrs + (adr_t)fsz) <= kmsp->so_vend)
	{
		if (fsz <= kmsp->so_objsz)
		{
			return kmsp;
		}
	}
	if (1 > kmsp->so_mextnr) {
		return nullptr;
	}

	kmbext_t *bexp = nullptr;
	list_h_t *tmplst = nullptr;
	list_for_each(tmplst, &kmsp->so_mextlst)
	{
		bexp = list_entry(tmplst, kmbext_t, mt_list);
		if (bexp->mt_kmsb != kmsp)
		{
			system_error("scan_delkmsob_isok err\n");
		}
		if ((adr_t)fadrs >= (bexp->mt_vstat + sizeof(kmbext_t)) && ((adr_t)fadrs + (adr_t)fsz) <= bexp->mt_vend) {
			if (fsz <= kmsp->so_objsz) {
				return kmsp;
			}
		}
	}

	return nullptr;
}

bool_t scan_nmszkmsob_isok(kmsob_t *kmsp, size_t msz)
{
	if (nullptr == kmsp || 1 > msz)
	{
		return FALSE;
	}
	if (1 > kmsp->so_fobjnr || 1 > kmsp->so_mobjnr)
	{
		return FALSE;
	}
	if (msz > kmsp->so_objsz)
	{
		return FALSE;
	}
	if ((kmsp->so_vend - kmsp->so_vstat + 1) < PAGESIZE ||
		(kmsp->so_vend - kmsp->so_vstat + 1) < (adr_t)(sizeof(kmsob_t) + sizeof(freobjh_t)))
	{
		return FALSE;
	}
	if (list_is_empty_careful(&kmsp->so_frelst) == TRUE)
	{
		return FALSE;
	}
	return TRUE;
}

bool_t scan_fadrskmsob_isok(adr_t fostat, adr_t vend, void *fadrs, size_t objsz)
{

	if ((adr_t)fadrs < fostat)
	{
		return FALSE;
	}
	if ((adr_t)fadrs >= fostat && ((adr_t)fadrs + (adr_t)objsz) <= vend)
	{
		if (0 == (((adr_t)fadrs - fostat) % objsz))
		{
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

bool_t scan_dfszkmsob_isok(kmsob_t *kmsp, void *fadrs, size_t fsz)
{
	list_h_t *tmplst = nullptr;
	kmbext_t *bextp = nullptr;
	if (nullptr == kmsp || nullptr == fadrs || 1 > fsz)
	{
		return FALSE;
	}

	if ((~0UL) <= kmsp->so_fobjnr)
	{
		return FALSE;
	}
	if ((adr_t)fadrs >= kmsp->so_vstat && ((adr_t)fadrs + (adr_t)fsz - 1) <= kmsp->so_vend)
	{
		if (FALSE == scan_fadrskmsob_isok((adr_t)(kmsp + 1), kmsp->so_vend, fadrs, kmsp->so_objsz))
		{
			return FALSE;
		}
		return TRUE;
	}
	if (kmsp->so_mextnr > 0)
	{
		list_for_each(tmplst, &kmsp->so_mextlst)
		{
			bextp = list_entry(tmplst, kmbext_t, mt_list);
			if ((adr_t)fadrs >= bextp->mt_vstat && ((adr_t)fadrs + (adr_t)fsz - 1) <= bextp->mt_vend)
			{
				if (FALSE == scan_fadrskmsob_isok((adr_t)(bextp + 1), bextp->mt_vend, fadrs, kmsp->so_objsz))
				{
					return FALSE;
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}

uint_t scan_kmsob_objnr(kmsob_t *kmsp)
{
	if (nullptr == kmsp)
	{
		system_error("scan_kmsob_objnr err1\n");
	}
	if (1 > kmsp->so_fobjnr && list_is_empty_careful(&kmsp->so_frelst) == FALSE)
	{
		system_error("scan_kmsob_objnr err2\n");
	}
	if (0 < kmsp->so_fobjnr)
	{
		return kmsp->so_fobjnr;
	}
	return 0;
}

// B2
kmsob_t *onkoblst_retn_newkmsob(koblst_t *koblp, size_t msz)
{
	kmsob_t *kmsp = nullptr, *tkmsp = nullptr;
	list_h_t *tmplst = nullptr;
	if (nullptr == koblp || 1 > msz) {
		return nullptr;
	}
	
	// 检查上一个刚刚分配了内存对象给其他模块内存池，能否满足msz尺寸内存对象的要求
	kmsp = scan_newkmsob_isok(koblp->ol_cahe, msz); 
	if (nullptr != kmsp) {
		return kmsp;
	}

    // 不满足则去访问其他内存池，看其能否满足要求
	if (0 < koblp->ol_emnr)
	{
		list_for_each(tmplst, &koblp->ol_emplst)
		{
			tkmsp = list_entry(tmplst, kmsob_t, so_list);
			kmsp = scan_newkmsob_isok(tkmsp, msz);
			if (nullptr != kmsp)
			{
				return kmsp;
			}
		}
	}
	return nullptr;
}

// 查找释放内存对象所属的kmso_t结构
kmsob_t *onkoblst_retn_delkmsob(koblst_t *koblp, void *fadrs, size_t fsz)
{
	kmsob_t *kmsp = nullptr, *tkmsp = nullptr;
	list_h_t *tmplst = nullptr;
	if (nullptr == koblp || nullptr == fadrs || 1 > fsz)
	{
		return nullptr;
	}

	// 看看上次刚刚操作的kmsob_t结构
	kmsp = scan_delkmsob_isok(koblp->ol_cahe, fadrs, fsz);
	if (nullptr != kmsp) {
		return kmsp;
	}

	if (0 < koblp->ol_emnr)
	{
		list_for_each(tmplst, &koblp->ol_emplst)
		{
			tkmsp = list_entry(tmplst, kmsob_t, so_list);
			kmsp = scan_delkmsob_isok(tkmsp, fadrs, fsz);
			if (nullptr != kmsp)
			{
				return kmsp;
			}
		}
	}
	return nullptr;
}

// B1

/**
 * @brief 根据内存对象大小查找并返回 koblst_t = 挂载 kmsob_t 的链表
 * (指定小内存为msz的内存池) 结构指针
 * 
 */
koblst_t *onmsz_retn_koblst(kmsobmgrhed_t *kmmgrhlokp, size_t msz)
{
	if (nullptr == kmmgrhlokp || 1 > msz)
	{
		return nullptr;
	}
	
	for (uint_t kli = 0; kli < KOBLST_MAX; kli++)
	{
		if (kmmgrhlokp->ks_msoblst[kli].ol_sz >= msz)
		{
			return &kmmgrhlokp->ks_msoblst[kli];
		}
	}

	return nullptr;
}

bool_t kmsob_add_koblst(koblst_t *koblp, kmsob_t *kmsp)
{
	if (nullptr == koblp || nullptr == kmsp) {
		return FALSE;
	}
	if (kmsp->so_objsz > koblp->ol_sz) {
		return FALSE;
	}
    
	list_add_to_before(&koblp->ol_emplst, &kmsp->so_list);
	koblp->ol_emnr++;
	return TRUE;
}

// 创建某个内存对象大小的最初内存池
kmsob_t *_create_init_kmsob(kmsob_t *kmsp, size_t objsz, adr_t cvadrs, adr_t cvadre, msadsc_t *msa, uint_t relpnr)
{
	if (nullptr == kmsp || 1 > objsz || NULL == cvadrs || NULL == cvadre || nullptr == msa || 1 > relpnr) {
		return nullptr;
	}
	if (objsz < sizeof(freobjh_t)) {
		return nullptr;
	}
	if ((cvadre - cvadrs + 1) < PAGESIZE) {
		return nullptr;
	}
	if ((cvadre - cvadrs + 1) <= (sizeof(kmsob_t) + sizeof(freobjh_t))) {
		return nullptr;
	}

	kmsob_t_init(kmsp);

	kmsp->so_vstat = cvadrs;
	kmsp->so_vend = cvadre;
	kmsp->so_objsz = objsz;
    
    // 在双向链表中，头节点的前面插入一个节点。相等于在链表尾添加一个节点
    list_add_to_before(&kmsp->so_mc.mc_kmobinlst, &msa->md_list);
	kmsp->so_mc.mc_kmobinpnr = (uint_t)relpnr;

	// ** 开始分割小内存，并且在其上建立管理对象，加入msob_t中 **
    // 标记内存对象的起始位置和结束位置
	freobjh_t *fohstat = (freobjh_t *)(kmsp + 1), *fohend = (freobjh_t *)cvadre;
	uint_t ap = (uint_t)((uint_t)fohstat);
	freobjh_t *tmpfoh = (freobjh_t *)((uint_t)ap);
	for (; tmpfoh < fohend;) // 建立内存对象
	{
		if ((ap + (uint_t)kmsp->so_objsz) <= (uint_t)cvadre)
		{
			freobjh_t_init(tmpfoh, 0, (void *)tmpfoh); // 初始化内存对象，内存对象就建立在要分配给其他进程的每个空闲内存上
			list_add_to_before(&kmsp->so_frelst, &tmpfoh->oh_list);
			kmsp->so_mobjnr++;
			kmsp->so_fobjnr++;
		}
		ap += (uint_t)kmsp->so_objsz;
		tmpfoh = (freobjh_t *)((uint_t)ap);
	}
	return kmsp;
}

// B3 创建某个内存对象大小的扩展内存池
kmsob_t *_create_kmsob(kmsobmgrhed_t *kmmgrlokp, koblst_t *koblp, size_t objsz)
{
	if (nullptr == kmmgrlokp || nullptr == koblp || 1 > objsz) {
		return nullptr;
	}

	kmsob_t *kmsp = nullptr;
	msadsc_t *msa = nullptr;
	uint_t relpnr = 0;
	uint_t pages = 1;
	if (128 < objsz) {
		pages = 2;
	}
	if (512 < objsz) {
		pages = 4;
	}
	
    // 申请物理页内存
    msa = mm_division_pages(&glomm, pages, &relpnr, MA_TYPE_KRNL, DMF_RELDIV);
	if (nullptr == msa) {
		return nullptr;
	}
	if (nullptr != msa && 0 == relpnr) {
		system_error("_create_kmsob mm_division_pages fail\n");
		return nullptr;
	}

	u64_t phyadr = msa->md_phyadrs.paf_padrs << PSHRSIZE; // physical address start 
	u64_t phyade = phyadr + (relpnr << PSHRSIZE) - 1;   // physical address end 
	adr_t vadrs = (adr_t)Phy_To_Virt((adr_t)phyadr);
	adr_t vadre = (adr_t)Phy_To_Virt((adr_t)phyade);
	
    kmsp = _create_init_kmsob((kmsob_t *)vadrs, koblp->ol_sz, vadrs, vadre, msa, relpnr);
	if (nullptr == kmsp)
	{
		if (mm_merge_pages(&glomm, msa, relpnr) == FALSE)
		{
			system_error("_create_kmsob mm_merge_pages fail\n");
		}
		return nullptr;
	}
	if (kmsob_add_koblst(koblp, kmsp) == FALSE)
	{
		system_error(" _create_kmsob kmsob_add_koblst FALSE\n");
	}
	kmmgrlokp->ks_msobnr++;
	return kmsp;
}

// C1  执行内存对象分配操作
void *kmsob_new_opkmsob(kmsob_t *kmsp, size_t msz)
{
	if (nullptr == kmsp || 1 > msz) {
		return nullptr;
	}
	if (scan_nmszkmsob_isok(kmsp, msz) == FALSE) {
		return nullptr;
	}
	
    freobjh_t *fobh = list_entry(kmsp->so_frelst.next, freobjh_t, oh_list);
	list_del(&fobh->oh_list);
	kmsp->so_fobjnr--;
	
    return (void *)(fobh);
}

// C2 扩展内存池
bool_t kmsob_extn_pages(kmsob_t *kmsp)
{
	if (nullptr == kmsp) {
		return FALSE;
	}
	if ((~0UL) <= kmsp->so_mobjnr || (~0UL) <= kmsp->so_mextnr || (~0UL) <= kmsp->so_fobjnr) {
		return FALSE;
	}

	msadsc_t *msa = nullptr;
	uint_t relpnr = 0;
	uint_t pages = 1;
	if (128 < kmsp->so_objsz) {
		pages = 2;
	}
	if (512 < kmsp->so_objsz) {
		pages = 4;
	}

	msa = mm_division_pages(&glomm, pages, &relpnr, MA_TYPE_KRNL, DMF_RELDIV);
	if (nullptr == msa) {
		return FALSE;
	}
	if (nullptr != msa && 0 == relpnr) {
		system_error("kmsob_extn_pages mm_division_pages fail\n");
		return FALSE;
	}

	u64_t phyadr = msa->md_phyadrs.paf_padrs << PSHRSIZE;
	u64_t phyade = phyadr + (relpnr << PSHRSIZE) - 1;
	adr_t vadrs = (adr_t)Phy_To_Virt((adr_t)phyadr);
	adr_t vadre = (adr_t)Phy_To_Virt((adr_t)phyade);
	sint_t mscidx = retn_mscidx(relpnr);
	if (MSCLST_MAX <= mscidx || 0 > mscidx) 
	{ 
		if (mm_merge_pages(&glomm, msa, relpnr) == FALSE)
		{
			system_error("kmsob_extn_pages mm_merge_pages fail\n");
		}
		return FALSE;
	}

    list_add_to_before(&kmsp->so_mc.mc_lst[mscidx].ml_list, &msa->md_list);
	kmsp->so_mc.mc_lst[mscidx].ml_msanr++;

	kmbext_t *bextp = (kmbext_t *)vadrs;
	kmbext_t_init(bextp, vadrs, vadre, kmsp);

	// 划分物理块
	freobjh_t *fohstat = (freobjh_t *)(bextp + 1), *fohend = (freobjh_t *)vadre;
	uint_t ap = (uint_t)((uint_t)fohstat);
	freobjh_t *tmpfoh = (freobjh_t *)((uint_t)ap);
	for (; tmpfoh < fohend;)
	{
		if ((ap + (uint_t)kmsp->so_objsz) <= (uint_t)vadre)
		{
			freobjh_t_init(tmpfoh, 0, (void *)tmpfoh);
			list_add_to_before(&kmsp->so_frelst, &tmpfoh->oh_list);
			kmsp->so_mobjnr++;
			kmsp->so_fobjnr++;
			bextp->mt_mobjnr++;
		}
		ap += (uint_t)kmsp->so_objsz;
		tmpfoh = (freobjh_t *)((uint_t)ap);
	}
	
    list_add_to_before(&kmsp->so_mextlst, &bextp->mt_list);
	kmsp->so_mextnr++;
	return TRUE;
}

// B4 
void *kmsob_new_onkmsob(kmsob_t *kmsp, size_t msz)
{
	if (nullptr == kmsp || 1 > msz) {
		return nullptr;
	}
	void *retptr = nullptr;
	cpuflg_t cpuflg;
	spinlock_storeflg_cli(&kmsp->so_lock, &cpuflg);
	if (scan_kmsob_objnr(kmsp) < 1)
	{ // 当前内存池内存对象不足，GOTO: 扩展内存池
		if (kmsob_extn_pages(kmsp) == FALSE)
		{
			retptr = nullptr;
			goto ret_step;
		}
	}
	retptr = kmsob_new_opkmsob(kmsp, msz);
ret_step:
	spinunlock_restoreflg(&kmsp->so_lock, &cpuflg);
	return retptr;
}

/**
 * @brief 小块内存申请函数核心
 * 
 * @param msz 
 * @return void* 
 */
void *kmsob_new_core(size_t msz)
{
	kmsobmgrhed_t *kmobmgrp = &glomm.mo_kmsobmgr;//获取kmsobmgrhed_t结构的地址
	void *retptr = nullptr;
	koblst_t *koblp = nullptr;
	kmsob_t *kmsp = nullptr;
	cpuflg_t cpuflg;
	spinlock_storeflg_cli(&kmobmgrp->ks_lock, &cpuflg);

	// 根据内存对象大小查找并返回 koblst_t(指定小内存为msz的内存池) 结构指针
	koblp = onmsz_retn_koblst(kmobmgrp, msz); 
	if (nullptr == koblp) {
		retptr = nullptr;
		goto ret_step;
	}
	
	// msz = ALIGN_UP8(msz);  // 对齐到8字节

    // 从 (内存池) 中获取 kmsob_t结构, kmsot_t是被建立在物理页上，其余空间用于划分成msz尺寸的小内存
	kmsp = onkoblst_retn_newkmsob(koblp, msz);
	if (nullptr == kmsp) {
        // 没有kmsob_t结构，则建立并挂载到koblst_t结构中
		kmsp = _create_kmsob(kmobmgrp, koblp, koblp->ol_sz);
		if (nullptr == kmsp) {
			retptr = nullptr;
			goto ret_step;
		}
	}

	retptr = kmsob_new_onkmsob(kmsp, msz);
	if (nullptr == retptr) {
		retptr = nullptr;
		goto ret_step;
	}

	//更新kmsobmgrhed_t结构的信息
	kmsob_updata_cache(kmobmgrp, koblp, kmsp, KUC_NEWFLG);
ret_step:
	spinunlock_restoreflg(&kmobmgrp->ks_lock, &cpuflg);
	return retptr;
}

// 内存对象分配接口
void *kmsob_new(size_t msz)
{
	if (1 > msz || 2048 < msz)	{
	//对于小于1 或者 大于2048字节的大小不支持 直接返回NULL表示失败
		return nullptr;
	}
	return kmsob_new_core(msz);
}

/**
 * @brief 检查kmsot_t是否所有内存块空闲
 * 
 * @param kmsp 
 * @return uint_t  0: 不可饶恕的错误 1：非全部空闲 2：全部空闲
 */
uint_t scan_freekmsob_isok(kmsob_t *kmsp)
{
	if (nullptr == kmsp)
	{
		return 0;
	}
	if (kmsp->so_mobjnr < kmsp->so_fobjnr)
	{
		return 0;
	}
	if (kmsp->so_mobjnr == kmsp->so_fobjnr)
	{
		return 2;
	}
	return 1;
}

// 执行回收kmsob_t的动作
bool_t _destroy_kmsob_core(kmsobmgrhed_t *kmobmgrp, koblst_t *koblp, kmsob_t *kmsp)
{
	if (nullptr == kmobmgrp || nullptr == koblp || nullptr == kmsp) {
		return FALSE;
	}
	if (1 > kmsp->so_mc.mc_kmobinpnr || list_is_empty_careful(&kmsp->so_mc.mc_kmobinlst) == TRUE) {
		return FALSE;
	}

	list_h_t *tmplst = nullptr;
	msadsc_t *msa = nullptr;
	msclst_t *mscp = kmsp->so_mc.mc_lst;
	list_del(&kmsp->so_list);
	koblp->ol_emnr--;
	kmobmgrp->ks_msobnr--;

	kmsob_updata_cache(kmobmgrp, koblp, kmsp, KUC_DSYFLG);

	for (uint_t j = 0; j < MSCLST_MAX; j++) // 释放扩展内存池占用的页
	{
		if (0 < mscp[j].ml_msanr)
		{
			list_for_each_head_dell(tmplst, &mscp[j].ml_list)
			{
				msa = list_entry(tmplst, msadsc_t, md_list);
				list_del(&msa->md_list);
				if (mm_merge_pages(&glomm, msa, (uint_t)mscp[j].ml_ompnr) == FALSE)
				{
					system_error("_destroy_kmsob_core mm_merge_pages FALSE2\n");
				}
			}
		}
	}

	list_for_each_head_dell(tmplst, &kmsp->so_mc.mc_kmobinlst) // 释放本内存池占用的页
	{
		msa = list_entry(tmplst, msadsc_t, md_list);
		list_del(&msa->md_list); // 断物理页链
		if (mm_merge_pages(&glomm, msa, (uint_t)kmsp->so_mc.mc_kmobinpnr) == FALSE) {
			system_error("_destroy_kmsob_core mm_merge_pages FALSE2\n");
		}
	}
	return TRUE;
}

bool_t _destroy_kmsob(kmsobmgrhed_t *kmobmgrp, koblst_t *koblp, kmsob_t *kmsp)
{
	if (nullptr == kmobmgrp || nullptr == koblp || nullptr == kmsp) {
		return FALSE;
	}
	if (1 > kmobmgrp->ks_msobnr || 1 > koblp->ol_emnr) {
		return FALSE;
	}

	uint_t screts = scan_freekmsob_isok(kmsp);
	
	switch(screts) {
		case 0: 
			system_error("_destroy_kmsob scan_freekmsob_isok rets 0\n");
			break;
		case 1:
			kmsob_updata_cache(kmobmgrp, koblp, kmsp, KUC_DELFLG); // 更新cache缓存
			return TRUE;
		case 2:
			return _destroy_kmsob_core(kmobmgrp, koblp, kmsp); // 回收 kmsob_t 占用的整个页面
	}

	return FALSE;
}

// 执行小内存的删除动作
bool_t kmsob_del_opkmsob(kmsob_t *kmsp, void *fadrs, size_t fsz)
{
	if (nullptr == kmsp || nullptr == fadrs || 1 > fsz) {
		return FALSE;
	}
	if ((kmsp->so_fobjnr + 1) > kmsp->so_mobjnr) {
		return FALSE;
	}
	if (scan_dfszkmsob_isok(kmsp, fadrs, fsz) == FALSE)
	{
		return FALSE;
	}

	freobjh_t *obhp = (freobjh_t *)fadrs;
	freobjh_t_init(obhp, 0, obhp);
	list_add_to_before(&kmsp->so_frelst, &obhp->oh_list);
	kmsp->so_fobjnr++;
	return TRUE;
}

bool_t kmsob_delete_onkmsob(kmsob_t *kmsp, void *fadrs, size_t fsz)
{
	if (nullptr == kmsp || nullptr == fadrs || 1 > fsz)
	{
		return FALSE;
	}
	bool_t rets = FALSE;
	cpuflg_t cpuflg;
	spinlock_storeflg_cli(&kmsp->so_lock, &cpuflg);
	if (kmsob_del_opkmsob(kmsp, fadrs, fsz) == FALSE) {
		rets = FALSE;
		goto ret_step;
	}
	rets = TRUE;
ret_step:
	spinunlock_restoreflg(&kmsp->so_lock, &cpuflg);
	return rets;
}

bool_t kmsob_delete_core(void *fadrs, size_t fsz)
{
	kmsobmgrhed_t *kmobmgrp = &glomm.mo_kmsobmgr;
	bool_t rets = FALSE;
	koblst_t *koblp = nullptr;
	kmsob_t *kmsp = nullptr;
	cpuflg_t cpuflg;
	spinlock_storeflg_cli(&kmobmgrp->ks_lock, &cpuflg);
	
	// 根据释放内存对象的大小在kmsobmgrhed_t中查找并返回koblst_t，
	koblp = onmsz_retn_koblst(kmobmgrp, fsz);
	if (nullptr == koblp) {
		rets = FALSE;
		goto ret_step;
	}

	// 根据释放内存对象的地址在koblst_t中查找并返回kmsob_t结构体，
	kmsp = onkoblst_retn_delkmsob(koblp, fadrs, fsz);
	if (nullptr == kmsp) {
		rets = FALSE;
		goto ret_step;
	}

	// TOGO: 执行删除动作
	rets = kmsob_delete_onkmsob(kmsp, fadrs, fsz);
	if (FALSE == rets) {
		rets = FALSE;
		goto ret_step;
	}

	if (_destroy_kmsob(kmobmgrp, koblp, kmsp) == FALSE)
	{
		rets = FALSE;
		goto ret_step;
	}
	rets = TRUE;
ret_step:
	spinunlock_restoreflg(&kmobmgrp->ks_lock, &cpuflg);
	return rets;
}

// 释放内存对象接口
bool_t kmsob_delete(void *fadrs, size_t fsz)
{
	if (nullptr == fadrs || 1 > fsz || 2048 < fsz)
	{
		return FALSE;
	}
	return kmsob_delete_core(fadrs, fsz);
}

