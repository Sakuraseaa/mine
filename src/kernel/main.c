#include "lib.h"
#include "printk.h"
#include "gate.h"
#include "trap.h"
#include "memory.h"
#include "interrupt.h"
#include "task.h"
#include "cpu.h"
#if APIC
#include "APIC.h"
#else
#include "8259A.h"
#endif
#include "keyboard.h"
#include "mouse.h"
#include "disk.h"
#include "time.h"
#include "HEPT.h"
#include "softirq.h"
#include "SMP.h"
#include "schedule.h"
#include "semaphore.h"
#include "fat32.h"
#include "VFS.h"
#include "serial.h"
#include "debug.h"
#include "bitmap.h"
#include "device.h"
#include "buffer.h"

extern semaphore_T visual_lock;
extern struct keyboard_inputbuffer *p_kb;
extern struct keyboard_inputbuffer *p_mouse;
extern long global_pid;
struct Global_Memory_Descriptor memory_management_struct = {{0}, 0};
extern int usr_init();
extern unsigned long shell_boot(unsigned long arg);
extern int kernel_thread(unsigned long (*fn)(unsigned long), unsigned long arg, unsigned long flags);
extern int shell_up;

#include "test.h"
void Start_Kernel(void)
{
	global_pid = 1;

	Pos.XResolution = 1440;
	Pos.YResolution = 850;
	Pos.XPosition = 0;
	Pos.YPosition = 0;

	Pos.XCharSize = 8;
	Pos.YCharSize = 16;

	Pos.FB_addr = (unsigned int *)0xffff800003000000;
	Pos.FB_length = (Pos.XResolution * Pos.YResolution * 4 + PAGE_4K_SIZE - 1) & PAGE_4K_MASK; // ?

	semaphore_init(&visual_lock, 1);
	spin_init(&Pos.printk_lock);

	load_TR(10);

	set_tss64(0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00);

	// 初始化异常函数表 IDT
	sys_vector_init();
	
	init_cpu();

	memory_management_struct.start_code = (unsigned long)&_text;
	memory_management_struct.end_code = (unsigned long)&_etext;
	memory_management_struct.end_data = (unsigned long)&_edata;
	memory_management_struct.end_rodata = (unsigned long)&_erodata;
	memory_management_struct.start_brk = (unsigned long)&_end;

	// color_printk(RED, BLACK, "memory init \n");
	init_memory();

	// color_printk(RED, BLACK, "slab init \n");
	slab_init();

	frame_buffer_init();
	
	pagetable_4K_init();

	device_init();

	buffer_init();

	HEPT_init();

#if APIC
	APIC_IOAPIC_init();
#else
	init_8259A();
#endif

	serial_init();

	keyboard_init();

	mouse_init();

	disk_init();

	schedule_init();

	softirq_init();

	timer_init();

	VFS_init();

	DEBUGK("task init \n");
	task_init();
	sti();


	// 此处的while用于线程同步
	while (!shell_up)
		;
	// kernel_thread(shell_boot, 12, CLONE_FS | CLONE_SIGNAL);
	
	while (1)
	{
		// 这里可以遍历所有进程，寻找僵尸进程。释放他。
		// 但是这种做法有点笨，来点消息通知就好了
	}
}
