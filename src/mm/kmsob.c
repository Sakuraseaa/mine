#include "basetype.h"
#include "memmgrob.h"
#include "marea_t.h"
#include "msadsc_t.h"
#include "pages64_t.h"
#include "memory.h"
#include "memdivmer_t.h"
#include "kmsob_t.h"
#include "printk.h"
#include "lib.h"


KLINE sint_t retn_mscidx(uint_t pages)
{
	sint_t pbits = search_64rlbits((uint_t)pages) - 1;
	if (pages & (pages - 1))
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
	msomdc_t_init(&initp->so_mc);
	initp->so_privp = NULL;
	initp->so_extdp = NULL;
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
	list_init(&initp->ol_emplst); // ���� kmsob_t�ṹ������
	initp->ol_cahe = NULL;
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
	initp->ks_msobnr = 0; // kmsob_t�ṹ������
	initp->ks_msobche = NULL;  // ��������ڴ����� kmsob_t �ṹ
	
	// ���ﲢ���ǰ��տ�ʼ��ͼ�η���Ķ���ÿ������32�ֽڣ�
	// ������32��64,96,128,160,192,224��256��...
	for (uint_t i = 0; i < KOBLST_MAX; i++)
	{
		koblst_t_init(&initp->ks_msoblst[i], koblsz);
		koblsz += 32;
	}
	return;
}

extern memmgrob_t glomm;
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
			kmobmgrp->ks_msobche = NULL;
		}
		if (kmsp == koblp->ol_cahe)
		{
			koblp->ol_cahe = NULL;
		}
		return;
	}
	return;
}

// C1
kmsob_t *scan_newkmsob_isok(kmsob_t *kmsp, size_t msz)
{
	if (NULL == kmsp || 1 > msz)
	{
		return NULL;
	}
	if (msz <= kmsp->so_objsz)
	{
		return kmsp;
	}
	return NULL;
}

kmsob_t *scan_delkmsob_isok(kmsob_t *kmsp, void *fadrs, size_t fsz)
{
	if (NULL == kmsp || NULL == fadrs || 1 > fsz)
	{
		return NULL;
	}
	if ((adr_t)fadrs >= (kmsp->so_vstat + sizeof(kmsob_t)) && ((adr_t)fadrs + (adr_t)fsz) <= kmsp->so_vend)
	{
		if (fsz <= kmsp->so_objsz)
		{
			return kmsp;
		}
	}
	if (1 > kmsp->so_mextnr)
	{
		return NULL;
	}
	kmbext_t *bexp = NULL;
	list_h_t *tmplst = NULL;
	list_for_each(tmplst, &kmsp->so_mextlst)
	{
		bexp = list_entry(tmplst, kmbext_t, mt_list);
		if (bexp->mt_kmsb != kmsp)
		{
			system_error("scan_delkmsob_isok err\n");
		}
		if ((adr_t)fadrs >= (bexp->mt_vstat + sizeof(kmbext_t)) && ((adr_t)fadrs + (adr_t)fsz) <= bexp->mt_vend)
		{
			if (fsz <= kmsp->so_objsz)
			{
				return kmsp;
			}
		}
	}

	return NULL;
}

bool_t scan_nmszkmsob_isok(kmsob_t *kmsp, size_t msz)
{
	if (NULL == kmsp || 1 > msz)
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
	list_h_t *tmplst = NULL;
	kmbext_t *bextp = NULL;
	if (NULL == kmsp || NULL == fadrs || 1 > fsz)
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
	if (NULL == kmsp)
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
	kmsob_t *kmsp = NULL, *tkmsp = NULL;
	list_h_t *tmplst = NULL;
	if (NULL == koblp || 1 > msz) {
		return NULL;
	}
	
	// �����һ���ոշ������ڴ���������ģ���ڴ�أ��ܷ�����msz�ߴ��ڴ�����Ҫ��
	kmsp = scan_newkmsob_isok(koblp->ol_cahe, msz); 
	if (NULL != kmsp) {
		return kmsp;
	}

    // ��������ȥ���������ڴ�أ������ܷ�����Ҫ��
	if (0 < koblp->ol_emnr)
	{
		list_for_each(tmplst, &koblp->ol_emplst)
		{
			tkmsp = list_entry(tmplst, kmsob_t, so_list);
			kmsp = scan_newkmsob_isok(tkmsp, msz);
			if (NULL != kmsp)
			{
				return kmsp;
			}
		}
	}
	return NULL;
}

