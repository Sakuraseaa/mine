#include "devkit.h"

GLOBVAR_DEFINITION(devtable_t, osdevtable);
GLOBVAR_DEFINITION(drventyexit_t, osdrvetytabl, []={NULL});

//在 cosmos/kernel/krldevice.c文件中
void devtlst_t_init(devtlst_t *initp, uint_t dtype)
{
    initp->dtl_type = dtype;//设置设备类型    initp->dtl_nr = 0;
    list_init(&initp->dtl_list);
    return;
}

void devtable_t_init(devtable_t *initp)
{
    list_init(&initp->devt_list);
    spin_init(&initp->devt_lock);
    list_init(&initp->devt_devlist);
    list_init(&initp->devt_drvlist);
    initp->devt_devnr = 0;
    initp->devt_drvnr = 0;
    for (uint_t t = 0; t < DEVICE_MAX; t++)
    {//初始化设备链表
        devtlst_t_init(&initp->devt_devclsl[t], t);
    }
    return;
}

void init_krldevice()
{
    devtable_t_init(&osdevtable);//初始化系统全局设备表
    return;
}

/* 初始化设备id */
void devid_t_init(devid_t *initp, uint_t mty, uint_t sty, uint_t nr)
{
    initp->dev_mtype = mty;
    initp->dev_stype = sty;
    initp->dev_nr = nr;
    return;
}

/* 初始化设备结构体 */
void device_t_init(device_t *initp)
{
    list_init(&initp->dev_list);
    list_init(&initp->dev_indrvlst);
    list_init(&initp->dev_intbllst);
    spin_init(&initp->dev_lock);
    initp->dev_count = 0;
    // krlsem_t_init(&initp->dev_sem);
    initp->dev_stus = 0;
    initp->dev_flgs = 0;
    devid_t_init(&initp->dev_id, 0, 0, 0);
    initp->dev_intlnenr = 0;
    list_init(&initp->dev_intserlst);
    list_init(&initp->dev_rqlist);
    initp->dev_rqlnr = 0;
    // krlsem_t_init(&initp->dev_waitints);
    initp->dev_drv = NULL;
    initp->dev_attrb = NULL;
    initp->dev_privdata = NULL;
    initp->dev_userdata = NULL;
    initp->dev_extdata = NULL;
    initp->dev_name = NULL;

    return;
}

void krlretn_driverid(driver_t *dverp)
{
    dverp->drv_id = (uint_t)dverp;
    return;
}

void driver_t_init(driver_t *initp)
{
    spin_init(&initp->drv_lock);
    list_init(&initp->drv_list);
    initp->drv_stuts = 0;
    initp->drv_flg = 0;
    krlretn_driverid(initp);
    initp->drv_count = 0;
    // krlsem_t_init(&initp->drv_sem);
    initp->drv_safedsc = NULL;
    initp->drv_attrb = NULL;
    initp->drv_privdata = NULL;

    for (uint_t dsi = 0; dsi < IOIF_CODE_MAX; dsi++)
    {
        initp->drv_dipfun[dsi] = drv_defalt_func;
    }
    list_init(&initp->drv_alldevlist);
    initp->drv_entry = NULL;
    initp->drv_exit = NULL;
    initp->drv_userdata = NULL;
    initp->drv_extdata = NULL;
    initp->drv_name = NULL;
    return;
}


drvstus_t krlrun_driverentry(drventyexit_t drventry)
{
    driver_t *drvp = new_driver_dsc(); // 建立设备描述符
    if (drvp == NULL) {
        return DFCERRSTUS;
    }

    if (drventry(drvp, 0, NULL) == DFCERRSTUS) { // 运行驱动程序入口函数
        return DFCERRSTUS;
    }

    if (krldriver_add_system(drvp) == DFCERRSTUS) { // 把驱动程序加入系统
        return DFCERRSTUS;
    }
    return DFCOKSTUS;
}

void init_krldriver()
{
    for (uint_t ei = 0; osdrvetytabl[ei] != NULL; ei++)
    {
        if (krlrun_driverentry(osdrvetytabl[ei]) == DFCERRSTUS)
        {
            color_printk(RED, BLACK, "init driver err");
        }
    }
    return;
}


drvstus_t del_driver_dsc(driver_t *drvp)
{
    if (kdelete((adr_t)drvp, sizeof(driver_t)) == FALSE)
    {
        return DFCERRSTUS;
    }
    return DFCOKSTUS;
}

