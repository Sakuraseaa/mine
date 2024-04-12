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

extern semaphore_T visual_lock;
extern struct keyboard_inputbuffer *p_kb;
extern struct keyboard_inputbuffer *p_mouse;
extern long global_pid;
struct Global_Memory_Descriptor memory_management_struct = {{0}, 0};

void Start_Kernel(void)
{
	global_pid = 1;

	Pos.XResolution = 1440;
	Pos.YResolution = 700;
	Pos.XPosition = 0;
	Pos.YPosition = 0;

	Pos.XCharSize = 8;
	Pos.YCharSize = 16;

	Pos.FB_addr = (int *)0xffff800003000000;
	Pos.FB_length = (Pos.XResolution * Pos.YResolution * 4 + PAGE_4K_SIZE - 1) & PAGE_4K_MASK; // ?

	semaphore_init(&visual_lock, 1);
	spin_init(&Pos.printk_lock);

	load_TR(10);

	set_tss64(0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00);

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

	// color_printk(RED, BLACK, "frame buffer init \n");
	frame_buffer_init();

	// color_printk(RED, BLACK, "pagetable init \n");
	pagetable_init();

	// color_printk(RED, BLACK, "interrupt init \n");
#if APIC
	APIC_IOAPIC_init();
#else
	init_8259A();
#endif

	// color_printk(RED, BLACK, "keyboard init \n");
	keyboard_init();

	// color_printk(RED, BLACK, "mouse init \n");
	mouse_init();
	sti();
	while (1)
	{
		if (p_kb->count)
			analysis_keycode();
		if (p_mouse->count)
			analysis_mousecode();
	}

	// color_printk(RED, BLACK, "disk init \n");
	disk_init();

	// color_printk(RED, BLACK, "schedule init \n");
	schedule_init();
	// color_printk(RED, BLACK, "Soft IRQ init \n");
	softirq_init();
	// color_printk(RED, BLACK, "Timer init \n");
	timer_init();
	// color_printk(RED, BLACK, "HPET init \n");
	HEPT_init();

	// color_printk(RED, BLACK, "task init \n");
	task_init();

	// unsigned char buf[512];
	// color_printk(PURPLE, BLACK, "disk write:\n");
	// memset(buf, 0x44, 512);
	// IDE_device_operation.transfer(ATA_WRITE_CMD, 0x78, 1, (unsigned char *)buf);

	// color_printk(PURPLE, BLACK, "disk write end\n");

	// color_printk(PURPLE, BLACK, "disk read:\n");
	// memset(buf, 0x00, 512);
	// IDE_device_operation.transfer(ATA_READ_CMD, 0x78, 1, (unsigned char *)buf);

	// for (i = 0; i < 512; i++)
	// 	color_printk(BLACK, WHITE, "%02x", buf[i]);
	// color_printk(PURPLE, BLACK, "\ndisk read end\n");

	// IDE_device_operation.ioctl(GET_IDENTIFY_DISK_CMD, NULL);
}