// �����ͷ��ڴ����������kmso_t�ṹ
kmsob_t *onkoblst_retn_delkmsob(koblst_t *koblp, void *fadrs, size_t fsz)
{
	kmsob_t *kmsp = NULL, *tkmsp = NULL;
	list_h_t *tmplst = NULL;
	if (NULL == koblp || NULL == fadrs || 1 > fsz)
	{
		return NULL;
	}
	// �����ϴθող�����kmsob_t�ṹ
	kmsp = scan_delkmsob_isok(koblp->ol_cahe, fadrs, fsz);
	if (NULL != kmsp)
	{
		return kmsp;
	}

	if (0 < koblp->ol_emnr)
	{
		list_for_each(tmplst, &koblp->ol_emplst)
		{
			tkmsp = list_entry(tmplst, kmsob_t, so_list);
			kmsp = scan_delkmsob_isok(tkmsp, fadrs, fsz);
			if (NULL != kmsp)
			{
				return kmsp;
			}
		}
	}
	return NULL;
}

// B1
koblst_t *onmsz_retn_koblst(kmsobmgrhed_t *kmmgrhlokp, size_t msz)
{
	if (NULL == kmmgrhlokp || 1 > msz)
	{
		return NULL;
	}
	for (uint_t kli = 0; kli < KOBLST_MAX; kli++)
	{
		if (kmmgrhlokp->ks_msoblst[kli].ol_sz >= msz)
		{
			return &kmmgrhlokp->ks_msoblst[kli];
		}
	}
	return NULL;
}

bool_t kmsob_add_koblst(koblst_t *koblp, kmsob_t *kmsp)
{
	if (NULL == koblp || NULL == kmsp) {
		return FALSE;
	}
	if (kmsp->so_objsz > koblp->ol_sz) {
		return FALSE;
	}
    list_add_to_before(&koblp->ol_emplst, &kmsp->so_list);
	// list_add(&kmsp->so_list, &koblp->ol_emplst);
	koblp->ol_emnr++;
	return TRUE;
}

// ����ĳ���ڴ�����С������ڴ��
kmsob_t *_create_init_kmsob(kmsob_t *kmsp, size_t objsz, adr_t cvadrs, adr_t cvadre, msadsc_t *msa, uint_t relpnr)
{
	if (NULL == kmsp || 1 > objsz || NULL == cvadrs || NULL == cvadre || NULL == msa || 1 > relpnr) {
		return NULL;
	}
	if (objsz < sizeof(freobjh_t)) {
		return NULL;
	}
	if ((cvadre - cvadrs + 1) < PAGESIZE) {
		return NULL;
	}
	if ((cvadre - cvadrs + 1) <= (sizeof(kmsob_t) + sizeof(freobjh_t))) {
		return NULL;
	}

	kmsob_t_init(kmsp);

	kmsp->so_vstat = cvadrs;
	kmsp->so_vend = cvadre;
	kmsp->so_objsz = objsz;
    
    // ��˫�������У�ͷ�ڵ��ǰ�����һ���ڵ㡣�����������β����һ���ڵ�
    list_add_to_before(&kmsp->so_mc.mc_kmobinlst, &msa->md_list);
	// list_add(&msa->md_list, &kmsp->so_mc.mc_kmobinlst);
	kmsp->so_mc.mc_kmobinpnr = (uint_t)relpnr;

    // ����ڴ�������ʼλ�úͽ���λ��
	freobjh_t *fohstat = (freobjh_t *)(kmsp + 1), *fohend = (freobjh_t *)cvadre;

	uint_t ap = (uint_t)((uint_t)fohstat);
	freobjh_t *tmpfoh = (freobjh_t *)((uint_t)ap);
	for (; tmpfoh < fohend;) // �����ڴ����
	{
		if ((ap + (uint_t)kmsp->so_objsz) <= (uint_t)cvadre)
		{
			freobjh_t_init(tmpfoh, 0, (void *)tmpfoh); // ��ʼ���ڴ�����ڴ����ͽ�����Ҫ������������̵�ÿ�������ڴ���
			list_add_to_before(&kmsp->so_frelst, &tmpfoh->oh_list);
			kmsp->so_mobjnr++;
			kmsp->so_fobjnr++;
		}
		ap += (uint_t)kmsp->so_objsz;
		tmpfoh = (freobjh_t *)((uint_t)ap);
	}
	return kmsp;
}