driver_t *new_driver_dsc()
{
    driver_t *dp = (driver_t *)knew(sizeof(driver_t));
    if (dp == NULL)
    {
        return NULL;
    }
    driver_t_init(dp);

    return dp;
}

drvstus_t del_device_dsc(device_t *devp)
{
    if (kdelete((adr_t)devp, sizeof(device_t)) == FALSE)
    {
        return DFCERRSTUS;
    }
    return DFCOKSTUS;
}

device_t *new_device_dsc()
{
    device_t *dp = (device_t *)knew(sizeof(device_t));
    if (dp == NULL)
    {
        return NULL;
    }
    device_t_init(dp);

    return dp;
}

drvstus_t drv_defalt_func(device_t *devp, void *iopack)
{
    return DFCERRSTUS;
}

bool_t krlcmp_devid(devid_t *sdidp, devid_t *cdidp)
{
    if (sdidp->dev_mtype != cdidp->dev_mtype)
    {
        return FALSE;
    }
    if (sdidp->dev_stype != cdidp->dev_stype)
    {
        return FALSE;
    }
    if (sdidp->dev_nr != cdidp->dev_nr)
    {
        return FALSE;
    }
    return TRUE;
}

drvstus_t knew_devid(devid_t *devid)
{
    device_t *findevp;
    drvstus_t rets = DFCERRSTUS;
    cpuflg_t cpufg;
    list_h_t *lstp;
    devtable_t *dtbp = &osdevtable;
    uint_t devmty = devid->dev_mtype;
    uint_t devidnr = 0;
    if (devmty >= DEVICE_MAX)
    {
        return DFCERRSTUS;
    }

    // krlspinlock_cli(&dtbp->devt_lock, &cpufg);
    if (devmty != dtbp->devt_devclsl[devmty].dtl_type)
    {
        rets = DFCERRSTUS;
        goto return_step;
    }
    if (list_is_empty(&dtbp->devt_devclsl[devmty].dtl_list) == TRUE)
    {
        rets = DFCOKSTUS;
        devid->dev_nr = 0;
        goto return_step;
    }
    list_for_each(lstp, &dtbp->devt_devclsl[devmty].dtl_list)
    {
        findevp = list_entry(lstp, device_t, dev_intbllst);
        if (findevp->dev_id.dev_nr > devidnr)
        {
            devidnr = findevp->dev_id.dev_nr;

        }
    }
    devid->dev_nr = devidnr++;
    rets = DFCOKSTUS;
return_step:
    // krlspinunlock_sti(&dtbp->devt_lock, &cpufg);
    return rets;
}

// 驱动加入内核: 将 driver_t 结构的实例变量挂载到设备表中
drvstus_t krldriver_add_system(driver_t *drvp)
{
    cpuflg_t cpufg;
    devtable_t *dtbp = &osdevtable;
    // krlspinlock_cli(&dtbp->devt_lock, &cpufg);
    list_add_to_behind(&dtbp->devt_drvlist, &drvp->drv_list);
    dtbp->devt_drvnr++;
    // krlspinunlock_sti(&dtbp->devt_lock, &cpufg);
    return DFCOKSTUS;
}

/* 把设备加入驱动 */
drvstus_t krldev_add_driver(device_t *devp, driver_t *drvp)
{
    list_h_t *lst;
    device_t *fdevp;
    if (devp == NULL || drvp == NULL) {
        return DFCERRSTUS;
    }
    // 一个驱动程序可以管理多个设备，所以在上述代码中，
    // 要遍历驱动设备链表中的所有设备，看看有没有设备 ID 冲突。
    
    // 遍历这个驱动上所有设备
    list_for_each(lst, &drvp->drv_alldevlist)
    { //比较设备ID有相同的则返回错误
        fdevp = list_entry(lst, device_t, dev_indrvlst);
        if (krlcmp_devid(&devp->dev_id, &fdevp->dev_id) == TRUE)
        {
            return DFCERRSTUS;
        }
    }
    //将设备挂载到驱动上
    list_add_to_behind(&drvp->drv_alldevlist, &devp->dev_indrvlst);
    devp->dev_drv = drvp;
    return DFCOKSTUS;
}

