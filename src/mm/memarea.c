#include "mmkit.h"

extern mmgro_t glomm;

void arclst_t_init(arclst_t *initp)
{
    list_init(&initp->al_lru1);
    list_init(&initp->al_lru2);
    initp->al_lru1nr = 0;
    initp->al_lru2nr = 0;
    return;
}

static mmstus_t mafo_deft_init(struct s_MEMAREA *memarea, void *valp, uint_t val)
{
    return MMSTUS_ERR;
}

static mmstus_t mafo_deft_exit(struct s_MEMAREA *memarea)
{
    return MMSTUS_ERR;
}

static mmstus_t mafo_deft_afry(struct s_MEMAREA *memarea, mmafrets_t *mafrspack, void *valp, uint_t val)
{
    return MMSTUS_ERR;
}

void mafuncobjs_t_init(mafuncobjs_t *initp)
{
    initp->mafo_init = mafo_deft_init;
    initp->mafo_exit = mafo_deft_exit;
    initp->mafo_aloc = mafo_deft_afry;
    initp->mafo_free = mafo_deft_afry;
    initp->mafo_recy = mafo_deft_afry;
    return;
}

/**
 * @brief 初始化内存块分配释放头链表
 * 
 * @param initp 
 * @param stus 分割 / 单一分配
 * @param oder 物理页数的以2为底的指数
 * @param oderpnr 一个内存块占用的 内存空间地址描述符的数量 / 占用的物理页数
 */
void bafhlst_t_init(bafhlst_t *initp, u32_t stus, uint_t oder, uint_t oderpnr)
{
    spin_init(&initp->af_lock);
    initp->af_stus = stus;
    initp->af_oder = oder;
    initp->af_oderpnr = oderpnr;
    initp->af_fmsanr = 0;
    initp->af_amsanr = 0;
    initp->af_alccnt = 0;
    initp->af_frecnt = 0;
    list_init(&initp->af_frelst);
    list_init(&initp->af_alclst);
    list_init(&initp->af_ovelst);
    return;
}

/**
 * @brief 初始化内存管理分割合并结构体
 * 
 * @param initp 
 */
void memdivmer_t_init(memdivmer_t *initp)
{
    //初始化medivmer_t结构体的基本数据
    spin_init(&initp->dm_lock);

    initp->dm_dmmaxindx = 0;
    initp->dm_phydmindx = 0;
    initp->dm_predmindx = 0;

    initp->dm_stus = 0;
    initp->dm_divnr = 0;
    initp->dm_mernr = 0;
    //循环初始化memdivmer_t结构体中dm_mdmlielst数组中的每个bafhlst_t结构的基本数据
    for (uint_t li = 0; li < MDIVMER_ARR_LMAX; li++)
    {
        bafhlst_t_init(&initp->dm_pools[li], BAFH_STUS_DIVM, li, (1UL << li));
    }
    bafhlst_t_init(&initp->dm_onepool, BAFH_STUS_ONEM, 0, 1UL);
    return;
}

/**
 * @brief 初始化内存区
 * 
 * @param initp 
 */
void memarea_t_init(memarea_t *initp)
{
    //初始化memarea_t结构体的基本数据
    list_init(&initp->ma_list);
    spin_init(&initp->ma_lock);
	//knl_sem_init(&initp->ma_sem,SEM_MUTEX_ONE_LOCK,SEM_FLG_MUTEX);
	//init_wait_l_head(&initp->ma_waitlst,general_wait_wake_up);
    initp->ma_stus = 0;
    initp->ma_flgs = 0;
    initp->ma_type = MA_TYPE_INIT;
    initp->ma_maxpages = 0;
    initp->ma_allocpages = 0;
    initp->ma_freepages = 0;
    initp->ma_resvpages = 0;
    initp->ma_horizline = 0;
    initp->ma_logicstart = 0;
    initp->ma_logicend = 0;
    initp->ma_logicsz = 0;
    initp->ma_effectstart = 0;
    initp->ma_effectend = 0;
    initp->ma_effectsz = 0;
    list_init(&initp->ma_allmsadsclst);
    initp->ma_allmsadscnr = 0;
    arclst_t_init(&initp->ma_arcpglst);
    mafuncobjs_t_init(&initp->ma_funcobj);
    //初始化memarea_t结构体中的memdivmer_t结构体
    memdivmer_t_init(&initp->ma_mdmdata);
    initp->ma_privp = nullptr;
    return;
}

