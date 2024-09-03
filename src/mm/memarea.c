#include "memory.h"
#include "marea_t.h"
#include "types.h"
#include "memmgrob.h"

void bafhlst_t_init(bafhlst_t *initp, u32_t stus, uint_t oder, uint_t oderpnr)
{
    //初始化bafhlst_t结构体的基本数据
    spin_init(&initp->af_lock);
    initp->af_stus = stus;
    initp->af_oder = oder;
    initp->af_oderpnr = oderpnr;
    initp->af_fobjnr = 0;
    initp->af_mobjnr = 0;
    initp->af_alcindx = 0;
    initp->af_freindx = 0;
    list_init(&initp->af_frelst);
    list_init(&initp->af_alclst);
    list_init(&initp->af_ovelst);
    return;
}

void memdivmer_t_init(memdivmer_t *initp)
{
    //初始化medivmer_t结构体的基本数据
    knl_spinlock_init(&initp->dm_lock);
    initp->dm_stus = 0;
    initp->dm_divnr = 0;
    initp->dm_mernr = 0;
    //循环初始化memdivmer_t结构体中dm_mdmlielst数组中的每个bafhlst_t结构的基本数据
    for (uint_t li = 0; li < MDIVMER_ARR_LMAX; li++)
    {
        bafhlst_t_init(&initp->dm_mdmlielst[li], BAFH_STUS_DIVM, li, (1UL << li));
    }
    bafhlst_t_init(&initp->dm_onemsalst, BAFH_STUS_ONEM, 0, 1UL);
    return;
}

void memarea_t_init(memarea_t *initp)
{
    //初始化memarea_t结构体的基本数据
    list_init(&initp->ma_list);
    knl_spinlock_init(&initp->ma_lock);
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
    //初始化memarea_t结构体中的memdivmer_t结构体
    memdivmer_t_init(&initp->ma_mdmdata);
    initp->ma_privp = NULL;
    return;
}

bool_t init_memarea_core()
{
    //获取memarea_t结构开始地址
    memarea_t *virmarea = (memarea_t *)memory_management_struct.end_of_struct;
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
    glomm.mo_mareanr = MEMAREA_MAX;
    //将所有memarea_t结构的大小写入kmachbsp结构中 
    //计算下一个空闲内存的开始地址 
    mbsp->mb_nextwtpadr = PAGE_ALIGN(phymarea + sizeof(memarea_t) * MEMAREA_MAX);
    return TRUE;
}
//初始化内存区
void init_memarea()
{
    //真正初始化内存区
    if (init_memarea_core() == FALSE)
    {
        color_printk(RED, BLACK, "init_memarea_core fail");
    }
    return;
}