#include "mmkit.h"


extern memmgrob_t glomm;
extern struct Global_Memory_Descriptor memory_management_struct;
#define PMR_T_OSAPUSERRAM 1


void msadsc_t_init(msadsc_t *initp)
{
	list_init(&initp->md_list);
	spin_init(&initp->md_lock);
	initp->md_indxflgs.mf_olkty = MF_OLKTY_INIT; // Overlay Link Type 挂入链表类型， 内核 or 用户?
	initp->md_indxflgs.mf_lstty = MF_LSTTY_LIST; // List type 链表类型，空闲链表，已分配链表
	initp->md_indxflgs.mf_mocty = MF_MOCTY_FREE; // Memory Occupation Type 内存占用类型, 空闲/被内核使用/被用户使用
	initp->md_indxflgs.mf_marty = MF_MARTY_INIT; // Memory Area Type 内存区域类型，内核区/用户区/设备区/
	initp->md_indxflgs.mf_uindx = MF_UINDX_INIT; // Usage index 使用计数
	initp->md_phyadrs.paf_alloc = PAF_NO_ALLOC;
	initp->md_phyadrs.paf_shared = PAF_NO_SHARED;
	initp->md_phyadrs.paf_swap = PAF_NO_SWAP;
	initp->md_phyadrs.paf_cache = PAF_NO_CACHE;
	initp->md_phyadrs.paf_kmap = PAF_NO_KMAP;
	initp->md_phyadrs.paf_lock = PAF_NO_LOCK;
	initp->md_phyadrs.paf_dirty = PAF_NO_DIRTY;
	initp->md_phyadrs.paf_busy = PAF_NO_BUSY;
	initp->md_phyadrs.paf_rv2 = PAF_RV2_VAL;    // reserve - 保留
	initp->md_phyadrs.paf_padrs = PAF_INIT_PADRS; // 页地址
	initp->md_odlink = nullptr; // Adjacent Link / Ordered Link 相邻链接-有序链接
	return;
}

void init_one_msadsc(msadsc_t *msap, u64_t phyadr)
{
    //对msadsc_t结构做基本的初始化，比如链表、锁、标志位
    msadsc_t_init(msap);
    //这是把一个64位的变量地址转换成phyadrflgs_t*类型方便取得其中的地址位段
    phyadrflgs_t *tmp = (phyadrflgs_t *)(&phyadr);
    //把页的物理地址写入到msadsc_t结构中
    msap->md_phyadrs.paf_padrs = tmp->paf_padrs;
    return;
}

u64_t init_msadsc_core(msadsc_t *msavstart, u64_t msanr)
{
    //获取phymmarge_t结构数组开始地址
    u64_t mdindx = 0, i, start, end;

    for (i = 0; i <= memory_management_struct.e820_length; i++)
    {
        if (memory_management_struct.e820[i].type != 1)
            continue;

        // start向上取整，end向下取整
        start = memory_management_struct.e820[i].address;
        end = memory_management_struct.e820[i].address + memory_management_struct.e820[i].length;
        if(end <= 0x100000UL)   // 忽略低于1MB的物理地址
            continue;
        
        for(;start < end; start += PAGE_4K_SIZE) { // 遍历区间
            
            if ((start + 4096 - 1) <= end) // 确保 start 到 end 之间任有 4KB 内存
			{
				init_one_msadsc(&msavstart[mdindx++], start);
            }
        }
    }
    return mdindx;
}


