// 本系统不支持多核处理器
#include "toolkit.h"
#include "arch_x86kit.h"
#include "kernelkit.h"
void SMP_init()
{
 
    u32_t a, b, c, d;
    // char i;
    // get local APIC ID
    // for (i = 0;; i++)
    {
        get_cpuid(0xb, 0, &a, &b, &c, &d);
        // if ((c >> 8 & 0xff) == 0)
        //     break;
        color_printk(WHITE, BLACK, "local APIC ID Package_../Core_2/SMT_1,type(%x) Width:%#010x,num of logical processor(%x)\n", c >> 8 & 0xff, a & 0x1f, b & 0xff);

        color_printk(WHITE, BLACK, "x2APIC ID level:%#010x\tx2APIC ID the current logical processor:%#010x\n", c & 0xff, d);

        // color_printk(WHITE, BLACK, "SMP copy byte:%#010x\n", (u64_t)&_APU_boot_end - (u64_t)&_APU_boot_start);
        // memcpy(_APU_boot_start, (u8_t *)0xffff800000020000, (u64_t)&_APU_boot_end - (u64_t)&_APU_boot_start);
    }
}

s32_t SMP_cpu_id() {
    return 0;
}
