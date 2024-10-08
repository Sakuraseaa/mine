#ifndef _APIC_H_
#define _APIC_H_

/* 不支持x2APIC, 在支持x2APIC的平台上，请使用书上的源代码*/
// Local APIC Register Address map
#define APIC_IDR (u32_t *)Phy_To_Virt(0xFEE00020)
#define APIC_VesrionR (u32_t *)Phy_To_Virt(0xFEE00030)
#define APIC_TaskPR (u32_t *)Phy_To_Virt(0xFEE00080)
#define APIC_ProcessorPR (u32_t *)Phy_To_Virt(0xFEE000A0)
#define APIC_SpuriousIVR (u32_t *)Phy_To_Virt(0xFEE000F0);
#define APIC_EOI (u32_t *)Phy_To_Virt(0xFEE000B0)
// LVT
#define CMCIR (u32_t *)Phy_To_Virt(0xFEE002F0)
#define TimerR (u32_t *)Phy_To_Virt(0xFEE00320)
#define ThermalSensorR (u32_t *)Phy_To_Virt(0xFEE00330)
#define PerformanceMonitoringCounterR (u32_t *)Phy_To_Virt(0xFEE00340)
#define LINT0R (u32_t *)Phy_To_Virt(0xFEE00350)
#define LINT1R (u32_t *)Phy_To_Virt(0xFEE00360)
#define ErrorR (u32_t *)Phy_To_Virt(0xFEE00370)

void ioapic_EOI(u64_t nr);
void do_IRQ(pt_regs_t *regs, u64_t nr);
void APIC_IOAPIC_init();
void Local_APIC_init();
u64_t ioapic_rte_read(u8_t index);
void ioapic_rte_write(u8_t index, u64_t value);

struct IOAPIC_map
{
    u32_t physical_address;        // 4B-保存着间接访问寄存器的物理基地址
    u8_t *virtual_index_address; // 记录着I/O APIC寄存器组的间接访问寄存器的索引寄存器_线性地址
    u32_t *virtual_data_address;   // 数据寄存器_线性地址
    u32_t *virtual_EOI_address;    // EOI寄存器_线性地址
} ioapic_map;

// I/O 中断定向投递寄存器组 - RTE
typedef struct IO_APIC_RET_entrya
{
    u32_t vector : 8, // 0 ~ 7 中断向量号
        deliver_mode : 3,    // 8 ~ 10 选择投递模式
        dest_mode : 1,       // 11 目标模式
        deliver_status : 1,  // 12 投递状态
        polarity : 1,        // 13 电平触发纪性
        irr : 1,             // 14 远程IRR标志位
        trigger : 1,         // 15 触发模式
        mask : 1,            // 16 屏蔽标志位
        reserved : 15;       // 17~31 保留

    union
    {
        struct
        {
            u32_t reserved1 : 24, // 32 ~ 55
                phy_dest : 4,            // 56 ~ 59, 投递目标
                reserved2 : 4;           // 60 ~ 63
        } physical;

        struct
        {
            u32_t reserved1 : 24, // 32 ~ 55 保留
                logical_dest : 8;        // 56 ~ 63  投递目标
        } logical;
    } destination;
} __attribute__((packed)) io_apic_ret_entry_t;

// delivery mode
#define APIC_ICR_IOAPIC_Fixed 0      // LAPIC	IOAPIC 	ICR
#define IOAPIC_ICR_Lowest_Priority 1 //	IOAPIC 	ICR
#define APIC_ICR_IOAPIC_SMI 2        // LAPIC	IOAPIC 	ICR

#define APIC_ICR_IOAPIC_NMI 4  // LAPIC	IOAPIC 	ICR
#define APIC_ICR_IOAPIC_INIT 5 // LAPIC	IOAPIC 	ICR
#define ICR_Start_up 6         //		ICR
#define IOAPIC_ExtINT 7        //	IOAPIC

// timer mode
#define APIC_LVT_Timer_One_Shot 0
#define APIC_LVT_Timer_Periodic 1
#define APIC_LVT_Timer_TSC_Deadline 2

// mask
#define APIC_ICR_IOAPIC_Masked 1
#define APIC_ICR_IOAPIC_UN_Masked 0

// trigger mode
#define APIC_ICR_IOAPIC_Edge 0
#define APIC_ICR_IOAPIC_Level 1

// delivery status
#define APIC_ICR_IOAPIC_Idle 0
#define APIC_ICR_IOAPIC_Send_Pending 1

// destination shorthand
#define ICR_No_Shorthand 0
#define ICR_Self 1
#define ICR_ALL_INCLUDE_Self 2
#define ICR_ALL_EXCLUDE_Self 3

// destination mode
#define ICR_IOAPIC_DELV_PHYSICAL 0
#define ICR_IOAPIC_DELV_LOGIC 1

// level
#define ICR_LEVEL_DE_ASSERT 0
#define ICR_LEVLE_ASSERT 1

// remote irr
#define APIC_IOAPIC_IRR_RESET 0
#define APIC_IOAPIC_IRR_ACCEPT 1

// pin polarity
#define APIC_IOAPIC_POLARITY_HIGH 0
#define APIC_IOAPIC_POLARITY_LOW 1

#endif // _APIC_H_