// return msadsc virtual address and size
void ret_msadsc_vadrandsz(msadsc_t **msavstart, u64_t* msar) {
    u64_t   TotalMem = 0, i;
    // 把可操作的地址对齐到2MB, 计算有多少2MB物理页可用
    for (i = 0; i <= memory_management_struct.e820_length; i++)
    {
        u64_t start, end;
        if (memory_management_struct.e820[i].type != 1)
            continue;

        // start向上取整，end向下取整
        start = PAGE_4K_ALIGN(memory_management_struct.e820[i].address);
        end = ((memory_management_struct.e820[i].address + memory_management_struct.e820[i].length) >> PAGE_4K_SHIFT) << PAGE_4K_SHIFT;

        if(end <= 0x100000UL)   // 忽略低于1MB的物理地址
            continue;

        if (end <= start)
            continue;
        TotalMem += (end - start) >> PAGE_4K_SHIFT;
    }
    
    *msar = TotalMem;
    *msavstart = (msadsc_t*)(memory_management_struct.end_of_struct);
    
    color_printk(BLUE, BLACK,"OS having %d 4k pagesize  = %d MB\n", TotalMem, (TotalMem * PAGE_4K_SIZE) / 1024 / 1024);
    
    return;
}

void disp_one_msadsc(msadsc_t *mp)
{
	color_printk(WHITE,BLACK,"msadsc_t.md_f:_ux[%x],_my[%x],md_phyadrs:_alc[%x],_shd[%x],_swp[%x],_che[%x],_kmp[%x],_lck[%x],_dty[%x],_bsy[%x],_padrs[0x%x]\n",
		   (uint_t)mp->md_indxflgs.mf_uindx, (uint_t)mp->md_indxflgs.mf_mocty, (uint_t)mp->md_phyadrs.paf_alloc, (uint_t)mp->md_phyadrs.paf_shared, (uint_t)mp->md_phyadrs.paf_swap, (uint_t)mp->md_phyadrs.paf_cache, (uint_t)mp->md_phyadrs.paf_kmap, (uint_t)mp->md_phyadrs.paf_lock,
		   (uint_t)mp->md_phyadrs.paf_dirty, (uint_t)mp->md_phyadrs.paf_busy, (uint_t)(mp->md_phyadrs.paf_padrs << 12));
	return;
}


void init_msadsc()
{
    u64_t coremdnr = 0, msadscnr = 0;
    msadsc_t *msadscvp = nullptr;
    
    //计算msadsc_t结构数组的开始地址和数组元素个数
    // return msadsc virtual address and size
    ret_msadsc_vadrandsz(&msadscvp, &msadscnr);
    
    //初始化 msadsc_t 结构数组 的核心逻辑
    coremdnr = init_msadsc_core(msadscvp, msadscnr);
    if (coremdnr != msadscnr)
    {
        color_printk(RED, BLACK,"init_msadsc init_msadsc_core err\n");
        return;
    }
    
    //将msadsc_t结构数组的开始的物理地址写入kmachbsp结构中 
    glomm.mo_msadscstat = msadscvp;
    glomm.mo_msanr = msadscnr;
    memory_management_struct.end_of_struct += (sizeof(msadsc_t) * coremdnr + sizeof(u64_t) * 2) & (~(sizeof(u64_t)-1));
    
    // for(int i = 159; i < 165; i++) //1MB
    //     disp_one_msadsc(glomm.mo_msadscstat + i);

    return;
}