bool_t init_memarea_core()
{
    //获取memarea_t结构开始地址, memearea坐落在4K边界上，占用一整个页面
    memarea_t *virmarea = (memarea_t *)PAGE_4K_ALIGN(memory_management_struct.end_of_struct);
    for (uint_t mai = 0; mai < MEMAREA_MAX; mai++)
    {   //循环初始化每个memarea_t结构实例变量
        memarea_t_init(&virmarea[mai]);
    }
    //设置硬件区的类型和空间大小
    virmarea[0].ma_type = MA_TYPE_HWAD;
    virmarea[0].ma_logicstart = MA_HWAD_LSTART;
    virmarea[0].ma_logicend = MA_HWAD_LEND;
    virmarea[0].ma_logicsz = MA_HWAD_LSZ;
    //设置内核区的类型和空间大小
    virmarea[1].ma_type = MA_TYPE_KRNL;
    virmarea[1].ma_logicstart = MA_KRNL_LSTART;
    virmarea[1].ma_logicend = MA_KRNL_LEND;
    virmarea[1].ma_logicsz = MA_KRNL_LSZ;
    //设置应用区的类型和空间大小
    virmarea[2].ma_type = MA_TYPE_PROC;
    virmarea[2].ma_logicstart = MA_PROC_LSTART;
    virmarea[2].ma_logicend = MA_PROC_LEND;
    virmarea[2].ma_logicsz = MA_PROC_LSZ;
    //将memarea_t结构的开始的物理地址写入kmachbsp结构中 
    glomm.mo_mareastat = virmarea;
    //将memarea_t结构的个数写入kmachbsp结构中 
    glomm.mo_mareanr = 3;
    //将所有memarea_t结构的大小写入kmachbsp结构中 
    //计算下一个空闲内存的开始地址
    memory_management_struct.end_of_struct += PAGE_4K_ALIGN(sizeof(memarea_t) * MEMAREA_MAX);
    return TRUE;
}

/* 初始化内存区 */
void init_memarea()
{
    /* 真正初始化内存区 */
    if (init_memarea_core() == FALSE)
    {
        color_printk(RED, BLACK, "init_memarea_core fail");
    }
    return;
}


/**
 * @brief  判断两个msadsc是否连续
 * 
 * @param prevmsa  第一个masdsc_t结构体
 * @param nextmsa  第二个masdsc_t结构体
 * @param cmpmdfp  期待的下一个(nextmsa)内存空间描述符的标志
 * @return uint_t ~0: 严重错误  1：判断为不连续的 内存空间地址描述符 2：判断为连续
 */
uint_t continumsadsc_is_ok(msadsc_t *prevmsa, msadsc_t *nextmsa, msadflgs_t *cmpmdfp)
{
    if (nullptr == prevmsa || nullptr == cmpmdfp)
    {
        return (~0UL);
    }

    if (nullptr != prevmsa && nullptr != nextmsa)
    {
        if (prevmsa->md_cntflgs.mf_marty == cmpmdfp->mf_marty &&
            0 == prevmsa->md_cntflgs.mf_refcnt &&
            MF_MOCTY_FREE == prevmsa->md_cntflgs.mf_mocty &&
            PAF_NO_ALLOC == prevmsa->md_phyadrs.paf_alloc)
        {
            if (nextmsa->md_cntflgs.mf_marty == cmpmdfp->mf_marty && /* 是否属于同一个内存区 */
                0 == nextmsa->md_cntflgs.mf_refcnt &&
                MF_MOCTY_FREE == nextmsa->md_cntflgs.mf_mocty &&
                PAF_NO_ALLOC == nextmsa->md_phyadrs.paf_alloc)
            {
                if ((nextmsa->md_phyadrs.paf_padrs << PSHRSIZE) - (prevmsa->md_phyadrs.paf_padrs << PSHRSIZE) == PAGESIZE) // 内存释放连续
                {
                    return 2;
                }
                return 1;
            }
            return 1;
        }
        return 0;
    }

    return (~0UL);
}

/**
 * @brief  从mstat开始，扫描最长的连续的内存空间地址描述符
 * 
 * @param mstat 起始的 msadsc_t 结构指针
 * @param cmpmdfp 内存区标志
 * @param mnr  最大 msdsc_t 数组结构索引
 * @param retmnr [out] 找到的连续个数
 * @return bool_t 
 */
