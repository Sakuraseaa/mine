#include "types.h"
#include "memory.h"
#include "task.h"
#include "printk.h"

typedef struct{
    u64 start_vir, end_vir;
    u64 start_phy, end_phy;
}map_entry_t;

// 行有不得，反求诸己。😤 
/**
 * @brief  打印 tsk任务的 虚拟内存到物理内存的映射情况。
 * 
 * @param tsk 
 */
void test_show_vir_phy(struct task_struct *tsk) {
	u64 *PML4 = 0, *Directory_Ptr = 0, *Directory = 0;
	size_t i = 0, j = 0, k = 0, i_m = 0;

	struct mm_struct *newmm = tsk->mm;
	PML4 = Phy_To_Virt(newmm->pgd);

    map_entry_t map[20];

    u64 start_vir = 0, end_vir = 0;
    u64 start_phy = 0, end_phy = 0;
    u64 tmp = 0;
	for(i = 0; i < 512; i++) {	// 遍历 PML4 页表
		if(i > 255)
            start_vir = ((unsigned long)0xffff000000000000);
        
        if(i == 511)
            i = 511;
        
        if((*(PML4 + i)) & PAGE_Present) {
			Directory_Ptr = Phy_To_Virt(*(PML4 + i) & ~(0xfffUL)); // 屏蔽目录项标志位，获取PDPT页表地址

            start_vir += i * PGTB_DPTB_MANAGE_SIZE;
			
            for (j = 0; j < 512; j++) { // 遍历 PDPT 页表
				if((*(Directory_Ptr + j)) & PAGE_Present) {
					start_vir +=  j * PGTB_DTB_MANAGE_SIZE;
                    Directory = Phy_To_Virt(*(Directory_Ptr + j) & ~(0xfffUL)) ; //遍历 PDT 页表项
					
                    for(k = 0; k < 512; k++) {
						if((*(Directory + k)) & PAGE_Present) {
                            tmp = start_vir +  k * PAGE_2M_SIZE;
                            end_vir = tmp + PAGE_2M_SIZE;

                            start_phy = ((*(Directory + k)) & PAGE_2M_MASK); // 得到物理页
                            end_phy = start_phy + PAGE_2M_SIZE;

                            if((i_m > 0) && (map[i_m - 1].end_vir == tmp && map[i_m - 1].end_phy == start_phy)) {
                                map[i_m - 1].end_vir = end_vir; // 连续页
                                map[i_m - 1].end_phy = end_phy;
                            } else {
                                map[i_m].start_vir = start_vir; // 非连续页 新创建一项
                                map[i_m].end_vir = end_vir;
                                map[i_m].start_phy = start_phy;
                                map[i_m].end_phy = end_phy;
                                i_m++;
                            }
                        }
                    }
				}
			}
		}
	}
    bool one = false;
    color_printk(WHITE,BLACK, "\n");
    for(i = 0; i < i_m; i++) {
        if(one == false && map[i].start_vir >= PAGE_OFFSET) {
            color_printk(RED, WHITE, "                                       Below is Kernel Map!                                         \n");
            one = true;
        }
        color_printk(WHITE, BLACK, "%#018lx ~ %#018lx",map[i].start_vir, map[i].end_vir);
        color_printk(YELLOW, BLACK, " <----> ");
        color_printk(WHITE, BLACK, "%#18lx ~ %#18lx", map[i].start_phy, map[i].end_phy);
        color_printk(BLUE, BLACK, " == ");
        color_printk(WHITE, BLACK, "[%d, %d)MB\n", 2 * (map[i].start_phy >> PAGE_2M_SHIFT), 2 * (map[i].end_phy >> PAGE_2M_SHIFT));
    }
}