//搜索一段内存地址空间所对应的msadsc_t结构
u64_t search_segment_occupymsadsc(msadsc_t *msastart, u64_t msanr, u64_t ocpystat, u64_t ocpyend)
{
    u64_t mphyadr = 0, fsmsnr = 0, mnr, tmpadr;
    msadsc_t *fstatmp = nullptr;
    for (mnr = 0; mnr < msanr; mnr++)
    {
        if ((msastart[mnr].md_phyadrs.paf_padrs << PSHRSIZE) == ocpystat)
        {
            //找出开始地址对应的第一个msadsc_t结构，就跳转到step1
            fstatmp = &msastart[mnr];
            goto step1;
        }
    }
step1:
    fsmsnr = 0;
    if (nullptr == fstatmp)
    {
        return 0;
    }
    for (tmpadr = ocpystat; tmpadr < ocpyend; tmpadr += PAGESIZE, fsmsnr++)
    {
        //从开始地址对应的第一个msadsc_t结构开始设置，直到结束地址对应的最后一个masdsc_t结构
        mphyadr = fstatmp[fsmsnr].md_phyadrs.paf_padrs << PSHRSIZE;
        if (mphyadr != tmpadr)
        {
            return 0;
        }
        if (MF_MOCTY_FREE != fstatmp[fsmsnr].md_indxflgs.mf_mocty ||
            0 != fstatmp[fsmsnr].md_indxflgs.mf_uindx ||
            PAF_NO_ALLOC != fstatmp[fsmsnr].md_phyadrs.paf_alloc)
        {
            return 0;
        }
        //设置msadsc_t结构为已经分配，已经分配给内核
        fstatmp[fsmsnr].md_indxflgs.mf_mocty = MF_MOCTY_KRNL;
        fstatmp[fsmsnr].md_indxflgs.mf_uindx++;
        fstatmp[fsmsnr].md_phyadrs.paf_alloc = PAF_ALLOC;
    }
    //进行一些数据的正确性检查
    u64_t ocpysz = ocpyend - ocpystat;
    if ((ocpysz & 0xfff) != 0)
    {
        if (((ocpysz >> PSHRSIZE) + 1) != fsmsnr)
        {
            return 0;
        }
        return fsmsnr;
    }
    if ((ocpysz >> PSHRSIZE) != fsmsnr)
    {
        return 0;
    }
    return fsmsnr;
}


bool_t search_krloccupymsadsc_core()
{
    u64_t retschmnr = 0;
    msadsc_t *msadstat = glomm.mo_msadscstat;
    u64_t msanr = glomm.mo_msanr;
    //搜索BIOS中断表占用的内存页所对应msadsc_t结构
    retschmnr = search_segment_occupymsadsc(msadstat, msanr, 0, 0x200000);
    retschmnr = search_segment_occupymsadsc(msadstat, msanr, 0x100000, PAGE_2M_ALIGN(Virt_To_Phy(memory_management_struct.end_of_struct)));
    if (0 == retschmnr)
    {
        return FALSE;
    }
//     //搜索内核栈占用的内存页所对应msadsc_t结构
//     retschmnr = search_segment_occupymsadsc(msadstat, msanr, mbsp->mb_krlinitstack & (~(0xfffUL)), mbsp->mb_krlinitstack);
//     if (0 == retschmnr)
//     {
//         return FALSE;
//     }
//     //搜索内核占用的内存页所对应msadsc_t结构
//     retschmnr = search_segment_occupymsadsc(msadstat, msanr, mbsp->mb_krlimgpadr, mbsp->mb_nextwtpadr);
//     if (0 == retschmnr)
//     {
//         return FALSE;
//     }
//     //搜索内核映像文件占用的内存页所对应msadsc_t结构
//     retschmnr = search_segment_occupymsadsc(msadstat, msanr, mbsp->mb_imgpadr, mbsp->mb_imgpadr + mbsp->mb_imgsz);
//     if (0 == retschmnr)
//     {
//         return FALSE;
//     }
    return TRUE;
}


//初始化搜索内核占用的内存页面
// init search kernel occupy memory
void init_search_krloccupymm()
{
    //实际初始化搜索内核占用的内存页面
    if (search_krloccupymsadsc_core() == FALSE)
    {
        color_printk(RED, BLACK,"search_krloccupymsadsc_core fail\n");
    }
    return;
}
adr_t msadsc_ret_addr(msadsc_t *msa)
{
    if (nullptr == msa) {
        return INVIALID;
    }
    return (msa->md_phyadrs.paf_padrs << PAGPHYADR_SZLSHBIT);
}

adr_t msadsc_ret_vaddr(msadsc_t *msa)
{
    if (nullptr == msa) {
        return INVIALID;
    }
    return (adr_t)Phy_To_Virt(msadsc_ret_addr(msa));
}