bool_t scan_len_msadsc(msadsc_t *mstat, msadflgs_t *cmpmdfp, uint_t mnr, uint_t *retmnr)
{
    uint_t retclok = 0;
    uint_t retnr = 0;

    if (nullptr == mstat || nullptr == cmpmdfp || 0 == mnr || nullptr == retmnr) 
    {
        return FALSE;
    }
    for (uint_t tmdx = 0; tmdx < mnr - 1; tmdx++) 
    {
        retclok = continumsadsc_is_ok(&mstat[tmdx], &mstat[tmdx + 1], cmpmdfp);
        if ((~0UL) == retclok)
        {
            *retmnr = 0;
            return FALSE;
        }
        if (0 == retclok) 
        {
            *retmnr = 0;
            return FALSE;
        }
        if (1 == retclok)
        {
            *retmnr = retnr;
            return TRUE;
        }
        retnr++;
    }

    *retmnr = retnr;
    return TRUE;
}

/**
 * @brief 获取最多且地址连续的msadsc_t结构体
 * 
 * @param mareap 某个内存区
 * @param fmstat  msadsc数组头
 * @param fntmsanr [out]本轮开始的msadsc数组索引
 * @param fmsanr 总共的 msadsc 的个数，数组长度
 * @param retmsastatp [out] 当前连续地址 msadsc 开始地址
 * @param retmsaendp [out] 当前连续地址 msadsc 结束地址 
 * @param retfmnr [out] 本轮有多少给地址连续的msadsc_t
 * @return bool_t 
 */
bool_t merlove_scan_continumsadsc(memarea_t *mareap, msadsc_t *fmstat, uint_t *fntmsanr, uint_t fmsanr,
                                msadsc_t **retmsastatp, msadsc_t **retmsaendp, uint_t *retfmnr)
{
    if (nullptr == mareap || nullptr == fmstat || nullptr == fntmsanr ||
        0 == fmsanr || nullptr == retmsastatp || nullptr == retmsaendp || nullptr == retfmnr) {
        return FALSE;
    }
    if (*fntmsanr >= fmsanr) {
        return FALSE;
    }

    u64_t muindx = 0;
    /* 获取内存区类型 */
    msadflgs_t *mdfp = nullptr;
    switch (mareap->ma_type) {
        case MA_TYPE_HWAD:
            muindx = MF_MARTY_HWD << 5;
            mdfp = (msadflgs_t *)(&muindx);
            break;
        case MA_TYPE_KRNL:
            muindx = MF_MARTY_KRL << 5;
            mdfp = (msadflgs_t *)(&muindx);
            break;
        case MA_TYPE_PROC:
            muindx = MF_MARTY_PRC << 5;
            mdfp = (msadflgs_t *)(&muindx);
            break;
        default:
            muindx = 0;
            mdfp = nullptr;
            break;
    }

    if (0 == muindx || nullptr == mdfp) {
        return FALSE;
    }

    msadsc_t *msastat = fmstat;
    uint_t retfindmnr = 0; // 找到的连续页面数
    bool_t rets = FALSE;
    uint_t tmidx = *fntmsanr;
    for (; tmidx < fmsanr; tmidx++)
    {
        if (msastat[tmidx].md_cntflgs.mf_marty == mdfp->mf_marty && // 保证该内存页属于该内存区
            0 == msastat[tmidx].md_cntflgs.mf_refcnt &&				// 索引数为0
            MF_MOCTY_FREE == msastat[tmidx].md_cntflgs.mf_mocty &&  // 空闲
            PAF_NO_ALLOC == msastat[tmidx].md_phyadrs.paf_alloc)	// 没有被分配
        {
            rets = scan_len_msadsc(&msastat[tmidx], mdfp, fmsanr, &retfindmnr);
            if (FALSE == rets) {
                color_printk(RED, BLACK,"scan_len_msadsc err\n");
            }
            *fntmsanr = tmidx + retfindmnr + 1;         /* 下一轮开始的page结构索引 */
            *retmsastatp = &msastat[tmidx];             /* 当前连续地址page的开始地址 */
            *retmsaendp = &msastat[tmidx + retfindmnr]; /* 当前连续地址page的结束地址 */
            *retfmnr = retfindmnr + 1;                  /* 本轮有多少个地址连续的msadsc_t */

            return TRUE;
        }
    }
    if (tmidx >= fmsanr)
    {
        *fntmsanr = fmsanr;
        *retmsastatp = nullptr;
        *retmsaendp = nullptr;
        *retfmnr = 0;
        return TRUE;
    }
    return FALSE;
}

/**
 * @brief 给内存区memarea_t 所囊括的物理页面结构体赋予内存区标识
 * 
 * @param mareap 内存区指针
 * @param mstat 内存页头地址
 * @param msanr 
 * @return uint_t 
 */