/* 向内核注册设备 */
drvstus_t knew_device(device_t *devp)
{
    device_t *findevp;
    drvstus_t rets = DFCERRSTUS;
    cpuflg_t cpufg;
    list_h_t *lstp;
    devtable_t *dtbp = &osdevtable;
    uint_t devmty = devp->dev_id.dev_mtype;
    if (devp == NULL) {
        return DFCERRSTUS;
    }
    if (devp->dev_drv == NULL) {    // 没有驱动的设备不能向内核注册
        return DFCERRSTUS;
    }
    if (devmty >= DEVICE_MAX) {
        return DFCERRSTUS;
    }

    // krlspinlock_cli(&dtbp->devt_lock, &cpufg);
    if (devmty != dtbp->devt_devclsl[devmty].dtl_type)
    {
        rets = DFCERRSTUS;
        goto return_step;
    }

    // 遍历设备类型链表上所有设备
    list_for_each(lstp, &dtbp->devt_devclsl[devmty].dtl_list)
    {
        findevp = list_entry(lstp, device_t, dev_intbllst);
        if (krlcmp_devid(&devp->dev_id, &findevp->dev_id) == TRUE)
        { // 不能有设有ID相同的设备有，则出错
            rets = DFCERRSTUS;
            goto return_step;
        }
    }

    // 把设备加入到对应设备类型的链表
    list_add_to_behind(&dtbp->devt_devclsl[devmty].dtl_list, &devp->dev_intbllst);
    
    // 把设备加入设备表的全局设备链表
    list_add_to_behind(&dtbp->devt_devlist, &devp->dev_list);
    
    dtbp->devt_devclsl[devmty].dtl_nr++; // 设备计数加一
    dtbp->devt_devnr++; // 总的设备数加一
    
    rets = DFCOKSTUS;
return_step:
    // krlspinunlock_sti(&dtbp->devt_lock, &cpufg);
    return rets;
}

drvstus_t krldev_inc_devcount(device_t *devp)
{

    if (devp->dev_count >= (~0UL))
    {
        return DFCERRSTUS;
    }
    cpuflg_t cpufg;
    // hal_spinlock_saveflg_cli(&devp->dev_lock, &cpufg);
    devp->dev_count++;
    // hal_spinunlock_restflg_sti(&devp->dev_lock, &cpufg);
    return DFCOKSTUS;
}

drvstus_t krldev_dec_devcount(device_t *devp)
{

    if (devp->dev_count < (1))
    {
        return DFCERRSTUS;
    }
    cpuflg_t cpufg;
    // hal_spinlock_saveflg_cli(&devp->dev_lock, &cpufg);
    devp->dev_count--;
    // hal_spinunlock_restflg_sti(&devp->dev_lock, &cpufg);
    return DFCOKSTUS;
}

drvstus_t krldev_add_request(device_t *devp, objnode_t *request)
{
    cpuflg_t cpufg;
    objnode_t *np = (objnode_t *)request;
    // krlspinlock_cli(&devp->dev_lock, &cpufg);
    list_add_to_behind(&devp->dev_rqlist, &np->on_list);
    devp->dev_rqlnr++;
    // krlspinunlock_sti(&devp->dev_lock, &cpufg);
    return DFCOKSTUS;
}

drvstus_t krldev_complete_request(device_t *devp, objnode_t *request)
{
    if (devp == NULL || request == NULL)
    {
        return DFCERRSTUS;
    }
    if (devp->dev_rqlnr < 1)
    {
        system_error("krldev_complete_request err devp->dev_rqlnr<1");
    }
    cpuflg_t cpufg;
    // krlspinlock_cli(&devp->dev_lock, &cpufg);
    list_del(&request->on_list);
    devp->dev_rqlnr--;
    // krlspinunlock_sti(&devp->dev_lock, &cpufg);
    // krlsem_up(&request->on_complesem);
    return DFCOKSTUS;
}

drvstus_t krldev_retn_request(device_t *devp, uint_t iocode, objnode_t **retreq)
{
    if (retreq == NULL || iocode >= IOIF_CODE_MAX)
    {
        return DFCERRSTUS;
    }
    cpuflg_t cpufg;
    objnode_t *np;
    list_h_t *list;
    drvstus_t rets = DFCERRSTUS;
    // krlspinlock_cli(&devp->dev_lock, &cpufg);
    list_for_each(list, &devp->dev_rqlist)
    {
        np = list_entry(list, objnode_t, on_list);
        if (np->on_opercode == (sint_t)iocode)
        {
            *retreq = np;
            rets = DFCOKSTUS;
            goto return_step;
        }
    }
    rets = DFCERRSTUS;
    *retreq = NULL;
return_step:
    // krlspinunlock_sti(&devp->dev_lock, &cpufg);
    return rets;
}

