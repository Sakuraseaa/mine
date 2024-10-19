#ifndef _PAGES64_T_H
#define _PAGES64_T_H

// Transaltion_Lookaside_Buffer, 刷新虚拟地址 vaddr 的 块表 TLB
#define flush_tlb_one(addr)                                   \
    __asm__ __volatile__("invlpg	(%0)	\n\t" ::"r"(addr) \
                         : "memory")

#define flush_tlb()               \
    do                            \
    {                             \
        u64_t tmpreg;     \
        __asm__ __volatile__(     \
            "movq	%%cr3,	%0	\n\t" \
            "movq	%0,	%%cr3	\n\t" \
            : "=r"(tmpreg)        \
            :                     \
            : "memory");          \
    } while (0)

inline static u64_t *Get_gdt()
{
    u64_t *tmp;
    __asm__ __volatile__(
        "movq	%%cr3,	%0	\n\t"
        : "=r"(tmp)
        :
        : "memory");
    return tmp;
}
//	8Bytes per cell, 页表项个数
#define PTRS_PER_PAGE 512

#define PAGE_OFFSET ((u64_t)0xffff800000000000)
#define TASK_SIZE ((u64_t)0x00007fffffffffff)
#define STASK_USR_START ((u64_t)0x00007ffffffff000)

#define PAGE_GDT_SHIFT 39
#define PAGE_1G_SHIFT 30 // 2的30次方是1GB
#define PAGE_2M_SHIFT 21 // 2的21次方是2MB
#define PAGE_4K_SHIFT 12 // 2的12次方是4KB

#define PGTB_ENTRY 512
#define PAGE_2M_SIZE (1UL << PAGE_2M_SHIFT)
#define PAGE_4K_SIZE (1UL << PAGE_4K_SHIFT)

#define PGTB_TB_MANAGE_SIZE     PAGE_4K_SIZE * PGTB_ENTRY           // 2MB
#define PGTB_DTB_MANAGE_SIZE    PGTB_TB_MANAGE_SIZE * PGTB_ENTRY    // 1GB
#define PGTB_DPTB_MANAGE_SIZE   PGTB_DTB_MANAGE_SIZE * PGTB_ENTRY   // 512 GB
#define PGTB_PML4_MANAGE_SIZE   PGTB_DPTB_MANAGE_SIZE * PGTB_ENTRY  // 256 TB


#define PAGE_2M_MASK (~(PAGE_2M_SIZE - 1)) // 用于屏蔽低2MB的数值
#define PAGE_4K_MASK (~(PAGE_4K_SIZE - 1))

#define PAGE_2M_ALIGN(addr) (((u64_t)(addr) + PAGE_2M_SIZE - 1) & PAGE_2M_MASK) // 把参数addr按2MB页的上边界对齐
#define PAGE_4K_ALIGN(addr) (((u64_t)(addr) + PAGE_4K_SIZE - 1) & PAGE_4K_MASK)

#define Virt_To_Phy(addr) ((u64_t)(addr) - PAGE_OFFSET) // 该函数将内核层虚拟地址转换成物理地址

// 把物理地址转换成虚拟地址，这种切换的把戏，只有一一映射，才能使用
#define Phy_To_Virt(addr) ((u64_t *)((u64_t)(addr) + PAGE_OFFSET))


#define KPML4_P (1<<0)
#define KPML4_RW (1<<1)
#define KPML4_US (1<<2)
#define KPML4_PWT (1<<3)
#define KPML4_PCD (1<<4)
#define KPML4_A (1<<5)

#define KPDPTE_P (1<<0)
#define KPDPTE_RW (1<<1)
#define KPDPTE_US (1<<2)
#define KPDPTE_PWT (1<<3)
#define KPDPTE_PCD (1<<4)
#define KPDPTE_A (1<<5)

#define KPDE_P (1<<0)
#define KPDE_RW (1<<1)
#define KPDE_US (1<<2)
#define KPDE_PWT (1<<3)
#define KPDE_PCD (1<<4)
#define KPDE_A (1<<5)
#define KPDE_D (1<<6)
#define KPDE_PS (1<<7)
#define KPDE_G (1<<8)
#define KPDE_PAT (1<<12)

#define KPML4_SHIFT 39
#define KPDPTTE_SHIFT 30
#define KPDP_SHIFT 21
#define PGENTY_SIZE 512

#define PAGEMAPBASS 0x100000
#define PSHRSIZE 12
#define PDENSHL  22
#define PTENSHL  12
#define PDTNSHR  10
#define PTENSHR  10
#define PDTNSIZE (1UL<<PDTNSHR)
#define PTENSIZE (1UL<<PTENSHR)
#define PAGESIZE (1UL<<PSHRSIZE)
#define PRMSIZEO (1UL<<PDENSHL)
#define PDTN_MASK	(~(PDTNSIZE-1))
#define PTEN_MASK	(~(PTENSIZE-1))
#define PAGE_MASK       (~(PAGESIZE-1))
#define PAGE_ALIGN(n) ALIGN(n,PAGESIZE)// (((n)+0xfff)&0xfffff000)// PAGE_ALIGN(x)((PAGESIZE-(x&(~(PAGE_MASK))))+x)
#define PTPNFUN(phyadr) (phyadr>>PSHRSIZE)
#define PNTPFUN(PN) (PN<<PSHRSIZE)
#define PDENFN(phyadr) ((phyadr>>PDENSHL))
#define PTENFN(phyadr) ((phyadr>>PTENSHL)&(~PTEN_MASK))
#define PDESIZE 1024