uint_t merlove_setallmarflgs_onmemarea(memarea_t *mareap, msadsc_t *mstat, uint_t msanr)
{
	if (nullptr == mareap || nullptr == mstat || 0 == msanr) {
		return ~0UL;
	}
	u64_t muindx = 0;
	msadflgs_t *mdfp = nullptr;
	// 获取内存区类型
	switch (mareap->ma_type) {
		case MA_TYPE_HWAD:
		    muindx = MF_MARTY_HWD << 5;		// 硬件区标签
		    mdfp = (msadflgs_t *)(&muindx);
			break;
		case MA_TYPE_KRNL:
		    muindx = MF_MARTY_KRL << 5;		// 内存区标签
		    mdfp = (msadflgs_t *)(&muindx);
			break;
		case MA_TYPE_PROC:
		    muindx = MF_MARTY_PRC << 5;		// 应用区标签
		    mdfp = (msadflgs_t *)(&muindx);
			break;
		case MA_TYPE_SHAR:
			return 0;
        case MA_TYPE_INIT:
            return 0;
		default:	
			muindx = 0;
			mdfp = nullptr;
			break;
	}
	
	if (0 == muindx || nullptr == mdfp) {
		return ~0UL;
	}
	u64_t phyadr = 0;
	uint_t retnr = 0, mix = 0;
	for (; mix < msanr; mix++)
	{
		if (MF_MARTY_INIT == mstat[mix].md_cntflgs.mf_marty)
		{	
			//获取msadsc_t结构对应的地址
			phyadr = mstat[mix].md_phyadrs.paf_padrs << PSHRSIZE;
			//和内存区的地址区间比较
			if (phyadr >= mareap->ma_logicstart && ((phyadr + PAGESIZE) - 1) <= mareap->ma_logicend)
			{
				//设置msadsc_t结构所属内存区
				mstat[mix].md_cntflgs.mf_marty = mdfp->mf_marty;
				retnr++;
			}
		}
	}
	return retnr;
}

/**
 * @brief 
 * 
 * @param mareap 内存区
 * @param bafhp 挂载链表结构体
 * @param fstat 被挂载的page集合首部
 * @param fend  集合尾部
 * @param fmnr  集合个数
 * @return bool_t 
 */
bool_t continumsadsc_add_bafhlst(memarea_t *mareap, bafhlst_t *bafhp, msadsc_t *fstat, msadsc_t *fend, uint_t fmnr)
{
	if (nullptr == mareap || nullptr == bafhp || nullptr == fstat || nullptr == fend || 0 == fmnr) {
		return FALSE;
	}
	if (bafhp->af_oderpnr != fmnr) {
		return FALSE;
	}
	if ((&fstat[fmnr - 1]) != fend) {
		return FALSE;
	}

	fstat->md_cntflgs.mf_olkty = MF_OLKTY_ODER; // 首
	//开始的msadsc_t结构指向最后的msadsc_t结构
	fstat->md_odlink = fend;
	// fstat==fend
	fend->md_cntflgs.mf_olkty = MF_OLKTY_BAFH; // 尾
	//最后的msadsc_t结构指向它属于的bafhlst_t结构
	fend->md_odlink = bafhp;
	
	list_add_to_behind(&bafhp->af_frelst, &fstat->md_list);
	bafhp->af_fmsanr++;
	bafhp->af_amsanr++;
	
    mareap->ma_maxpages += fmnr;
	mareap->ma_freepages += fmnr;
	mareap->ma_allmsadscnr += fmnr;
	
    return TRUE;
}

bafhlst_t *find_continumsa_inbafhlst(memarea_t *mareap, uint_t fmnr)
{
    bafhlst_t *retbafhp = nullptr;
    uint_t in = 0;
    if (nullptr == mareap || 0 == fmnr) {
        return nullptr;
    }

    if (MA_TYPE_PROC == mareap->ma_type) {
        return &mareap->ma_mdmdata.dm_onepool;
    }
    if (MA_TYPE_SHAR == mareap->ma_type) {
        return nullptr;
    }

    in = 0;
    retbafhp = nullptr;
    for (uint_t li = 0; li < MDIVMER_ARR_LMAX; li++) // 这里可以用二分算法
    {
        if ((mareap->ma_mdmdata.dm_pools[li].af_oderpnr) <= fmnr) {
            retbafhp = &mareap->ma_mdmdata.dm_pools[li];
            in++;
        }
    }

    if (MDIVMER_ARR_LMAX <= in || nullptr == retbafhp) {
        return nullptr;
    }

    return retbafhp;
}

