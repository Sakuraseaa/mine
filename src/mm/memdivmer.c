#include "basetype.h"
#include "memmgrob.h"
#include "marea_t.h"
#include "msadsc_t.h"
#include "pages64_t.h"
#include "memdivmer_t.h"
#include "printk.h"
#include "lib.h"

extern memmgrob_t glomm;
// 更新整个管理结构的计数
void mm_update_memmgrob(uint_t realpnr, uint_t flgs)
{
	cpuflg_t cpuflg;
	if (0 == flgs)
	{
		//knl_spinlock_cli(&memmgrob.mo_lock, &cpuflg);
		glomm.mo_alocpages += realpnr;
		glomm.mo_freepages -= realpnr;
		//knl_spinunlock_sti(&memmgrob.mo_lock, &cpuflg);
	}
	if (1 == flgs)
	{
		//knl_spinlock_cli(&memmgrob.mo_lock, &cpuflg);
		glomm.mo_alocpages -= realpnr;
		glomm.mo_freepages += realpnr;
		//knl_spinunlock_sti(&memmgrob.mo_lock, &cpuflg);
	}
	return;
}

// 更新内存区
void mm_update_memarea(memarea_t *malokp, uint_t pgnr, uint_t flgs)
{
	if (NULL == malokp)
	{
		return;
	}
	if (0 == flgs) // 分割
	{
		malokp->ma_freepages -= pgnr;
		malokp->ma_allocpages += pgnr;
	}
	if (1 == flgs) // 合并
	{
		malokp->ma_freepages += pgnr;
		malokp->ma_allocpages -= pgnr;
	}
	return;
}

KLINE sint_t retn_divoder(uint_t pages)
{
	
	sint_t pbits = search_64rlbits((uint_t)pages) - 1;
	
	// 检查 pages 是否是2的幂
	// 如果 pages 是2的幂，那么它的二进制表示只有一个位是1，其余都是0。对于这种情况，pages & (pages - 1) 的结果是0（例如，8 & 7 = 0）。
	// 如果 pages 不是2的幂，pages & (pages - 1) 的结果将不是0，表示有多于一个的1位存在。这时，pbits 增加1
	if (pages & (pages - 1))
	{
		pbits++;
	}
	return pbits;
}

// C_1 获取释放msadsc_t结构所在的内存区
memarea_t *onfrmsa_retn_marea(memmgrob_t *mmobjp, msadsc_t *freemsa, uint_t freepgs)
{

	if (MF_OLKTY_ODER != freemsa->md_indxflgs.mf_olkty || NULL == freemsa->md_odlink)
	{
		return NULL;
	}
	msadsc_t *fmend = (msadsc_t *)freemsa->md_odlink;
	if (((uint_t)(fmend - freemsa) + 1) != freepgs)
	{
		return NULL;
	}
	if (freemsa->md_indxflgs.mf_marty != fmend->md_indxflgs.mf_marty)
	{
		return NULL;
	}

	for (uint_t mi = 0; mi < mmobjp->mo_mareanr; mi++)
	{
		if ((uint_t)(freemsa->md_indxflgs.mf_marty) == mmobjp->mo_mareastat[mi].ma_type)
		{
			return &mmobjp->mo_mareastat[mi];
		}
	}
	return NULL;
}

/**
 * @brief B_1获得 type 类型的内存区指针
 * 
 * @param mmobjp 
 * @param mrtype 
 * @return memarea_t* 
 */
memarea_t *onmrtype_retn_marea(memmgrob_t *mmobjp, uint_t mrtype)
{
	for (uint_t mi = 0; mi < mmobjp->mo_mareanr; mi++)
	{
		if (mrtype == mmobjp->mo_mareastat[mi].ma_type)
		{
			return &mmobjp->mo_mareastat[mi];
		}
	}
	return NULL;
}

bafhlst_t *onma_retn_maxbafhlst(memarea_t *malckp) // 得到一个有空闲结点且最大的链表
{
	for (s64_t li = (MDIVMER_ARR_LMAX - 1); li >= 0; li--)
	{
		if (malckp->ma_mdmdata.dm_mdmlielst[li].af_fobjnr > 0)
		{
			return &malckp->ma_mdmdata.dm_mdmlielst[li];
		}
	}
	return NULL;
}

// E3 设置节点被申请后的属性
msadsc_t *mm_divpages_opmsadsc(msadsc_t *msastat, uint_t mnr)
{
	if (NULL == msastat || 0 == mnr) {
		return NULL;
	}

	if ((MF_OLKTY_ODER != msastat->md_indxflgs.mf_olkty &&
		MF_OLKTY_BAFH != msastat->md_indxflgs.mf_olkty) ||
		NULL == msastat->md_odlink ||
		PAF_NO_ALLOC != msastat->md_phyadrs.paf_alloc)
	{
		color_printk(RED, BLACK, "mm_divpages_opmsadsc err1\n");
	}

	msadsc_t *mend = (msadsc_t *)msastat->md_odlink;
	if (MF_OLKTY_BAFH == msastat->md_indxflgs.mf_olkty) { // 用户？
		mend = msastat;
	}

	if (mend < msastat) {
		color_printk(RED, BLACK, "mm_divpages_opmsadsc err2\n");
	}

	if ((uint_t)((mend - msastat) + 1) != mnr) {
		color_printk(RED, BLACK, "mm_divpages_opmsadsc err3\n");
	}
	if (msastat->md_indxflgs.mf_uindx > (MF_UINDX_MAX - 1) || mend->md_indxflgs.mf_uindx > (MF_UINDX_MAX - 1) ||
		msastat->md_indxflgs.mf_uindx != mend->md_indxflgs.mf_uindx)
	{
		color_printk(RED, BLACK, "mm_divpages_opmsadsc err4");
	}
	
	if (mend == msastat)
	{
		msastat->md_indxflgs.mf_uindx++;
		msastat->md_phyadrs.paf_alloc = PAF_ALLOC;
		msastat->md_indxflgs.mf_olkty = MF_OLKTY_ODER;
		msastat->md_odlink = mend;
		return msastat;
	}

	msastat->md_indxflgs.mf_uindx++;
	msastat->md_phyadrs.paf_alloc = PAF_ALLOC;
	mend->md_indxflgs.mf_uindx++;
	mend->md_phyadrs.paf_alloc = PAF_ALLOC;
	//msastat->md_indxflgs.mf_olkty = MF_OLKTY_ODER; // 是否有错？mend->md_indxflgs.mf_olkty = MF_OLKTY_ODER
	//msastat->md_odlink = mend; // 是否有错？ mend->md_odlink = msastat;
	mend->md_indxflgs.mf_olkty = MF_OLKTY_ODER;
	mend->md_odlink = msastat;
	return msastat;
}

