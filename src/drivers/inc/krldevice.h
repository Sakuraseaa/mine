#ifndef _KRLDEVICE_T_H
#define _KRLDEVICE_T_H

/**
 * @brief 首先 devtable_t 结构中能找到所有的设备和驱动，
 * 然后从设备能找到对应的驱动，
 * 从驱动也能找到其管理的所有设备 ，
 * 最后就能实现一个驱动管理多个设备。
 */
#define NOT_DEVICE 0               //不表示任何设备
#define BRIDGE_DEVICE 4            //总线桥接器设备
#define CPUCORE_DEVICE 5           //CPU设备，CPU也是设备
#define RAMCONTER_DEVICE 6        //内存控制器设备
#define RAM_DEVICE 7              //内存设备
#define USBHOSTCONTER_DEVICE 8    //USB主控制设备
#define INTUPTCONTER_DEVICE 9     //中断控制器设备
#define DMA_DEVICE 10             //DMA设备
#define CLOCKPOWER_DEVICE 11      //时钟电源设备
#define LCDCONTER_DEVICE 12        //LCD控制器设备
#define NANDFLASH_DEVICE 13       //nandflash设备
#define CAMERA_DEVICE 14          //摄像头设备
#define UART_DEVICE 15             //串口设备
#define TIMER_DEVICE 16            //定时器设备
#define USB_DEVICE 17              //USB设备
#define WATCHDOG_DEVICE 18        //看门狗设备
#define RTC_DEVICE 22              //实时时钟设备
#define SD_DEVICE 25               //SD卡设备
#define AUDIO_DEVICE 26            //音频设备
#define TOUCH_DEVICE 27           //触控设备
#define NETWORK_DEVICE 28         //网络设备
#define VIR_DEVICE 29               //虚拟设备
#define FILESYS_DEVICE 30            //文件系统设备
#define SYSTICK_DEVICE 31           //系统TICK设备
#define UNKNOWN_DEVICE 32        //未知设备，也是设备
#define HD_DEVICE 33        //硬盘设备
#define DEVICE_MAX 34

#define IOIF_CODE_OPEN 0 //对应于open操作
#define IOIF_CODE_CLOSE 1 //对应于close操作
#define IOIF_CODE_READ 2 //对应于read操作
#define IOIF_CODE_WRITE 3 //对应于write操作
#define IOIF_CODE_LSEEK 4 //对应于lseek操作
#define IOIF_CODE_IOCTRL 5 //对应于ioctrl操作
#define IOIF_CODE_DEV_START 6 //对应于start操作
#define IOIF_CODE_DEV_STOP 7 //对应于stop操作
#define IOIF_CODE_SET_POWERSTUS 8 //对应于powerstus操作
#define IOIF_CODE_ENUM_DEV 9 //对应于enum操作
#define IOIF_CODE_FLUSH 10 //对应于flush操作
#define IOIF_CODE_SHUTDOWN 11 //对应于shutdown操作
#define IOIF_CODE_MAX 12 //最大功能码

#define DEVFLG_EXCLU (1<<0)
#define DEVFLG_SHARE (1<<1)
#define DEVSTS_NORML (1<<0)
#define DEVSTS_FAILU (1<<1)
#define DIDFIL_IDN 1
#define DIDFIL_FLN 2

#define FSDEV_IOCTRCD_DELFILE 5
#define FSDEV_OPENFLG_NEWFILE 1
#define FSDEV_OPENFLG_OPEFILE 2

typedef struct s_DEVID
{
    uint_t  dev_mtype;//设备类型号
    uint_t  dev_stype; //设备子类型号
    uint_t  dev_nr; //设备序号
}devid_t;

typedef struct s_DEVTLST
{
    uint_t dtl_type;//设备类型
    uint_t dtl_nr;//设备计数
    list_h_t dtl_list;//挂载设备device_t结构的链表
}devtlst_t;

typedef struct s_DEVTABLE
{
    list_h_t devt_list; //设备表自身的链表
    spinlock_t devt_lock; //设备表自旋锁
    list_h_t devt_devlist; //全局设备链表
    list_h_t devt_drvlist; //全局驱动程序链表，驱动程序不需要分类，一个链表就行
    uint_t   devt_devnr; //全局设备计数
    uint_t   devt_drvnr; //全局驱动程序计数
    devtlst_t devt_devclsl[DEVICE_MAX]; //分类存放设备数据结构的devtlst_t结构数组
}devtable_t;

/* 设备结构体 */
typedef struct s_DEVICE
{
    list_n_t    dev_list;//设备链表
    list_n_t    dev_indrvlst; //设备在驱动程序数据结构中对应的挂载链表
    list_n_t    dev_intbllst; //设备在设备表数据结构中对应的挂载链表
    spinlock_t  dev_lock; //设备自旋锁
    uint_t      dev_count; //设备计数
    sem_t       dev_sem; //设备信号量
    uint_t      dev_stus; //设备状态
    uint_t      dev_flgs; //设备标志
    devid_t      dev_id; //设备ID
    uint_t      dev_intlnenr; //设备中断服务例程的个数
    list_h_t    dev_intserlst; //设备中断服务例程的链表
    list_h_t    dev_rqlist; //对设备的请求服务链表
    uint_t      dev_rqlnr; //对设备的请求服务个数
    sem_t       dev_waitints; //用于等待设备的信号量
    struct s_DRIVER* dev_drv; //设备对应的驱动程序数据结构的指针
    void* dev_attrb; //设备属性指针
    void* dev_privdata; //设备私有数据指针
    void* dev_userdata;//将来扩展所用
    void* dev_extdata;//将来扩展所用
    char_t* dev_name; //设备名
}device_t;

/* 驱动结构体 */
typedef drvstus_t (*drivcallfun_t)(device_t*,void*);
typedef drvstus_t (*drventyexit_t)(struct s_DRIVER*,uint_t,void*);
typedef struct s_DRIVER
{
    spinlock_t drv_lock; //保护驱动程序数据结构的自旋锁
    list_h_t drv_list;//挂载驱动程序数据结构的链表
    uint_t drv_stuts; //驱动程序的相关状态
    uint_t drv_flg; //驱动程序的相关标志
    uint_t drv_id; //驱动程序ID
    uint_t drv_count; //驱动程序的计数器
    sem_t drv_sem; //驱动程序的信号量
    void* drv_safedsc; //驱动程序的安全体
    void* drv_attrb; //LMOSEM内核要求的驱动程序属性体
    void* drv_privdata; //驱动程序私有数据的指针
    drivcallfun_t drv_dipfun[IOIF_CODE_MAX]; //驱动程序功能派发函数指针数组
    list_h_t drv_alldevlist; //挂载驱动程序所管理的所有设备的链表
    drventyexit_t drv_entry; //驱动程序的入口函数指针
    drventyexit_t drv_exit; //驱动程序的退出函数指针
    void* drv_userdata;//用于将来扩展
    void* drv_extdata; //用于将来扩展
    char_t* drv_name; //驱动程序的名字
}driver_t;



#endif // KRLDEVICE_T_H