bool_t continumsadsc_add_procmareabafh(memarea_t *mareap, bafhlst_t *bafhp, msadsc_t *fstat, msadsc_t *fend, uint_t fmnr)
{
	if (nullptr == mareap || nullptr == bafhp || nullptr == fstat || nullptr == fend || 0 == fmnr) {
		return FALSE;
	}
	if (BAFH_STUS_ONEM != bafhp->af_stus || MA_TYPE_PROC != mareap->ma_type) {
		return FALSE;
	}
	if (bafhp->af_oderpnr != 1) {
		return FALSE;
	}
	if ((&fstat[fmnr - 1]) != fend) {
		return FALSE;
	}
	for (uint_t tmpnr = 0; tmpnr < fmnr; tmpnr++)
	{
		fstat[tmpnr].md_cntflgs.mf_olkty = MF_OLKTY_BAFH; // 尾巴 ？
		fstat[tmpnr].md_odlink = bafhp;
        
        list_add_to_behind(&bafhp->af_frelst, &fstat[tmpnr].md_list);
		bafhp->af_fmsanr++;
		bafhp->af_amsanr++;
		mareap->ma_maxpages++;
		mareap->ma_freepages++;
		mareap->ma_allmsadscnr++;
	}
	return TRUE;
}

/**
 *  @brief 从rfmnr中 分割出 【最大的2整数次幂的连续物理页】，挂载到bafhlst_t结构中
 *  
 * @param mareap 内存区
 * @param rfstat [out/in]内存页起始地址
 * @param rfend  [out/in]内存页终止地址
 * @param rfmnr  [out/in]内存页数(剩余的)
 * @return bool_t 
 */
bool_t continumsadsc_mareabafh_core(memarea_t *mareap, msadsc_t **rfstat, msadsc_t **rfend, uint_t *rfmnr)
{

    if (nullptr == mareap || nullptr == rfstat || nullptr == rfend || nullptr == rfmnr) {
        return FALSE;
    }
    uint_t retval = *rfmnr, tmpmnr = 0;
    msadsc_t *mstat = *rfstat, *mend = *rfend;
    if (1 > (retval)) {
        return FALSE;
    }

    // 根据地址连续的msadsc_t结构的数量查找合适bafhlst_t结构
    bafhlst_t *bafhp = find_continumsa_inbafhlst(mareap, retval);

    if (nullptr == bafhp) {
        return FALSE;
    }
    if (retval < bafhp->af_oderpnr) {
        return FALSE;
    }

    if ((BAFH_STUS_DIVP == bafhp->af_stus || BAFH_STUS_DIVM == bafhp->af_stus) && MA_TYPE_PROC != mareap->ma_type)
    {
        tmpmnr = retval - bafhp->af_oderpnr;
        // 把 页面们加入 到 bafhlst 结构之中
        if (continumsadsc_add_bafhlst(mareap, bafhp, mstat, &mstat[bafhp->af_oderpnr - 1], bafhp->af_oderpnr) == FALSE) {
            return FALSE;
        }
        INFOK("Kernel::on bafhlst[%d](%d) = %dMB, mounted memory space descriptor arange[%#lx - %#lx]/4KB",bafhp->af_oder,bafhp->af_oderpnr,(bafhp->af_oderpnr * PAGE_4K_SIZE / 1024 / 1024),
        mstat->md_phyadrs.paf_padrs, (mstat + bafhp->af_oderpnr - 1)->md_phyadrs.paf_padrs);
        // 如果地址连续的msadsc_t结构的数量正好是bafhp->af_oderpnr则完成，否则返回再次进入此函数
        if (tmpmnr == 0) {
            *rfmnr = tmpmnr;
            *rfend = nullptr;
            return TRUE;
        }

        // 挂载bafhp.af_oderpnr地址连续的msadsc_t结构到bafhlst_t中
        *rfstat = &mstat[bafhp->af_oderpnr];
        // 还剩多少个地址连续的msadsc_t结构
        *rfmnr = tmpmnr;

        return TRUE;
    }

    if (BAFH_STUS_ONEM == bafhp->af_stus && MA_TYPE_PROC == mareap->ma_type)
    { /* 专为用户池分配的 */
        if (continumsadsc_add_procmareabafh(mareap, bafhp, mstat, mend, *rfmnr) == FALSE)
        {
            return FALSE;
        }
        INFOK("User::on bafhlst[%d](%d), every one is 4KB, ALL:%d,Arange[%#lx - %#lx]/4KB",bafhp->af_oder,bafhp->af_oderpnr,bafhp->af_amsanr, mstat->md_phyadrs.paf_padrs, mend->md_phyadrs.paf_padrs);
        *rfmnr = 0;
        *rfend = nullptr;
        return TRUE;
    }

    return FALSE;
}