// B3 ����ĳ���ڴ�����С����չ�ڴ��
kmsob_t *_create_kmsob(kmsobmgrhed_t *kmmgrlokp, koblst_t *koblp, size_t objsz)
{
	if (NULL == kmmgrlokp || NULL == koblp || 1 > objsz) {
		return NULL;
	}

	kmsob_t *kmsp = NULL;
	msadsc_t *msa = NULL;
	uint_t relpnr = 0;
	uint_t pages = 1;
	if (128 < objsz) {
		pages = 2;
	}
	if (512 < objsz) {
		pages = 4;
	}
	
    // ��������ҳ�ڴ�
    msa = mm_division_pages(&glomm, pages, &relpnr, MA_TYPE_KRNL, DMF_RELDIV);
	if (NULL == msa) {
		return NULL;
	}
	if (NULL != msa && 0 == relpnr) {
		system_error("_create_kmsob mm_division_pages fail\n");
		return NULL;
	}

	u64_t phyadr = msa->md_phyadrs.paf_padrs << PSHRSIZE; // physical address start 
	u64_t phyade = phyadr + (relpnr << PSHRSIZE) - 1;   // physical address end 
	adr_t vadrs = Phy_To_Virt((adr_t)phyadr);
	adr_t vadre = Phy_To_Virt((adr_t)phyade);
	
    kmsp = _create_init_kmsob((kmsob_t *)vadrs, koblp->ol_sz, vadrs, vadre, msa, relpnr);
	if (NULL == kmsp)
	{
		if (mm_merge_pages(&glomm, msa, relpnr) == FALSE)
		{
			system_error("_create_kmsob mm_merge_pages fail\n");
		}
		return NULL;
	}
	if (kmsob_add_koblst(koblp, kmsp) == FALSE)
	{
		system_error(" _create_kmsob kmsob_add_koblst FALSE\n");
	}
	kmmgrlokp->ks_msobnr++;
	return kmsp;
}

// C1  ִ���ڴ����������
void *kmsob_new_opkmsob(kmsob_t *kmsp, size_t msz)
{
	if (NULL == kmsp || 1 > msz) {
		return NULL;
	}
	if (scan_nmszkmsob_isok(kmsp, msz) == FALSE) {
		return NULL;
	}
	
    freobjh_t *fobh = list_entry(kmsp->so_frelst.next, freobjh_t, oh_list);
	list_del(&fobh->oh_list);
	kmsp->so_fobjnr--;
	
    return (void *)(fobh);
}

