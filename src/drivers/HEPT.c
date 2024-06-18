#include "time.h"
#include "HEPT.h"
#include "interrupt.h"
#include "APIC.h"
#include "printk.h"
#include "lib.h"
#include "softirq.h"
#include "timer.h"
#include "schedule.h"
#include "ptrace.h"

#define IRQ0_FREQUENCY 100 // 1秒100个时钟中断 / 1
#define INPUT_FREQUENCY 1193180
#define COUNTER0_VALUE INPUT_FREQUENCY / IRQ0_FREQUENCY
#define CONTRER0_PORT 0x40
#define COUNTER0_NO 0
#define COUNTER_MODE 2
#define READ_WRITE_LATCH 3
#define PIT_CONTROL_PORT 0x43

#define mil_seconds_per_intr (1000 / IRQ0_FREQUENCY) // 1个时钟中断10毫秒, 1000/100

unsigned long volatile jiffies = 0; // ticks是内核自中断开启以来总共的嘀嗒数
extern struct timer_list timer_list_head;
extern struct time Time;

hw_int_controller HPET_int_controller =
    {
        .enable = IOAPIC_enable,
        .disable = IOAPIC_disable,
        .install = IOAPIC_install,
        .uninstall = IOAPIC_uninstall,
        .ack = IOAPIC_edge_ack,
};

// 24-06-14 17:22 bug报告：本次注释的ticks_to_sleep的函数，直接使用传入参数sleep_ticks（rdi）进行运算。
//                在系统执行一段时间后，传入参数sleep_ticks会变成脏数据。猜测是因为rdi寄存器在中断等地方，
//                切换的时候没有妥善保存导致的。sleep_ticks被修改 --> rdi寄存器被改变 --> 猜测在终端过程中改变。
//                
//
// /* 以tick为单位的sleep,任何时间形式的sleep会转换此ticks形式 */
// static void ticks_to_sleep(unsigned int sleep_ticks)
// {
//    unsigned int Old_ticks = jiffies;

//    /* 若间隔的ticks数不够便让出cpu */
//    while (jiffies - Old_ticks < sleep_ticks)
//       hlt();
// }

/* 以tick为单位的sleep,任何时间形式的sleep会转换此ticks形式 */
static void ticks_to_sleep(unsigned int sleep_ticks)
{
   unsigned int Old_ticks = jiffies;
   unsigned int mid_ticks = sleep_ticks;

   /* 若间隔的ticks数不够便让出cpu */
   while (jiffies - Old_ticks < mid_ticks)
      hlt();

}



/* 把操作的计数器counter_no、读写锁属性rwl、计数器模式counter_mode写入模式控制寄存器并赋予初始值counter_value */
static void frequency_set(unsigned char counter_port,
                          unsigned char counter_no,
                          unsigned char rwl,
                          unsigned char counter_mode,
                          unsigned short counter_value)
{
   /* 往控制字寄存器端口0x43中写入控制字 */
   io_out8(PIT_CONTROL_PORT, (unsigned char)(counter_no << 6 | rwl << 4 | counter_mode << 1));
   /* 先写入counter_value的低8位 */
   io_out8(counter_port, (unsigned char)counter_value);
   /* 再写入counter_value的高8位 */
   io_out8(counter_port, (unsigned char)(counter_value >> 8));
}

/* 时钟的中断处理函数 */
void intr_timer_handler(unsigned long nr, unsigned long parameter, struct pt_regs *regs)
{
   jiffies++;
   // 如果定时任务的失效日期没有到，那么不进入中断下半部
   if ((container_of(list_next(&timer_list_head.list), struct timer_list, list))->expire_jiffies <= jiffies)
      set_softirq_status(TIMER_SIRQ);

   // 依据进程的优先级，增加进程虚拟运行时间，减少处理器时间片维护代码
   switch (current->priority)
   {
   case 0:
   case 1:
      task_schedule.CPU_exec_task_jiffies--;
      current->vrun_time += 1;
      break;
   case 2:
   default:
      task_schedule.CPU_exec_task_jiffies -= 2;
      current->vrun_time += 2;
      break;
   }
   // 本进程的时间片耗尽，可调度下一个进程
   if (task_schedule.CPU_exec_task_jiffies <= 0)
      current->flags |= NEED_SCHEDULE;
}

/* 初始化PIT8253 */
void HEPT_init()
{
   jiffies = 0;
   /* 设置8253的定时周期,也就是发中断的周期 */
   frequency_set(CONTRER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER_MODE, COUNTER0_VALUE);

   struct IO_APIC_RET_entry entry;
   unsigned long i, j;

   entry.vector = 0x20;
   entry.deliver_mode = APIC_ICR_IOAPIC_Fixed;
   entry.dest_mode = ICR_IOAPIC_DELV_PHYSICAL;
   entry.deliver_status = APIC_ICR_IOAPIC_Idle;
   entry.polarity = APIC_IOAPIC_POLARITY_HIGH;
   entry.irr = APIC_IOAPIC_IRR_RESET;
   entry.trigger = APIC_ICR_IOAPIC_Edge;
   entry.mask = APIC_ICR_IOAPIC_Masked;
   entry.reserved = 0;

   entry.destination.physical.reserved1 = 0;
   entry.destination.physical.phy_dest = 0; // 物理模式
   entry.destination.physical.reserved2 = 0;

   register_irq(0x20, &entry, intr_timer_handler, 0, &HPET_int_controller, "HPET");

   get_cmos_time(&Time);
   color_printk(RED, BLACK, "year:%#010d, month:%#010d, day:%#010d,  week:%#010d, hour:%#010d, mintue:%#010d, second:%#010d\n",
   Time.year, Time.month, Time.day, Time.week_day, Time.hour, Time.minute, Time.second);
}

/* 以毫秒为单位的sleep   1秒= 1000毫秒 */
void mtime_sleep(unsigned int m_seconds)
{
   unsigned int sleep_ticks = (m_seconds + mil_seconds_per_intr - 1) / mil_seconds_per_intr;
   ticks_to_sleep(sleep_ticks);
}

/* 以秒为单位的sleep   1秒 */
void time_sleep(unsigned int seconds)
{
   mtime_sleep(seconds * 1000);
}