#if 0
int_t mm_merpages_opmsadsc(bafhlst_t *bafh, msadsc_t *freemsa, uint_t freepgs)
{
    msadsc_t *fmend = (msadsc_t *)freemsa->md_odlink;
    //处理只有一个单页的情况
    if (freemsa == fmend)
    {
        //页面的分配计数减1
        freemsa->md_indxflgs.mf_uindx--;
        if (0 < freemsa->md_indxflgs.mf_uindx)
        {//如果依然大于0说明它是共享页面 直接返回1指示不需要进行下一步操作
            return 1;
        }
        //设置页未分配的标志
        freemsa->md_phyadrs.paf_alloc = PAF_NO_ALLOC;
        freemsa->md_indxflgs.mf_olkty = MF_OLKTY_BAFH;
        freemsa->md_odlink = bafh;//指向所属的bafhlst_t结构
        //返回2指示需要进行下一步操作
        return 2;
    }
    //多个页面的起始页面和结束页面都要减一
    freemsa->md_indxflgs.mf_uindx--;
    fmend->md_indxflgs.mf_uindx--;
    //如果依然大于0说明它是共享页面 直接返回1指示不需要进行下一步操作
    if (0 < freemsa->md_indxflgs.mf_uindx)
    {
        return 1;
    }
    //设置起始、结束页页未分配的标志
    freemsa->md_phyadrs.paf_alloc = PAF_NO_ALLOC;
    fmend->md_phyadrs.paf_alloc = PAF_NO_ALLOC;
    freemsa->md_indxflgs.mf_olkty = MF_OLKTY_ODER;
    //起始页面指向结束页面
    freemsa->md_odlink = fmend;
    fmend->md_indxflgs.mf_olkty = MF_OLKTY_BAFH;
    //结束页面指向所属的bafhlst_t结构
    fmend->md_odlink = bafh;
    //返回2指示需要进行下一步操作
    return 2;
}

bool_t onfpgs_retn_bafhlst(memarea_t *malckp, uint_t freepgs, bafhlst_t **retrelbf, bafhlst_t **retmerbf)
{
    //获取bafhlst_t结构数组的开始地址
    bafhlst_t *bafhstat = malckp->ma_mdmdata.dm_mdmlielst;
    //根据分配页面数计算出分配页面在dm_mdmlielst数组中下标
    sint_t dividx = retn_divoder(freepgs);
    //返回请求释放的bafhlst_t结构指针
    *retrelbf = &bafhstat[dividx];
    //返回最大释放的bafhlst_t结构指针
    *retmerbf = &bafhstat[MDIVMER_ARR_LMAX - 1];
    return TRUE;
}

bool_t mm_merpages_onmarea(memarea_t *malckp, msadsc_t *freemsa, uint_t freepgs)
{
    bafhlst_t *prcbf = NULL;
    sint_t pocs = 0;
    bafhlst_t *retrelbf = NULL, *retmerbf = NULL;
    bool_t rets = FALSE;
    //根据freepgs返回请求释放的和最大释放的bafhlst_t结构指针
    rets = onfpgs_retn_bafhlst(malckp, freepgs, &retrelbf, &retmerbf);
    //设置msadsc_t结构的信息，完成释放，返回1表示不需要下一步合并操作，返回2表示要进行合并操作
    sint_t mopms = mm_merpages_opmsadsc(retrelbf, freemsa, freepgs);
    if (2 == mopms)
    {
        //把msadsc_t结构进行合并然后加入对应bafhlst_t结构
        return mm_merpages_onbafhlst(freemsa, freepgs, retrelbf, retmerbf);
    }
    if (1 == mopms)
    {
        return TRUE;
    }
    return FALSE;
}
#endif

//E-2 设置msadsc_t结构的信息，完成释放，返回1表示不需要下一步合并操作，返回2表示要进行合并操作
sint_t mm_merpages_opmsadsc(bafhlst_t *bafh, msadsc_t *freemsa, uint_t freepgs)
{
	if (NULL == bafh || NULL == freemsa || 1 > freepgs) {
		return 0;
	}
	if (MF_OLKTY_ODER != freemsa->md_indxflgs.mf_olkty || NULL == freemsa->md_odlink) {
		system_error("mm_merpages_opmsadsc err1\n");
	}
	msadsc_t *fmend = (msadsc_t *)freemsa->md_odlink;
	if (fmend < freemsa) {
		system_error("mm_merpages_opmsadsc err2\n");
	}
	if (bafh->af_oderpnr != freepgs || ((uint_t)(fmend - freemsa) + 1) != freepgs) {
		system_error("mm_merpages_opmsadsc err3\n");
	}
	if (PAF_NO_ALLOC == freemsa->md_phyadrs.paf_alloc || 1 > freemsa->md_indxflgs.mf_uindx) {
		system_error("mm_merpages_opmsadsc err4\n");
	}
	if (PAF_NO_ALLOC == fmend->md_phyadrs.paf_alloc || 1 > fmend->md_indxflgs.mf_uindx) {
		system_error("mm_merpages_opmsadsc err5\n");
	}
	if (freemsa->md_indxflgs.mf_uindx != fmend->md_indxflgs.mf_uindx) {
		system_error("mm_merpages_opmsadsc err6\n");
	}
	if (freemsa == fmend) {
		freemsa->md_indxflgs.mf_uindx--;
		if (0 < freemsa->md_indxflgs.mf_uindx) { // 共享返回1
			return 1;
		}
		freemsa->md_phyadrs.paf_alloc = PAF_NO_ALLOC;
		freemsa->md_indxflgs.mf_olkty = MF_OLKTY_BAFH;
		freemsa->md_odlink = bafh;
		return 2;
	}

	freemsa->md_indxflgs.mf_uindx--;
	fmend->md_indxflgs.mf_uindx--;
	if (0 < freemsa->md_indxflgs.mf_uindx) {
		return 1;
	}
	freemsa->md_phyadrs.paf_alloc = PAF_NO_ALLOC;
	fmend->md_phyadrs.paf_alloc = PAF_NO_ALLOC;
	
	freemsa->md_indxflgs.mf_olkty = MF_OLKTY_ODER;
	freemsa->md_odlink = fmend;
	fmend->md_indxflgs.mf_olkty = MF_OLKTY_BAFH;
	fmend->md_odlink = bafh;
	return 2;
}