// 多次分割这段内存页, 直到将其全部挂载到 area -> divmerge_t -> bafhlst_t
bool_t merlove_continumsadsc_mareabafh(memarea_t *mareap, msadsc_t *mstat, msadsc_t *mend, uint_t mnr)
{
	if (nullptr == mareap || nullptr == mstat || nullptr == mend || 0 == mnr) {
		return FALSE;
	}
	uint_t mnridx = mnr;
	msadsc_t *fstat = mstat, *fend = mend;
	for (;(mnridx > 0 && nullptr != fend);)
	{
		if (continumsadsc_mareabafh_core(mareap, &fstat, &fend, &mnridx) == FALSE)
		{
			color_printk(RED, BLACK, "continumsadsc_mareabafh_core fail\n");
		}
	}
	return TRUE;
}


static uint_t check_continumsadsc(memarea_t *mareap, msadsc_t *stat, msadsc_t *end, uint_t fmnr)
{
    msadsc_t *ms = stat, *me = end;
    u64_t muindx = 0;
    msadflgs_t *mdfp = NULL;
    if (NULL == ms || NULL == me || 0 == fmnr || ms > me)
    {
        return 0;
    }
    switch (mareap->ma_type)
    {
        case MA_TYPE_HWAD:
        {
            muindx = MF_MARTY_HWD << 5;
            mdfp = (msadflgs_t *)(&muindx);
            break;
        }
        case MA_TYPE_KRNL:
        {
            muindx = MF_MARTY_KRL << 5;
            mdfp = (msadflgs_t *)(&muindx);
            break;
        }
        case MA_TYPE_PROC:
        {
            muindx = MF_MARTY_PRC << 5;
            mdfp = (msadflgs_t *)(&muindx);
            break;
        }
        default:
        {
            muindx = 0;
            mdfp = NULL;
            break;
        }
    }
    if (0 == muindx || NULL == mdfp)
    {
        return 0;
    }
    uint_t ok = 0;
    if (ms == me)
    {
        if (0 != ms->md_cntflgs.mf_refcnt)
        {
            return 0;
        }
        if (ms->md_cntflgs.mf_marty != mdfp->mf_marty)
        {
            return 0;
        }
        if (MF_MOCTY_FREE != ms->md_cntflgs.mf_mocty)
        {
            return 0;
        }
        if (PAF_NO_ALLOC != ms->md_phyadrs.paf_alloc)
        {
            return 0;
        }

        if ((ok + 1) != fmnr)
        {
            return 0;
        }
        return ok + 1;
    }
    for (; ms < me; ms++)
    {
        if (ms->md_cntflgs.mf_marty != mdfp->mf_marty ||
            (ms + 1)->md_cntflgs.mf_marty != mdfp->mf_marty)
        {
            return 0;
        }
        if (MF_MOCTY_FREE != ms->md_cntflgs.mf_mocty ||
            MF_MOCTY_FREE != (ms + 1)->md_cntflgs.mf_mocty)
        {
            return 0;
        }
        if (ms->md_cntflgs.mf_refcnt != 0 ||
            (ms + 1)->md_cntflgs.mf_refcnt != 0)
        {
            return 0;
        }
        if (PAF_NO_ALLOC != ms->md_phyadrs.paf_alloc ||
            PAF_NO_ALLOC != (ms + 1)->md_phyadrs.paf_alloc)
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
    if ((ok + 1) != fmnr)
    {
        return 0;
    }
    return ok;
}

/**
 * @brief 尽最大努力把连续的物理页块，挂载到对应的内存区结构中
 * 
 * @param mareap 内存区域指针
 * @param mstat 页面结构体数组头指针
 * @param msanr 系统页面结构体的数量
 * @return bool_t 
 */
bool_t merlove_mem_onmemarea(memarea_t *mareap, msadsc_t *mstat, uint_t msanr)
{
    if (nullptr == mareap || nullptr == mstat || 0 == msanr) {
        return FALSE;
    }
    if (MA_TYPE_SHAR == mareap->ma_type) {
        return TRUE;
    }
    if (MA_TYPE_INIT == mareap->ma_type) {
        return FALSE;
    }

    msadsc_t *retstatmsap = nullptr, *retendmsap = nullptr, *fntmsap = mstat; 
    /*  
        fntsd: first next msadsc page 下一次循环中第一个等待处理的内存页结构体
        retstatmsap:	当前连续地址page的开始地址 return start msadsc_t point
        retendmsap: 	当前连续地址page的结束地址 return end msadsc_t posint
        retfindmnr: 	当前有多少给连续地址的 page 结构 return finded msadsc_t Number
        fntmsanr:		本轮开始的page结构页地址
    */
    uint_t retfindmnr = 0;
    uint_t fntmnr = 0; // find next msadsc number
    bool_t retscan = FALSE;

    for (; fntmnr < msanr;)
    {	
        // 获取最多且地址连续的msadsc_t结构体的开始、结束地址、一共多少个msadsc_t结构体，下一次循环的 msadsc_t 数组索引号
        retscan = merlove_scan_continumsadsc(mareap, fntmsap, &fntmnr, msanr, &retstatmsap, &retendmsap, &retfindmnr);
        if (FALSE == retscan) {
            color_printk(RED, BLACK,"merlove_scan_continumsadsc fail\n");
        }
        if (nullptr != retstatmsap && nullptr != retendmsap)
        {
            if (check_continumsadsc(mareap, retstatmsap, retendmsap, retfindmnr) == 0) {
                color_printk(RED, BLACK, "check_continumsadsc fail\n");
            }
            
            // 把一组连续的msadsc_t结构体挂载到合适的m_mdmlielst数组中的bafhlst_t结构中
            if (merlove_continumsadsc_mareabafh(mareap, retstatmsap, retendmsap, retfindmnr) == FALSE) {
                color_printk(RED, BLACK, "merlove_continumsadsc_mareabafh fail\n");
            }
        }
    }
    return TRUE;
}

bool_t merlove_mem_core()
{
    // 获取msadsc_t结构的首地址和个数
    msadsc_t *mstatp = (msadsc_t *)glomm.mo_msadscstat;
    uint_t msanr = (uint_t)glomm.mo_msanr , maxp = 0;
    // 获得memarea_t结构的首地址
    memarea_t *marea = (memarea_t *)glomm.mo_mareastat;
    uint_t sretf = ~0UL;

    // 给每一个 msadsc_t 内存空间地址描述符, 添加内存区标志
    for (uint_t mi = 0; mi < (uint_t)glomm.mo_mareanr; mi++)
    {
        sretf = merlove_setallmarflgs_onmemarea(&marea[mi], mstatp, msanr);
        if ((~0UL) == sretf)
        {
            return FALSE;
        }
        #if 0
        uint_t tretf  = ~0UL;
        // tretf = test_setflgs(&marea[mi], mstatp, msanr);
        // if ((~0UL) == tretf)
        // {
        // 	return FALSE;
        // }
        // if (sretf != tretf)
        // {
        // 	return FALSE;
        // }
        #endif
    }

    // 把 物理页 挂载到 内存区 结构上
    for (uint_t maidx = 0; maidx < (uint_t)glomm.mo_mareanr; maidx++)
    {
        switch (marea[maidx].ma_type)
        {
            case MA_TYPE_HWAD:
                INFOK(" ============ Begin Hard area's pages mount=================");
                break;
            case MA_TYPE_KRNL:
                INFOK(" ============ Begin Kernel area's pages mount=================");
                break;
            case MA_TYPE_PROC:
                INFOK(" ============ Begin User area's pages mount================");
                break;
            default:
                break;
        }

        if (merlove_mem_onmemarea(&marea[maidx], mstatp, msanr) == FALSE)
        {
            return FALSE;
        }
        maxp += marea[maidx].ma_maxpages;

        INFOK(" ============ mounted %dMB ===============\n", (marea[maidx].ma_maxpages * PAGE_4K_SIZE)/1024/1024);
    }

    glomm.mo_freepages = maxp;
    glomm.mo_memsz = maxp * PAGE_4K_SIZE;

    return TRUE;
}

uint_t check_multi_msadsc(msadsc_t *mstat, bafhlst_t *bafhp, memarea_t *mareap)
{
    if (nullptr == mstat || nullptr == bafhp || nullptr == mareap)
    {
        return 0;
    }
    if (MF_OLKTY_ODER != mstat->md_cntflgs.mf_olkty &&
        MF_OLKTY_BAFH != mstat->md_cntflgs.mf_olkty)
    {
        return 0;
    }
    if (nullptr == mstat->md_odlink)
    {
        return 0;
    }

    msadsc_t *mend = nullptr;
    if (MF_OLKTY_ODER == mstat->md_cntflgs.mf_olkty)
    {
        mend = (msadsc_t *)mstat->md_odlink;
    }
    if (MF_OLKTY_BAFH == mstat->md_cntflgs.mf_olkty)
    {
        mend = mstat;
    }
    if (nullptr == mend)
    {
        return 0;
    }
    uint_t mnr = (mend - mstat) + 1;
    if (mnr != bafhp->af_oderpnr)
    {
        return 0;
    }
    if (MF_OLKTY_BAFH != mend->md_cntflgs.mf_olkty)
    {
        return 0;
    }
    if ((bafhlst_t *)(mend->md_odlink) != bafhp)
    {
        return 0;
    }

    u64_t phyadr = (~0UL);
    if (mnr == 1)
    {
        if (mstat->md_cntflgs.mf_marty != (u32_t)mareap->ma_type)
        {
            return 0;
        }
        if (PAF_NO_ALLOC != mstat->md_phyadrs.paf_alloc ||
            0 != mstat->md_cntflgs.mf_refcnt)
        {
            return 0;
        }
        phyadr = mstat->md_phyadrs.paf_padrs << PSHRSIZE;
        if (phyadr < mareap->ma_logicstart || (phyadr + PAGESIZE - 1) > mareap->ma_logicend)
        {
            return 0;
        }
        return 1;
    }
    uint_t idx = 0;
    for (uint_t mi = 0; mi < mnr - 1; mi++)
    {
        if (mstat[mi].md_cntflgs.mf_marty != (u32_t)mareap->ma_type)
        {
            return 0;
        }
        if (PAF_NO_ALLOC != mstat[mi].md_phyadrs.paf_alloc ||
            0 != mstat[mi].md_cntflgs.mf_refcnt)
        {
            return 0;
        }
        if (PAF_NO_ALLOC != mstat[mi + 1].md_phyadrs.paf_alloc ||
            0 != mstat[mi + 1].md_cntflgs.mf_refcnt)
        {
            return 0;
        }
        if (((mstat[mi].md_phyadrs.paf_padrs << PSHRSIZE) + PAGESIZE) != (mstat[mi + 1].md_phyadrs.paf_padrs << PSHRSIZE))
        {
            return 0;
        }
        if ((mstat[mi].md_phyadrs.paf_padrs << PSHRSIZE) < mareap->ma_logicstart ||
            (((mstat[mi + 1].md_phyadrs.paf_padrs << PSHRSIZE) + PAGESIZE) - 1) > mareap->ma_logicend)
        {
            return 0;
        }
        idx++;
    }
    return idx + 1;
}

bool_t check_one_bafhlst(bafhlst_t *bafhp, memarea_t *mareap)
{
    if (nullptr == bafhp || nullptr == mareap) {
        return FALSE;
    }
    if (1 > bafhp->af_amsanr && 1 > bafhp->af_fmsanr) {
        return TRUE;
    }

    uint_t lindx = 0;
    list_h_t *tmplst = nullptr;
    msadsc_t *msap = nullptr;

    list_for_each(tmplst, &bafhp->af_frelst)
    {
        msap = list_entry(tmplst, msadsc_t, md_list);
        if (bafhp->af_oderpnr != check_multi_msadsc(msap, bafhp, mareap))
        {
            return FALSE;
        }
        lindx++;
    }

    if (lindx != bafhp->af_fmsanr || lindx != bafhp->af_amsanr) {
        return FALSE;
    }
    return TRUE;
}

bool_t check_one_memarea(memarea_t *mareap)
{
    if (nullptr == mareap) {
        return FALSE;
    }
    if (1 > mareap->ma_maxpages) {
        return TRUE;
    }

    for (uint_t li = 0; li < MDIVMER_ARR_LMAX; li++)
    {
        if (check_one_bafhlst(&mareap->ma_mdmdata.dm_pools[li], mareap) == FALSE)
        {
            return FALSE;
        }
    }

    if (check_one_bafhlst(&mareap->ma_mdmdata.dm_onepool, mareap) == FALSE) {
        return FALSE;
    }
    return TRUE;
}

void mem_check_mareadata()
{
    memarea_t *marea = (memarea_t *)glomm.mo_mareastat;
    for (uint_t maidx = 0; maidx < glomm.mo_mareanr; maidx++)
    {
        if (check_one_memarea(&marea[maidx]) == FALSE) {

            system_error("check_one_memarea fail\n");
        }
    }
    return;
}

void init_merlove_mem()
{
    if (merlove_mem_core() == FALSE)
    {
        color_printk(RED, BLACK, "merlove_mem_core fail\n");
    }
    mem_check_mareadata();
    return;
}