// C2 ��չ�ڴ��
bool_t kmsob_extn_pages(kmsob_t *kmsp)
{
	if (NULL == kmsp) {
		return FALSE;
	}
	if ((~0UL) <= kmsp->so_mobjnr || (~0UL) <= kmsp->so_mextnr || (~0UL) <= kmsp->so_fobjnr) {
		return FALSE;
	}

	msadsc_t *msa = NULL;
	uint_t relpnr = 0;
	uint_t pages = 1;
	if (128 < kmsp->so_objsz) {
		pages = 2;
	}
	if (512 < kmsp->so_objsz) {
		pages = 4;
	}

	msa = mm_division_pages(&glomm, pages, &relpnr, MA_TYPE_KRNL, DMF_RELDIV);
	if (NULL == msa) {
		return FALSE;
	}
	if (NULL != msa && 0 == relpnr) {
		system_error("kmsob_extn_pages mm_division_pages fail\n");
		return FALSE;
	}

	u64_t phyadr = msa->md_phyadrs.paf_padrs << PSHRSIZE;
	u64_t phyade = phyadr + (relpnr << PSHRSIZE) - 1;
	adr_t vadrs = Phy_To_Virt((adr_t)phyadr);
	adr_t vadre = Phy_To_Virt((adr_t)phyade);
	sint_t mscidx = retn_mscidx(relpnr);
	if (MSCLST_MAX <= mscidx || 0 > mscidx)
	{ 
		if (mm_merge_pages(&glomm, msa, relpnr) == FALSE)
		{ // ���ʹ��󣬻����ڴ�ҳ
			system_error("kmsob_extn_pages mm_merge_pages fail\n");
		}
		return FALSE;
	}

    list_add_to_before(&kmsp->so_mc.mc_lst[mscidx].ml_list, &msa->md_list);
	// list_add(&msa->md_list, &kmsp->so_mc.mc_lst[mscidx].ml_list);
	kmsp->so_mc.mc_lst[mscidx].ml_msanr++;

	kmbext_t *bextp = (kmbext_t *)vadrs;
	kmbext_t_init(bextp, vadrs, vadre, kmsp);

	freobjh_t *fohstat = (freobjh_t *)(bextp + 1), *fohend = (freobjh_t *)vadre;
	uint_t ap = (uint_t)((uint_t)fohstat);
	freobjh_t *tmpfoh = (freobjh_t *)((uint_t)ap);
	for (; tmpfoh < fohend;)
	{
		//ap+=(uint_t)kmsp->so_objsz;
		if ((ap + (uint_t)kmsp->so_objsz) <= (uint_t)vadre)
		{
			freobjh_t_init(tmpfoh, 0, (void *)tmpfoh);
			list_add_to_behind(&kmsp->so_frelst, &tmpfoh->oh_list);
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
	if (NULL == kmsp || 1 > msz) {
		return NULL;
	}
	void *retptr = NULL;
	cpuflg_t cpuflg;
	// knl_spinlock_cli(&kmsp->so_lock, &cpuflg);
	if (scan_kmsob_objnr(kmsp) < 1)
	{ // ��ǰ�ڴ���ڴ�����㣬GOTO: ��չ�ڴ��
		if (kmsob_extn_pages(kmsp) == FALSE)
		{
			retptr = NULL;
			goto ret_step;
		}
	}
	retptr = kmsob_new_opkmsob(kmsp, msz);
ret_step:
	// knl_spinunlock_sti(&kmsp->so_lock, &cpuflg);
	return retptr;
}

// A
void *kmsob_new_core(size_t msz)
{
	kmsobmgrhed_t *kmobmgrp = &glomm.mo_kmsobmgr;//��ȡkmsobmgrhed_t�ṹ�ĵ�ַ
	void *retptr = NULL;
	koblst_t *koblp = NULL;
	kmsob_t *kmsp = NULL;
	cpuflg_t cpuflg;
	// knl_spinlock_cli(&kmobmgrp->ks_lock, &cpuflg);
	
	// �����ڴ�����С���Ҳ����� koblst_t�ṹָ��
	koblp = onmsz_retn_koblst(kmobmgrp, msz);
	if (NULL == koblp) {
		retptr = NULL;
		goto ret_step;
	}

    // ��koblst_t�ṹ�л�ȡ kmsob_t�ṹ(�ڴ�ؽṹ)
	kmsp = onkoblst_retn_newkmsob(koblp, msz);
	if (NULL == kmsp) {
        // û��msz��С���ڴ�أ���ȥ����������С��һ���ڴ�أ����ص�koblst_t�ṹ��
		kmsp = _create_kmsob(kmobmgrp, koblp, koblp->ol_sz);
		if (NULL == kmsp) {
			retptr = NULL;
			goto ret_step;
		}
	}

	retptr = kmsob_new_onkmsob(kmsp, msz);
	if (NULL == retptr) {
		retptr = NULL;
		goto ret_step;
	}

	//����kmsobmgrhed_t�ṹ����Ϣ
	kmsob_updata_cache(kmobmgrp, koblp, kmsp, KUC_NEWFLG);
ret_step:
	// knl_spinunlock_sti(&kmobmgrp->ks_lock, &cpuflg);
	return retptr;
}
// �ڴ�������ӿ�
void *kmsob_new(size_t msz)
{
	if (1 > msz || 2048 < msz)
	//����С��1 ���� ����2048�ֽڵĴ�С��֧�� ֱ�ӷ���NULL��ʾʧ��
	{
		return NULL;
	}
	return kmsob_new_core(msz);
}