// D2: 从内存区域中，得到具体的bhfhlst_t
// 返回请求分配的bafhlst_t结构指针
// 返回实际分配的bafhlst_t结构指针
bool_t onmpgs_retn_bafhlst(memarea_t *malckp, uint_t pages, bafhlst_t **retrelbafh, bafhlst_t **retdivbafh)
{
	if (NULL == malckp || 1 > pages || NULL == retrelbafh || NULL == retdivbafh) {
		return FALSE;
	}
	bafhlst_t *bafhstat = malckp->ma_mdmdata.dm_mdmlielst;
	sint_t dividx = retn_divoder(pages); // 把页面数向上对齐到2的整数次幂
	
	if (0 > dividx || MDIVMER_ARR_LMAX <= dividx) {
		*retrelbafh = NULL;
		*retdivbafh = NULL;
		return FALSE;
	}

	if (pages > bafhstat[dividx].af_oderpnr) { 
		*retrelbafh = NULL;
		*retdivbafh = NULL;
		return FALSE;
	}
	
	for (sint_t idx = dividx; idx < MDIVMER_ARR_LMAX; idx++)
	{ // 请求的page页对应的bafhstat的链表不一定有空闲节点，所以向上寻求更大的页链表，然后分割他的大的空闲节点
		if (bafhstat[idx].af_oderpnr >= pages && 0 < bafhstat[idx].af_fobjnr)
		{
			*retrelbafh = &bafhstat[dividx];  // 请求分配的bafhlst_t结构指针
			*retdivbafh = &bafhstat[idx];	 // 实际分配的bafhlst_t结构指针
			return TRUE;
		}
	}

	*retrelbafh = NULL;
	*retdivbafh = NULL;
	return FALSE;
}


// E-1 根据freepgs返回请求释放的和最大释放的bafhlst_t结构指针
#if 0
bool_t onfpgs_retn_bafhlst(memarea_t *malckp, uint_t freepgs, bafhlst_t **retrelbf, bafhlst_t **retmerbf)
{    
	//获取bafhlst_t结构数组的开始地址    
	bafhlst_t *bafhstat = malckp->ma_mdmdata.dm_mdmlielst;    
	//根据分配页面数计算出分配页面在dm_mdmlielst数组中下标    
	sint_t dividx = retn_divoder(freepgs);    
	//返回请求释放的bafhlst_t结构指针    
	*retrelbf = &bafhstat[dividx];    
	//返回最大释放的bafhlst_t结构指针    
	*retmerbf = &bafhstat[MDIVMER_ARR_LMAX - 1];    
	return TRUE;
}
#endif

// 	*retrelbf 请求释放的页，位于的bafhlst_t结构指针
//	*retmerbf 最大的内存的页，位于的bafhlst_t结构指针
bool_t onfpgs_retn_bafhlst(memarea_t *malckp, uint_t freepgs, bafhlst_t **retrelbf, bafhlst_t **retmerbf)
{
	if (NULL == malckp || 1 > freepgs || NULL == retrelbf || NULL == retmerbf) {
		return FALSE;
	}
	bafhlst_t *bafhstat = malckp->ma_mdmdata.dm_mdmlielst;
	sint_t dividx = retn_divoder(freepgs);
	if (0 > dividx || MDIVMER_ARR_LMAX <= dividx) {
		*retrelbf = NULL;
		*retmerbf = NULL;
		return FALSE;
	}
	if ((~0UL) <= bafhstat[dividx].af_mobjnr) {
		system_error("onfpgs_retn_bafhlst af_mobjnr max");
	}
	if ((~0UL) <= bafhstat[dividx].af_fobjnr) {
		system_error("onfpgs_retn_bafhlst af_fobjnr max");
	}

	if (freepgs != bafhstat[dividx].af_oderpnr) {
		*retrelbf = NULL;
		*retmerbf = NULL;
		return FALSE;
	}
	*retrelbf = &bafhstat[dividx];
	*retmerbf = &bafhstat[MDIVMER_ARR_LMAX - 1];
	return TRUE;
}

msadsc_t *mm_divipages_onbafhlst(bafhlst_t *bafhp)
{
	if (NULL == bafhp)
	{
		return NULL;
	}
	if (1 > bafhp->af_fobjnr)
	{
		return NULL;
	}
	if (list_is_empty_careful(&bafhp->af_frelst) == TRUE)
	{
		return NULL;
	}
	msadsc_t *tmp = list_entry(bafhp->af_frelst.next, msadsc_t, md_list);
	list_del(&tmp->md_list);
	tmp->md_indxflgs.mf_uindx++;
	tmp->md_phyadrs.paf_alloc = PAF_ALLOC;
	bafhp->af_fobjnr--;
	bafhp->af_mobjnr--;
	bafhp->af_alcindx++;
	return tmp;
}


// E1, 获得bafh list上的一个节点，用mstat 和 mend记录这个节点的信息
bool_t mm_retnmsaob_onbafhlst(bafhlst_t *bafhp, msadsc_t **retmstat, msadsc_t **retmend)
{
	if (NULL == bafhp || NULL == retmstat || NULL == retmend) {
		return FALSE;
	}
	if (1 > bafhp->af_mobjnr || 1 > bafhp->af_fobjnr) {
		*retmstat = NULL;
		*retmend = NULL;
		return FALSE;
	}
	
	if (list_is_empty_careful(&bafhp->af_frelst) == TRUE){
		*retmstat = NULL;
		*retmend = NULL;
		return FALSE;
	}
	
	msadsc_t *tmp = list_entry(bafhp->af_frelst.next, msadsc_t, md_list);
	list_del(&tmp->md_list);
	bafhp->af_mobjnr--; // 减少总页面数
	bafhp->af_fobjnr--; // 减少空闲页面数
	bafhp->af_freindx++; // 增加释放计数
	
	*retmstat = tmp;
	*retmend = (msadsc_t *)tmp->md_odlink;
	if (MF_OLKTY_BAFH == tmp->md_indxflgs.mf_olkty) {
		*retmend = tmp;
	}

	return TRUE;
}

// D1 确保内存区malckp 有 pages 个页
bool_t scan_mapgsalloc_ok(memarea_t *malckp, uint_t pages)
{
	if (NULL == malckp || 1 > pages) {
		return FALSE;
	}
	if (malckp->ma_freepages >= pages && malckp->ma_maxpages >= pages) {
		return TRUE;
	}
	return FALSE;
}

