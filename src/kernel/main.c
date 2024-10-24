#include "toolkit.h"
#include "fskit.h"
#include "mmkit.h"
#include "kernelkit.h"
#include "arch_x86kit.h"
#include "devkit.h"
#include "syskit.h"
#include "memmgrinit.h"
#include "krlmm.h"

extern semaphore_t visual_lock;
extern keyboard_inputbuffer_t *p_kb;
extern keyboard_inputbuffer_t *p_mouse;
extern s64_t global_pid;
struct Global_Memory_Descriptor memory_management_struct = {{{0}}, 0};
extern s32_t usr_init();
extern u64_t shell_boot(u64_t arg);
extern s32_t kernel_thread(u64_t (*fn)(u64_t), u64_t arg, u64_t flags);
extern s32_t shell_up;

#include "test.h"
static u64_t SKOFLOVE = 0;
void Start_Kernel(void)
{
	global_pid = 1;

	Pos.XResolution = 1440;
	Pos.YResolution = 750;
	Pos.XPosition = 0;
	Pos.YPosition = 0;

	Pos.XCharSize = 8;
	Pos.YCharSize = 16;

	Pos.FB_addr = (u32_t *)0xffff800003000000;
	Pos.FB_length = (Pos.XResolution * Pos.YResolution * 4 + PAGE_4K_SIZE - 1) & PAGE_4K_MASK; // ?

	semaphore_init(&visual_lock, 1);
	spin_init(&Pos.printk_lock);
	
	load_TR(10);

	set_tss64(0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00);

	// 初始化异常函数表 IDT
	sys_vector_init();
	
	// init_cpu();

	memory_management_struct.start_code = (u64_t)&_text;
	memory_management_struct.end_code = (u64_t)&_etext;
	memory_management_struct.end_data = (u64_t)&_edata;
	memory_management_struct.end_rodata = (u64_t)&_erodata;
	memory_management_struct.start_brk = (u64_t)&_end;

	init_memory();
	
	init_phymm();
	
	slab_init();

	frame_buffer_init();
	pagetable_4K_init();
	
	test_mmobj();
	init_krlmm();
	
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

	task_init();
	sti();


	// 此处的while用于线程同步
	while (!shell_up)
		SKOFLOVE++; 
	// kernel_thread(shell_boot, 12, CLONE_FS | CLONE_SIGNAL);
	
	while (1)
	{

		SKOFLOVE++; // 检查内核进程初始化完成在运行之后，是否还会再运行
		
		// 这里可以遍历所有进程，寻找僵尸进程。释放他。
		// 但是这种做法有点笨，来点消息通知就好了
	}
}