drvstus_t krldev_wait_request(device_t *devp, objnode_t *request)
{
    if (devp == NULL || request == NULL)
    {
        return DFCERRSTUS;
    }
    // krlsem_down(&request->on_complesem);
    return DFCOKSTUS;
}

void krldev_wait_intupt(device_t *devp)
{
    return;
}

void krldev_up_intupt(device_t *devp)
{
    return;
}

drvstus_t krldev_retn_rqueparm(void *request, buf_t *retbuf, uint_t *retcops, uint_t *retlen, uint_t *retioclde, uint_t *retbufcops, size_t *retbufsz)
{
    objnode_t *ondep = (objnode_t *)request;
    if (ondep == NULL)
    {
        return DFCERRSTUS;
    }
    if (retbuf != NULL)
    {
        *retbuf = ondep->on_buf;
    }
    if (retcops != NULL)
    {
        *retcops = ondep->on_currops;
    }
    if (retlen != NULL)
    {
        *retlen = ondep->on_len;
    }
    if (retioclde != NULL)
    {
        *retioclde = ondep->on_ioctrd;
    }
    if (retbufcops != NULL)
    {
        *retbufcops = ondep->on_bufcurops;
    }
    if (retbufsz != NULL)
    {
        *retbufsz = ondep->on_bufsz;
    }
    return DFCOKSTUS;
}

device_t *krlonidfl_retn_device(void *dfname, uint_t flgs)
{
    device_t *findevp;
    cpuflg_t cpufg;
    list_h_t *lstp;
    devtable_t *dtbp = &osdevtable;

    if (dfname == NULL || flgs != DIDFIL_IDN)
    {
        return NULL;
    }
    devid_t *didp = (devid_t *)dfname;
    uint_t devmty = didp->dev_mtype;
    if (devmty >= DEVICE_MAX)
    {
        return NULL;
    }
    // krlspinlock_cli(&dtbp->devt_lock, &cpufg);
    if (devmty != dtbp->devt_devclsl[devmty].dtl_type)
    {
        findevp = NULL;
        goto return_step;
    }
    list_for_each(lstp, &dtbp->devt_devclsl[devmty].dtl_list)
    {
        findevp = list_entry(lstp, device_t, dev_intbllst);
        if (krlcmp_devid(didp, &findevp->dev_id) == TRUE)
        {
            findevp = findevp;
            goto return_step;
        }
    }

    findevp = NULL;
return_step:
    // krlspinunlock_sti(&dtbp->devt_lock, &cpufg);
    return findevp;
}

drvstus_t knew_devhandle(device_t *devp, intflthandle_t handle, uint_t phyiline)
{
    // intserdsc_t *sdp = krladd_irqhandle(devp, handle, phyiline);
    // if (sdp == NULL)
    // {

    //     return DFCERRSTUS;
    // }
    // cpuflg_t cpufg;
    // // krlspinlock_cli(&devp->dev_lock, &cpufg);

    // list_add_to_behind(&devp->dev_intserlst, &sdp->s_indevlst);
    // devp->dev_intlnenr++;
    // krlspinunlock_sti(&devp->dev_lock, &cpufg);
    return DFCOKSTUS;
}

drvstus_t krldev_io(objnode_t *nodep)
{
    device_t *devp = (device_t *)(nodep->on_objadr);
    if ((nodep->on_objtype != OBJN_TY_DEV && nodep->on_objtype != OBJN_TY_FIL) || nodep->on_objadr == NULL)
    {
        return DFCERRSTUS;
    }
    if (nodep->on_opercode < 0 || nodep->on_opercode >= IOIF_CODE_MAX)
    {
        return DFCERRSTUS;
    }
    return krldev_call_driver(devp, nodep->on_opercode, 0, 0, NULL, nodep);
}

drvstus_t krldev_call_driver(device_t *devp, uint_t iocode, uint_t val1, uint_t val2, void *p1, void *p2)
{
    driver_t *drvp = NULL;
    if (devp == NULL || iocode >= IOIF_CODE_MAX)
    {
        return DFCERRSTUS;
    }
    drvp = devp->dev_drv;
    if (drvp == NULL)
    {
        return DFCERRSTUS;
    }
    return drvp->drv_dipfun[iocode](devp, p2);
}