// C2
msadsc_t *mm_maxdivpages_onmarea(memarea_t *malckp, uint_t *retrelpnr)
{
	bafhlst_t *bafhp = onma_retn_maxbafhlst(malckp);
	if (NULL == bafhp)
	{
		*retrelpnr = 0;
		return NULL;
	}
	msadsc_t *retmsa = NULL;

	msadsc_t *retmstat = NULL, *retmend = NULL;
	bool_t rets = mm_retnmsaob_onbafhlst(bafhp, &retmstat, &retmend);
	if (FALSE == rets || NULL == retmstat || NULL == retmend)
	{
		*retrelpnr = 0;
		return NULL;
	}
	retmsa = mm_divpages_opmsadsc(retmstat, bafhp->af_oderpnr);

	if (NULL == retmsa)
	{
		*retrelpnr = 0;
		return NULL;
	}
	*retrelpnr = bafhp->af_oderpnr;
	return retmsa;
}

uint_t chek_divlenmsa(msadsc_t *msastat, msadsc_t *msaend, uint_t mnr)
{
	uint_t ok = 0;
	msadsc_t *ms = msastat, *me = msaend;
	if (NULL == msastat || NULL == msaend || 0 == mnr)
	{
		return 0;
	}
	if ((uint_t)(msaend - msastat + 1) != mnr)
	{
		return 0;
	}
	if (1 == mnr)
	{
		if (0 < msastat->md_indxflgs.mf_uindx)
		{
			return 0;
		}
		if (PAF_NO_ALLOC != msastat->md_phyadrs.paf_alloc)
		{
			return 0;
		}
		if (list_is_empty_careful(&msastat->md_list) == FALSE)
		{
			return 0;
		}
		return ok + 1;
	}
	for (; ms < me; ms++)
	{
		if (ms->md_indxflgs.mf_uindx != 0 ||
			(ms + 1)->md_indxflgs.mf_uindx != 0)
		{
			return 0;
		}
		if (PAF_NO_ALLOC != ms->md_phyadrs.paf_alloc ||
			PAF_NO_ALLOC != (ms + 1)->md_phyadrs.paf_alloc)
		{
			return 0;
		}
		if (list_is_empty_careful(&ms->md_list) == FALSE || list_is_empty_careful(&((ms + 1)->md_list)) == FALSE)
		{
			return 0;
		}
		if (PAGESIZE != (((ms + 1)->md_phyadrs.paf_padrs << PSHRSIZE) - (ms->md_phyadrs.paf_padrs << PSHRSIZE)))
		{
			return 0;
		}
		ok++;
	}
	if (0 == ok)
	{
		return 0;
	}
	if ((ok + 1) != mnr)
	{
		return 0;
	}
	return ok;
}

// E2 分割动作
bool_t mrdmb_add_msa_bafh(bafhlst_t *bafhp, msadsc_t *msastat, msadsc_t *msaend)
{
	if (NULL == bafhp || NULL == msastat || NULL == msaend) {
		return FALSE;
	}
	uint_t mnr = (msaend - msastat) + 1;
	if (mnr != (uint_t)bafhp->af_oderpnr) {
		return FALSE;
	}
	if (0 < msastat->md_indxflgs.mf_uindx || 0 < msaend->md_indxflgs.mf_uindx) {
		return FALSE;
	}
	if (PAF_NO_ALLOC != msastat->md_phyadrs.paf_alloc ||
		PAF_NO_ALLOC != msaend->md_phyadrs.paf_alloc) {
		return FALSE;
	}

	if (msastat == msaend && 1 != mnr && 1 != bafhp->af_oderpnr) {
		return FALSE;
	}

	msastat->md_indxflgs.mf_olkty = MF_OLKTY_ODER;
	msastat->md_odlink = msaend;

	msaend->md_indxflgs.mf_olkty = MF_OLKTY_BAFH;
	msaend->md_odlink = bafhp;
	list_add_to_behind(&bafhp->af_frelst, &msastat->md_list);
	bafhp->af_mobjnr++;
	bafhp->af_fobjnr++;
	return TRUE;
}

// D2 从bafhlst中划分物理页
msadsc_t *mm_reldpgsdivmsa_bafhl(memarea_t *malckp, uint_t pages, uint_t *retrelpnr, bafhlst_t *relbfl, bafhlst_t *divbfl)
{
	msadsc_t *retmsa = NULL;
	bool_t rets = FALSE;
	msadsc_t *retmstat = NULL, *retmend = NULL;
	if (NULL == malckp || 1 > pages || NULL == retrelpnr || NULL == relbfl || NULL == divbfl) {
		return NULL;
	}
	if (relbfl > divbfl) {
		*retrelpnr = 0;
		return NULL;
	}
	if (relbfl == divbfl)
	{
		//  获得bafh list上的一个节点，用mstat 和 mend记录这个节点的信息
		rets = mm_retnmsaob_onbafhlst(relbfl, &retmstat, &retmend);
		if (FALSE == rets || NULL == retmstat || NULL == retmend) {
			*retrelpnr = 0;
			return NULL;
		}
		if ((uint_t)((retmend - retmstat) + 1) != relbfl->af_oderpnr) {
			*retrelpnr = 0;
			return NULL;
		}
		
		// 设置实际的分配页数
		retmsa = mm_divpages_opmsadsc(retmstat, relbfl->af_oderpnr);
		if (NULL == retmsa) {
			*retrelpnr = 0;
			return NULL;
		}
		
		// 返回实际的分配页数
		*retrelpnr = relbfl->af_oderpnr;
		return retmsa;
	}
	
	
	// 获得bafh list上的一个节点，用mstat 和 mend记录这个节点的信息
	rets = mm_retnmsaob_onbafhlst(divbfl, &retmstat, &retmend);
	if (FALSE == rets || NULL == retmstat || NULL == retmend) {
		*retrelpnr = 0;
		return NULL;
	}
	if ((uint_t)((retmend - retmstat) + 1) != divbfl->af_oderpnr) {
		*retrelpnr = 0;
		return NULL;
	}
	uint_t divnr = divbfl->af_oderpnr;
	// 按照2的次幂划分 divnr的大节点(从高端向低端划分，核心操作)，把他们挂载到低端合适的链表上
	for (bafhlst_t *tmpbfl = divbfl - 1; tmpbfl >= relbfl; tmpbfl--)
	{
		if (mrdmb_add_msa_bafh(tmpbfl, &retmstat[tmpbfl->af_oderpnr], (msadsc_t *)retmstat->md_odlink) == FALSE) {
			color_printk(RED, BLACK, "mrdmb_add_msa_bafh fail\n");
		}
		retmstat->md_odlink = &retmstat[tmpbfl->af_oderpnr - 1];
		divnr -= tmpbfl->af_oderpnr; // 划分后的，剩余页数
	}

	retmsa = mm_divpages_opmsadsc(retmstat, divnr);
	if (NULL == retmsa) {
		*retrelpnr = 0;
		return NULL;
	}
	*retrelpnr = relbfl->af_oderpnr;
	return retmsa;
}

