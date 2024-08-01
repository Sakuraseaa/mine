#include "types.h"
#include "memory.h"
#include "task.h"
#include "printk.h"

typedef struct{
    u64 start_vir, end_vir;
    u64 start_phy, end_phy;
}map_entry_t;

static void Print_Map(map_entry_t* map, const long i_m) {
    size_t i;
    // User
    for(i = 0; i < i_m; i++) {
        if(map[i].start_vir >= PAGE_OFFSET)     break;
        
        color_printk(WHITE, BLACK, "%#018lx ~ %#018lx",map[i].start_vir, map[i].end_vir);
        color_printk(YELLOW, BLACK, " <----> ");
        color_printk(WHITE, BLACK, "%#18lx ~ %#18lx", map[i].start_phy, map[i].end_phy);
        color_printk(BLUE, BLACK, " == ");
        color_printk(WHITE, BLACK, "[%d, %d)KB\n", 4 * (map[i].start_phy >> PAGE_4K_SHIFT), 4 * (map[i].end_phy >> PAGE_4K_SHIFT));
    }

    color_printk(RED, WHITE, "                                       Below is Kernel Map!                                         \n");
    // Kernel
    for(;i < i_m; i++) {
        color_printk(WHITE, BLACK, "%#018lx ~ %#018lx",map[i].start_vir, map[i].end_vir);
        color_printk(YELLOW, BLACK, " <----> ");
        color_printk(WHITE, BLACK, "%#18lx ~ %#18lx", map[i].start_phy, map[i].end_phy);
        color_printk(BLUE, BLACK, " == ");
        color_printk(WHITE, BLACK, "[%d, %d)MB\n", 2 * (map[i].start_phy >> PAGE_2M_SHIFT), 2 * (map[i].end_phy >> PAGE_2M_SHIFT));
    }   
}

// 行有不得，反求诸己。😤 <-😣 
// 在解析内核的时候会有很多条内核映射目录，倒数第二层的目录是有问题的 如何修改 ？😢 
/**
 * @brief  打印 tsk任务的 虚拟内存到物理内存的映射情况。
 * 
 * @param tsk 
 */
void User_Map(u64* PML4, map_entry_t* map, size_t* i_m) {
    u64 *PTDPE = 0, *PTDE = 0, *PTE = 0;
	size_t i = 0, j = 0, k = 0, z = 0;

    u64 start_vir = 0, end_vir = 0;
    u64 start_phy = 0, end_phy = 0;
    u64 tmp = 0;
	for(i = 0; i < 512; i++) {	// 遍历四级页表
        start_vir = 0;    

        if(i == 256)
            start_vir = ((unsigned long)0xffff000000000000);
        
        if((*(PML4 + i)) & PAGE_Present) {
			PTDPE = Phy_To_Virt(*(PML4 + i) & ~(0xfffUL)); // 屏蔽目录项标志位，获取PDPT页表地址
            
            start_vir += (i * PGTB_DPTB_MANAGE_SIZE);
			
            for (j = 0; j < 512; j++) { // 遍历页目录指针页表
				if((*(PTDPE + j)) & PAGE_Present) {
					start_vir +=  (j * PGTB_DTB_MANAGE_SIZE);
                    PTDE = Phy_To_Virt(*(PTDPE + j) & ~(0xfffUL)) ; //遍历页目录表项
                    
                    for(k = 0; k < 512; k++) {

						if((*(PTDE + k)) & PAGE_Present) {
                            
                            start_vir += (k * PGTB_TB_MANAGE_SIZE);
                            PTE = Phy_To_Virt(*(PTDE + k) & ~(0xfffUL)) ; //遍历页表项
                            for(z = 0; z < 512; z++) {
                                if((*(PTE + z)) & PAGE_Present) {
                                    
                                    tmp = start_vir +  z * PAGE_4K_SIZE;
                                    end_vir = tmp + PAGE_4K_SIZE;

                                    start_phy = ((*(PTE + z)) & PAGE_4K_MASK); // 得到物理页

                                    end_phy = start_phy + PAGE_4K_SIZE;
                                    
                                    map_entry_t* sk_addr = &map[(*i_m) - 1];

                                    if(((*i_m) > 0) && (map[(*i_m) - 1].end_vir == tmp && map[(*i_m) - 1].end_phy == start_phy))  {
                                        map[(*i_m) - 1].end_vir = end_vir; // 连续页
                                        map[(*i_m) - 1].end_phy = end_phy;
                                    } else {
                                        map[(*i_m)].start_vir = tmp; // 非连续页 新创建一项
                                        map[(*i_m)].end_vir = end_vir;
                                        map[(*i_m)].start_phy = start_phy;
                                        map[(*i_m)].end_phy = end_phy;
                                        (*i_m)++;
                                    }
                                }
                            }
                        }
                    }
				}
			}
		}
    }
}

void Kelner_Map(u64* PML4, map_entry_t* map, size_t* i_m) {
    u64 *PTDPE = 0, *PTDE = 0;
	size_t i = 0, j = 0, k = 0;

    u64 start_vir = 0, end_vir = 0;
    u64 start_phy = 0, end_phy = 0;
    u64 tmp = 0;
	for(i = 256; i < 512; i++) {	// 遍历 PML4 页表
        start_vir = ((unsigned long)0xffff000000000000);
        
        if(i == 511)
            i = 511;
        
        if((*(PML4 + i)) & PAGE_Present) {
			PTDPE = Phy_To_Virt(*(PML4 + i) & ~(0xfffUL)); // 屏蔽目录项标志位，获取PDPT页表地址

            start_vir += i * PGTB_DPTB_MANAGE_SIZE;
			
            for (j = 0; j < 512; j++) { // 遍历 PDPT 页表
				if((*(PTDPE + j)) & PAGE_Present) {
					start_vir +=  j * PGTB_DTB_MANAGE_SIZE;
                    PTDE = Phy_To_Virt(*(PTDPE + j) & ~(0xfffUL)) ; //遍历 PDT 页表项
					
                    for(k = 0; k < 512; k++) {
						if((*(PTDE + k)) & PAGE_Present) {
                            tmp = start_vir +  k * PAGE_2M_SIZE;
                            end_vir = tmp + PAGE_2M_SIZE;

                            start_phy = ((*(PTDE + k)) & PAGE_2M_MASK); // 得到物理页
                            end_phy = start_phy + PAGE_2M_SIZE;

                            if(((*i_m) > 0) && (map[(*i_m) - 1].end_vir == tmp && map[(*i_m) - 1].end_phy == start_phy)) {
                                map[(*i_m) - 1].end_vir = end_vir; // 连续页
                                map[(*i_m) - 1].end_phy = end_phy;
                            } else {
                                map[(*i_m)].start_vir = tmp; // 非连续页 新创建一项
                                map[(*i_m)].end_vir = end_vir;
                                map[(*i_m)].start_phy = start_phy;
                                map[(*i_m)].end_phy = end_phy;
                                (*i_m)++;
                            }
                        }
                    }
				}
			}
		}
    }
}

void test_show_vir_phy(struct task_struct *tsk) {

	long i_m = 0;

	struct mm_struct *newmm = tsk->mm;
	u64* PML4 = Phy_To_Virt(newmm->pgd);

    map_entry_t* map = kmalloc(sizeof(map_entry_t) * 200, 0);

    User_Map(PML4, map, &i_m);
    // Kelner_Map(PML4, map, &i_m);

    Print_Map(map, i_m);
    kfree(map);
}