typedef struct s_KPML4
{
	u64_t p_val;
}__attribute__((packed)) kpml4_t;


typedef struct s_KPDPTE
{
	u64_t p_val;
}__attribute__((packed)) kpdpte_t;


typedef struct s_KPDE
{
	u64_t p_val;
}__attribute__((packed)) kpde_t;

typedef struct
{
    u64_t pml4t;
} pml4t_t;
#define mk_mpl4t(addr, attr) ((u64_t)(addr) | (u64_t)(attr))
#define set_mpl4t(mpl4tptr, mpl4tval) (*(mpl4tptr) = (mpl4tval))

typedef struct
{
    u64_t pdpt;
} pdpt_t;
#define mk_pdpt(addr, attr) ((u64_t)(addr) | (u64_t)(attr))
#define set_pdpt(pdptptr, pdptval) (*(pdptptr) = (pdptval))

typedef struct
{
    u64_t pdt;
} pdt_t;
#define mk_pdt(addr, attr) ((u64_t)(addr) | (u64_t)(attr))
#define set_pdt(pdtptr, pdtval) (*(pdtptr) = (pdtval))

typedef struct
{
    u64_t pt;
} pt_t;
#define mk_pt(addr, attr) ((u64_t)(addr) | (u64_t)(attr))
#define set_pt(ptptr, ptval) (*(ptptr) = (ptval))
// ===============================================================

////page table attribute

//	bit 63	Execution Disable:
#define PAGE_XD (1UL << 63)

//	bit 12	Page Attribute Table
#define PAGE_PAT (1UL << 12)

//	bit 8	Global Page:1,global;0,part
#define PAGE_Global (1UL << 8)

//	bit 7	Page Size:1,big page;0,small page;
#define PAGE_PS (1UL << 7)

//	bit 6	Dirty:1,dirty;0,clean;
#define PAGE_Dirty (1UL << 6)

//	bit 5	Accessed:1,visited;0,unvisited;
#define PAGE_Accessed (1UL << 5)

//	bit 4	Page Level Cache Disable
#define PAGE_PCD (1UL << 4)

//	bit 3	Page Level Write Through
#define PAGE_PWT (1UL << 3)

//	bit 2	User Supervisor:1,user and supervisor;0,supervisor;
#define PAGE_U_S (1UL << 2)

//	bit 1	Read Write:1,read and write;0,read;
#define PAGE_R_W (1UL << 1)

//	bit 0	Present:1,present;0,no present;
#define PAGE_Present (1UL << 0)

// 1,0
#define PAGE_KERNEL_GDT (PAGE_R_W | PAGE_Present)

// 1,0
#define PAGE_KERNEL_Dir (PAGE_R_W | PAGE_Present)

// 7,1,0
#define PAGE_KERNEL_Page (PAGE_PS | PAGE_R_W | PAGE_Present)

#define PAGE_USER_PML4 (PAGE_U_S | PAGE_R_W | PAGE_Present)
// 1,0
#define PAGE_USER_GDT (PAGE_U_S | PAGE_R_W | PAGE_Present)

// 2,1,0
#define PAGE_USER_Dir (PAGE_U_S | PAGE_R_W | PAGE_Present)
// 2,1,0
#define PAGE_USER_Page_4K (PAGE_U_S | PAGE_R_W | PAGE_Present)
// 7,2,1,0
#define PAGE_USER_Page (PAGE_PS | PAGE_U_S | PAGE_R_W | PAGE_Present)

// 7,2,0 , only read
#define PAGE_USER_Page_OR (PAGE_PS | PAGE_U_S | PAGE_Present)

////alloc_pages zone_select
#define ZONE_DMA (1 << 0)
#define ZONE_NORMAL (1 << 1)
#define ZONE_UNMAPED (1 << 2)

////struct page attribute (alloc_pages flags)
#define PG_PTable_Maped (1 << 0) // 经过页表映射的页/未在页表中映射, mapped = 1 or un-mapped = 0
#define PG_Kernel_Init (1 << 1)  // 内核初始化程序/非内核初始化程序, init-code = 1 or normal-code/date=0
#define PG_Referenced (1 << 2)
#define PG_Dirty (1 << 3)
#define PG_Active (1 << 4) // 使用中的页
#define PG_Up_To_Date (1 << 5)
#define PG_Device (1 << 6) // 设备寄存器/物理内存地址  device = 1 or memory =0
#define PG_Kernel (1 << 7) // 内核层页/应用层页   kernel = 1 or user = 0
#define PG_Shared (1 << 8) // 是否被共享   shared = 1 or single-use = 0
#define PG_Slab (1 << 9)
#endif