/**
 * @brief C1
 * 
 * @param malckp 内存区指针
 * @param pages 请求分配的内存页面数
 * @param retrelpnr 存放实际分配内存页面数的指针
 * @return msadsc_t* 
 */
msadsc_t *mm_reldivpages_onmarea(memarea_t *malckp, uint_t pages, uint_t *retrelpnr)
{
	if (NULL == malckp || 1 > pages || NULL == retrelpnr) {
		return NULL;
	}
	
	if (scan_mapgsalloc_ok(malckp, pages) == FALSE) {
		*retrelpnr = 0;
		return NULL;
	}

	bafhlst_t *retrelbhl = NULL, *retdivbhl = NULL;

	//根据页面数在内存区的m_mdmlielst数组中找出下面两个东西
	//1. 请求分配页面的bafhlst_t结构（retrelbhl）
	//2. 实际要在其中分配页面的bafhlst_t结构(retdivbhl)
	bool_t rets = onmpgs_retn_bafhlst(malckp, pages, &retrelbhl, &retdivbhl);
	if (FALSE == rets) {
		*retrelpnr = 0;
		return NULL;
	}

	//实际在bafhlst_t结构中划分出内存
	uint_t retpnr = 0;
	msadsc_t *retmsa = mm_reldpgsdivmsa_bafhl(malckp, pages, &retpnr, retrelbhl, retdivbhl);
	if (NULL == retmsa) {
		*retrelpnr = 0;
		return NULL;
	}
	*retrelpnr = retpnr;
	return retmsa;
}

msadsc_t *mm_prcdivpages_onmarea(memarea_t *malckp, uint_t pages, uint_t *retrelpnr)
{
	if (NULL == malckp || NULL == retrelpnr || 1 != pages)
	{
		return NULL;
	}
	if (MA_TYPE_PROC != malckp->ma_type)
	{
		*retrelpnr = 0;
		return NULL;
	}
	if (scan_mapgsalloc_ok(malckp, pages) == FALSE)
	{
		*retrelpnr = 0;
		return NULL;
	}
	bafhlst_t *prcbfh = &malckp->ma_mdmdata.dm_onemsalst;
	bool_t rets = FALSE;
	msadsc_t *retmsa = NULL, *retmstat = NULL, *retmend = NULL;
	rets = mm_retnmsaob_onbafhlst(prcbfh, &retmstat, &retmend);
	if (FALSE == rets || NULL == retmstat || NULL == retmend)
	{
		*retrelpnr = 0;
		return NULL;
	}
	if ((uint_t)((retmend - retmstat) + 1) != prcbfh->af_oderpnr)
	{
		*retrelpnr = 0;
		return NULL;
	}
	retmsa = mm_divpages_opmsadsc(retmstat, prcbfh->af_oderpnr);
	if (NULL == retmsa)
	{
		*retrelpnr = 0;
		return NULL;
	}
	*retrelpnr = prcbfh->af_oderpnr;
	return retmsa;
}

/**
 * @brief B2
 * 
 * @param mareap 内存区指针
 * @param pages 请求分配的内存页面数
 * @param retrealpnr 存放实际分配内存页面数的指针
 * @param flgs 请求分配的内存页面的标志位
 * @return msadsc_t* 
 */
msadsc_t *mm_divpages_core(memarea_t *mareap, uint_t pages, uint_t *retrealpnr, uint_t flgs)
{
	uint_t retpnr = 0;
	msadsc_t *retmsa = NULL; //,*tmpmsa=NULL;
	cpuflg_t cpuflg;
	if (DMF_RELDIV != flgs && DMF_MAXDIV != flgs) {
		*retrealpnr = 0;
		return NULL;
	}
	if (MA_TYPE_KRNL != mareap->ma_type && MA_TYPE_HWAD != mareap->ma_type) { // 只给内核分配
		*retrealpnr = 0; 
		return NULL;
	}
	
	// knl_spinlock_cli(&mareap->ma_lock, &cpuflg); // 上锁

	if (DMF_MAXDIV == flgs)
	{	// 每次分配页，只给用户分配？
		retmsa = mm_maxdivpages_onmarea(mareap, &retpnr);
		goto ret_step;
	}
	if (DMF_RELDIV == flgs)
	{
		retmsa = mm_reldivpages_onmarea(mareap, pages, &retpnr);
		goto ret_step;
	}
	
	retmsa = NULL;
	retpnr = 0;
ret_step:
	if (NULL != retmsa && 0 != retpnr)
	{
		mm_update_memarea(mareap, retpnr, 0);
		mm_update_memmgrob(retpnr, 0);
	}
	
	// knl_spinunlock_sti(&mareap->ma_lock, &cpuflg);
	
	*retrealpnr = retpnr;
	return retmsa;
}

// a. 分配内存入口函数
msadsc_t *mm_divpages_fmwk(memmgrob_t *mmobjp, uint_t pages, uint_t *retrelpnr, uint_t mrtype, uint_t flgs)
{
	// 返回 对应的内存区结构指针
	memarea_t *marea = onmrtype_retn_marea(mmobjp, mrtype);
	if (NULL == marea) {
		*retrelpnr = 0;
		return NULL;
	}

	// 内存分配的核心函数
	uint_t retpnr = 0;
	msadsc_t *retmsa = mm_divpages_core(marea, pages, &retpnr, flgs);
	if (NULL == retmsa) {
		*retrelpnr = 0;
		return NULL;
	}

	*retrelpnr = retpnr;
	return retmsa;
}

/**
 * @brief 内存分配页面接口
 * 
 * @param mmobjp 内存管理数据结构指针
 * @param pages 请求分配的内存页面数
 * @param retrealpnr 存放实际分配内存页面数的指针
 * @param mrtype 请求的分配内存页面的内存区类型
 * @param flgs 请求分配的内存页面的标志位
 * 			flgs == 1: 表示获取该内存区中，尺寸最大的一个内存块
 *  		flgs == 0: 表示获取 pages * 4KB 大小的内存块 
 * @return msadsc_t* 
 */
msadsc_t *mm_division_pages(memmgrob_t *mmobjp, uint_t pages, uint_t *retrealpnr, uint_t mrtype, uint_t flgs)
{
	if (NULL == mmobjp || NULL == retrealpnr || 0 == mrtype) {
		return NULL;
	}

	uint_t retpnr = 0;
	msadsc_t *retmsa = mm_divpages_fmwk(mmobjp, pages, &retpnr, mrtype, flgs);
	if (NULL == retmsa) {
		*retrealpnr = 0;
		return NULL;
	}
	
    *retrealpnr = retpnr;
	return retmsa;
}



memarea_t *retn_procmarea(memmgrob_t *mmobjp)
{
	if (NULL == mmobjp) {
		return NULL;
	}
	for (uint_t mi = 0; mi < mmobjp->mo_mareanr; mi++)
	{
		if (MA_TYPE_PROC == mmobjp->mo_mareastat[mi].ma_type)
		{
			return &mmobjp->mo_mareastat[mi];
		}
	}
	return NULL;
}

msadsc_t *divpages_procmarea_core(memmgrob_t *mmobjp, uint_t pages, uint_t *retrealpnr)
{
	cpuflg_t cpuflg;
	uint_t retpnr = 0;						  
	msadsc_t *retmsa = NULL, *retmsap = NULL; 
	if (NULL == mmobjp || 1 != pages || NULL == retrealpnr) {
		return NULL;
	}
	memarea_t *marp = retn_procmarea(mmobjp);
	if (NULL == marp) {
		*retrealpnr = 0;
		return NULL;
	}
	//knl_spinlock_cli(&marp->ma_lock, &cpuflg);
	if (scan_mapgsalloc_ok(marp, pages) == FALSE) {
		retmsap = NULL;
		retpnr = 0;
		goto ret_step;
	}
	retmsa = mm_prcdivpages_onmarea(marp, pages, &retpnr);

	if (NULL != retmsa && 0 != retpnr) {
		mm_update_memarea(marp, retpnr, 0);
		mm_update_memmgrob(retpnr, 0);
		retmsap = retmsa;
		goto ret_step;
	}
	retpnr = 0;
	retmsap = NULL;
ret_step:
	//knl_spinunlock_sti(&marp->ma_lock, &cpuflg);
	*retrealpnr = retpnr;
	return retmsap;
}

msadsc_t *mm_divpages_procmarea(memmgrob_t *mmobjp, uint_t pages, uint_t *retrealpnr)
{
	msadsc_t *retmsa = NULL;
	uint_t retpnr = 0;
	if (NULL == mmobjp || 1 != pages || NULL == retrealpnr)
	{
		return NULL;
	}
	retmsa = divpages_procmarea_core(mmobjp, pages, &retpnr);
	if (NULL != retmsa)
	{
		*retrealpnr = retpnr;
		return retmsa;
	}
	retmsa = mm_division_pages(mmobjp, pages, &retpnr, MA_TYPE_KRNL, DMF_RELDIV);
	if (NULL == retmsa)
	{
		*retrealpnr = 0;
		return NULL;
	}
	*retrealpnr = retpnr;
	return retmsa;
}

bool_t scan_freemsa_isok(msadsc_t *freemsa, uint_t freepgs)
{
	if (NULL == freemsa || 1 > freepgs) {
		return FALSE;
	}
	if (MF_OLKTY_ODER != freemsa->md_indxflgs.mf_olkty ||
		NULL == freemsa->md_odlink || 1 > freemsa->md_indxflgs.mf_uindx) {
		return FALSE;
	}
	msadsc_t *end = (msadsc_t *)freemsa->md_odlink;

	if (PAF_ALLOC != freemsa->md_phyadrs.paf_alloc ||
		PAF_ALLOC != end->md_phyadrs.paf_alloc ||
		1 > end->md_indxflgs.mf_uindx) {
		return FALSE;
	}
	if (((uint_t)((end - freemsa) + 1)) != freepgs) {
		return FALSE;
	}
	return TRUE;
}

sint_t mm_cmsa1blk_isok(bafhlst_t *bafh, msadsc_t *_1ms, msadsc_t *_1me)
{
	if (NULL == bafh || NULL == _1ms || NULL == _1me) {
		return 0;
	}
	if (_1me < _1ms) {
		return 0;
	}
	if (_1ms == _1me)
	{
		if (MF_OLKTY_BAFH != _1me->md_indxflgs.mf_olkty)
		{
			return 0;
		}
		if (bafh != (bafhlst_t *)_1me->md_odlink)
		{
			return 0;
		}
		if (PAF_NO_ALLOC != _1me->md_phyadrs.paf_alloc)
		{
			return 0;
		}
		if (0 != _1me->md_indxflgs.mf_uindx)
		{
			return 0;
		}
		if ((_1me->md_phyadrs.paf_padrs - _1ms->md_phyadrs.paf_padrs) != (uint_t)(_1me - _1ms))
		{
			return 0;
		}
		return 2;
	}

	if (MF_OLKTY_ODER != _1ms->md_indxflgs.mf_olkty)
	{
		return 0;
	}
	if (_1me != (msadsc_t *)_1ms->md_odlink)
	{
		return 0;
	}
	if (PAF_NO_ALLOC != _1ms->md_phyadrs.paf_alloc)
	{
		return 0;
	}
	if (0 != _1ms->md_indxflgs.mf_uindx)
	{
		return 0;
	}

	if (MF_OLKTY_BAFH != _1me->md_indxflgs.mf_olkty)
	{
		return 0;
	}
	if (bafh != (bafhlst_t *)_1me->md_odlink)
	{
		return 0;
	}
	if (PAF_NO_ALLOC != _1me->md_phyadrs.paf_alloc)
	{
		return 0;
	}
	if (0 != _1me->md_indxflgs.mf_uindx)
	{
		return 0;
	}
	if ((_1me->md_phyadrs.paf_padrs - _1ms->md_phyadrs.paf_padrs) != (uint_t)(_1me - _1ms))
	{
		return 0;
	}
	return 2;
}

sint_t mm_cmsa2blk_isok(bafhlst_t *bafh, msadsc_t *_1ms, msadsc_t *_1me, msadsc_t *_2ms, msadsc_t *_2me)
{
	if (NULL == bafh || NULL == _1ms || NULL == _1me ||
		NULL == _2ms || NULL == _2me || _1ms == _2ms || _1me == _2me) {
		return 0;
	}
	sint_t ret1s = 0, ret2s = 0;
	
	ret1s = mm_cmsa1blk_isok(bafh, _1ms, _1me);
	if (0 == ret1s) {
		system_error("mm_cmsa1blk_isok ret1s == 0\n");
	}
	
	ret2s = mm_cmsa1blk_isok(bafh, _2ms, _2me);
	if (0 == ret2s) {
		system_error("mm_cmsa1blk_isok ret2s == 0\n");
	}

	if (2 == ret1s && 2 == ret2s)
	{
		if (_1ms < _2ms && _1me < _2me)
		{
			if ((_1me + 1) != _2ms)
			{
				return 1;
			}
			if ((_1me->md_phyadrs.paf_padrs + 1) != _2ms->md_phyadrs.paf_padrs)
			{
				return 1;
			}
			return 2;
		}
		if (_1ms > _2ms && _1me > _2me)
		{
			if ((_2me + 1) != _1ms)
			{
				return 1;
			}
			if ((_2me->md_phyadrs.paf_padrs + 1) != _1ms->md_phyadrs.paf_padrs)
			{
				return 1;
			}
			return 4;
		}
		return 0;
	}
	return 0;
}

bool_t chek_cl2molkflg(bafhlst_t *bafh, msadsc_t *_1ms, msadsc_t *_1me, msadsc_t *_2ms, msadsc_t *_2me)
{
	if (NULL == bafh || NULL == _1ms || NULL == _1me || NULL == _2ms || NULL == _2me)
	{
		return FALSE;
	}
	if (_1ms == _2ms || _1me == _2me)
	{
		return FALSE;
	}
	if (((uint_t)(_2me - _1ms) + 1) != bafh->af_oderpnr)
	{
		return FALSE;
	}
	if (_1ms == _1me && _2ms == _2me)
	{
		if (MF_OLKTY_ODER != _1ms->md_indxflgs.mf_olkty || (msadsc_t *)_1ms->md_odlink != _2me)
		{
			return FALSE;
		}
		if (MF_OLKTY_BAFH != _2me->md_indxflgs.mf_olkty || (bafhlst_t *)_2me->md_odlink != bafh)
		{
			return FALSE;
		}
		return TRUE;
	}

	if (MF_OLKTY_ODER != _1ms->md_indxflgs.mf_olkty || (msadsc_t *)_1ms->md_odlink != _2me)
	{
		return FALSE;
	}
	if (MF_OLKTY_INIT != _1me->md_indxflgs.mf_olkty || NULL != _1me->md_odlink)
	{
		return FALSE;
	}
	if (MF_OLKTY_INIT != _2ms->md_indxflgs.mf_olkty || NULL != _2ms->md_odlink)
	{
		return FALSE;
	}
	if (MF_OLKTY_BAFH != _2me->md_indxflgs.mf_olkty || (bafhlst_t *)_2me->md_odlink != bafh)
	{
		return FALSE;
	}
	return TRUE;
}

// 合并物理页面操作，并且对合并的页面初始化
bool_t mm_clear_2msaolflg(bafhlst_t *bafh, msadsc_t *_1ms, msadsc_t *_1me, msadsc_t *_2ms, msadsc_t *_2me)
{
	if (NULL == bafh || NULL == _1ms || NULL == _1me || NULL == _2ms || NULL == _2me) {
		return FALSE;
	}
	if (_1ms == _2ms || _1me == _2me) {
		return FALSE;
	}

	_1me->md_indxflgs.mf_olkty = MF_OLKTY_INIT;
	_1me->md_odlink = NULL;
	_2ms->md_indxflgs.mf_olkty = MF_OLKTY_INIT;
	_2ms->md_odlink = NULL;
	_1ms->md_indxflgs.mf_olkty = MF_OLKTY_ODER;
	_1ms->md_odlink = _2me;
	_2me->md_indxflgs.mf_olkty = MF_OLKTY_BAFH;
	_2me->md_odlink = bafh;
	return TRUE;
}

// F1 查看最大地址连续、且空闲msadsc_t结构，如释放的是第0个msadsc_t结构我们就去查找第1个msadsc_t结构是否空闲，且与第0个msadsc_t结构的地址是不是连续的
// 2: 找到连续页面 1：没找到连续页面 0：不可接收错误
sint_t mm_find_cmsa2blk(bafhlst_t *fbafh, msadsc_t **rfnms, msadsc_t **rfnme)
{
	if (NULL == fbafh || NULL == rfnms || NULL == rfnme) {
		return 0;
	}
	msadsc_t *freemstat = *rfnms;
	msadsc_t *freemend = *rfnme;
	if (1 > fbafh->af_fobjnr) {
		return 1;
	}
	
	list_h_t *tmplst = NULL;
	msadsc_t *tmpmsa = NULL, *blkms = NULL, *blkme = NULL;
	sint_t rets = 0;
	list_for_each(tmplst, &fbafh->af_frelst)
	{
		tmpmsa = list_entry(tmplst, msadsc_t, md_list);
		rets = mm_cmsa2blk_isok(fbafh, freemstat, freemend, tmpmsa, &tmpmsa[fbafh->af_oderpnr - 1]);
		if (2 == rets || 4 == rets)
		{
			blkms = tmpmsa;
			blkme = &tmpmsa[fbafh->af_oderpnr - 1];
			list_del(&tmpmsa->md_list); // ?
			fbafh->af_fobjnr--;
			fbafh->af_mobjnr--;
			goto step1;
		}
	}
step1:
	if (0 == rets || 1 == rets)
	{
		return 1;
	}
	if (2 == rets)
	{
		if (mm_clear_2msaolflg(fbafh + 1, freemstat, freemend, blkms, blkme) == TRUE)
		{
			if (chek_cl2molkflg(fbafh + 1, freemstat, freemend, blkms, blkme) == FALSE)
			{
				system_error("chek_cl2molkflg err1\n");
			}
			*rfnms = freemstat;
			*rfnme = blkme;
			return 2;
		}
		return 0;
	}
	if (4 == rets)
	{
		if (mm_clear_2msaolflg(fbafh + 1, blkms, blkme, freemstat, freemend) == TRUE)
		{
			if (chek_cl2molkflg(fbafh + 1, blkms, blkme, freemstat, freemend) == FALSE)
			{
				system_error("chek_cl2molkflg err2\n");
			}
			*rfnms = blkms;
			*rfnme = freemend;
			return 2;
		}

		return 0;
	}
	return 0;
}

// F2 把合并的msadsc_t结构（从mnxs到mnxe）加入到对应的bafhlst_t结构中
bool_t mpobf_add_msadsc(bafhlst_t *bafhp, msadsc_t *freemstat, msadsc_t *freemend)
{
	if (NULL == bafhp || NULL == freemstat || NULL == freemend)
	{
		return FALSE;
	}
	if (freemend < freemstat)
	{
		return FALSE;
	}
	if (bafhp->af_oderpnr != ((uint_t)(freemend - freemstat) + 1))
	{
		return FALSE;
	}
	if ((~0UL) <= bafhp->af_fobjnr || (~0UL) <= bafhp->af_mobjnr)
	{
		system_error("(~0UL)<=bafhp->af_fobjnr\n");
		return FALSE;
	}
	freemstat->md_indxflgs.mf_olkty = MF_OLKTY_ODER;
	freemstat->md_odlink = freemend;
	freemend->md_indxflgs.mf_olkty = MF_OLKTY_BAFH;
	freemend->md_odlink = bafhp;
	
	list_add_to_behind(&bafhp->af_frelst, &freemstat->md_list);
	bafhp->af_fobjnr++;
	bafhp->af_mobjnr++;
	return TRUE;
}

// E-3 把msadsc_t结构进行合并然后加入对应bafhlst_t结构
bool_t mm_merpages_onbafhlst(msadsc_t *freemsa, uint_t freepgs, bafhlst_t *relbf, bafhlst_t *merbf)
{
	sint_t rets = 0;
	msadsc_t *mnxs = freemsa, *mnxe = &freemsa[freepgs - 1];
	bafhlst_t *tmpbf = relbf;
	for (; tmpbf < merbf; tmpbf++)
	{
		// 查看最大地址连续、且空闲msadsc_t结构(bafhlst_t节点)，
		// 若是找到会把待释放节点 和空闲节点 向上融合成一个大节点。存储在mnxstart,mnxend中
		// 不断的向上融合，直到无法融合或者有故障发生。
		rets = mm_find_cmsa2blk(tmpbf, &mnxs, &mnxe);
		if (1 == rets) {
			break;
		}
		if (0 == rets) {
			system_error("mm_find_cmsa2blk retn 0\n");
		}
	}

	// 把大节点放入 bafhlst_t 链表中
	if (mpobf_add_msadsc(tmpbf, mnxs, mnxe) == FALSE)
	{
		return FALSE;
	}
	return TRUE;
}

// D_1 在内存区上合并页
bool_t mm_merpages_onmarea(memarea_t *malckp, msadsc_t *freemsa, uint_t freepgs)
{
	if (NULL == malckp || NULL == freemsa || 1 > freepgs) {
		return FALSE;
	}

	bafhlst_t *prcbf = NULL;
	sint_t pocs = 0;
	if (MA_TYPE_PROC == malckp->ma_type) {

		prcbf = &malckp->ma_mdmdata.dm_onemsalst;
		pocs = mm_merpages_opmsadsc(prcbf, freemsa, freepgs);
		if (2 == pocs)
		{
			if (mpobf_add_msadsc(prcbf, freemsa, &freemsa[freepgs - 1]) == FALSE)
			{
				system_error("mm_merpages_onmarea proc memarea merge fail\n");
			}
			mm_update_memarea(malckp, freepgs, 1);
			mm_update_memmgrob(freepgs, 1);
			return TRUE;
		}
		if (1 == pocs)
		{
			return TRUE;
		}
		if (0 == pocs)
		{
			return FALSE;
		}
		return FALSE;
	}

	bafhlst_t *retrelbf = NULL, *retmerbf = NULL;
	bool_t rets = FALSE;
	
	//E-1 根据freepgs返回请求释放的和最大释放的bafhlst_t结构指针
	rets = onfpgs_retn_bafhlst(malckp, freepgs, &retrelbf, &retmerbf);
	if (FALSE == rets) {
		return FALSE;
	}
	if (NULL == retrelbf || NULL == retmerbf) {
		return FALSE;
	}
	//E-2 设置msadsc_t结构的信息，完成释放，返回1表示不需要下一步合并操作，返回2表示要进行合并操作
	sint_t mopms = mm_merpages_opmsadsc(retrelbf, freemsa, freepgs);
	if (2 == mopms)
	{
		// E-3 把msadsc_t结构进行合并然后加入对应bafhlst_t结构, (核心操作)
		rets = mm_merpages_onbafhlst(freemsa, freepgs, retrelbf, retmerbf);
		if (TRUE == rets)
		{
			mm_update_memarea(malckp, freepgs, 1);
			mm_update_memmgrob(freepgs, 1);
			return rets;
		}
		return FALSE;
	}
	if (1 == mopms) {
		return TRUE;
	}
	if (0 == mopms) {
		return FALSE;
	}
	return FALSE;
}


// C_2 释放内存页面的核心函数
bool_t mm_merpages_core(memarea_t *marea, msadsc_t *freemsa, uint_t freepgs)
{
	if (NULL == marea || NULL == freemsa || 1 > freepgs) {
		return FALSE;
	}
	if (scan_freemsa_isok(freemsa, freepgs) == FALSE) {
		return FALSE;
	}
	bool_t rets = FALSE;
	cpuflg_t cpuflg;

	// knl_spinlock_cli(&marea->ma_lock, &cpuflg);
	// D_1 针对内存区进行操作
	rets = mm_merpages_onmarea(marea, freemsa, freepgs);
	// knl_spinunlock_sti(&marea->ma_lock, &cpuflg);
	return rets;
}

//B_释放页内存
bool_t mm_merpages_fmwk(memmgrob_t *mmobjp, msadsc_t *freemsa, uint_t freepgs)
{
	// C_1 获取释放msadsc_t结构所在的内存区
	memarea_t *marea = onfrmsa_retn_marea(mmobjp, freemsa, freepgs);
	if (NULL == marea) {
		return FALSE;
	}

	// C_2 释放内存页面的核心函数
	bool_t rets = mm_merpages_core(marea, freemsa, freepgs);
	if (FALSE == rets) {
		return FALSE;
	}
	return rets;
}


// A_释放页内存, 释放内存页面接口
//mmobjp->内存管理数据结构指针
//freemsa->释放内存页面对应的首个msadsc_t结构指针
//freepgs->请求释放的内存页面数
bool_t mm_merge_pages(memmgrob_t *mmobjp, msadsc_t *freemsa, uint_t freepgs)
{
	if (NULL == mmobjp || NULL == freemsa || 1 > freepgs) {
		return FALSE;
	}

	bool_t rets = mm_merpages_fmwk(mmobjp, freemsa, freepgs);
	if (FALSE == rets) {
		return FALSE;
	}
	return rets;
}








