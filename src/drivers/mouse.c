#include "toolkit.h"
#include "devkit.h"
#include "mmkit.h"
#include "arch_x86kit.h"
#include "kernelkit.h"

// 鼠标输入缓存区
keyboard_inputbuffer_t *p_mouse = nullptr;
static s32_t mouse_count = 0;

void mouse_handler(u64_t nr, u64_t parameter, pt_regs_t *regs)
{
    u8_t x;
    x = io_in8(PORT_KB_DATA);
    if (p_mouse->p_head == p_mouse->buf + KB_BUF_SIZE)
        p_mouse->p_head = p_mouse->buf;

    *p_mouse->p_head = x;
    p_mouse->count++;
    p_mouse->p_head++;
}

u8_t get_mousecode()
{
    u8_t ret = 0;

    if (p_mouse->count == 0)
        while (!p_mouse->count)
            nop();

    if (p_mouse->p_tail == p_mouse->buf + KB_BUF_SIZE)
        p_mouse->p_tail = p_mouse->buf;

    ret = *p_mouse->p_tail;
    p_mouse->count--;
    p_mouse->p_tail++;

    return ret;
}

hw_int_controller mouse_int_controller =
    {
        .enable = IOAPIC_enable,
        .disable = IOAPIC_disable,
        .install = IOAPIC_install,
        .uninstall = IOAPIC_uninstall,
        .ack = IOAPIC_edge_ack,
};

void mouse_exit()
{
    unregister_irq(0x2c);
    kdelete(p_mouse, sizeof(keyboard_inputbuffer_t));
}

void mouse_init()
{
    io_apic_ret_entry_t entry;

    p_mouse = (keyboard_inputbuffer_t *)knew(sizeof(keyboard_inputbuffer_t), 0);
    p_mouse->p_head = p_mouse->buf;
    p_mouse->p_tail = p_mouse->buf;
    p_mouse->count = 0;
    memset(p_mouse->buf, 0, KB_BUF_SIZE);

    entry.vector = 0x2c;
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

    mouse_count = 0;

    register_irq(0x2c, &entry, &mouse_handler, (u64_t)p_mouse, &mouse_int_controller, "ps/2 mouse");
    // 开启鼠标端口
    wait_KB_write();
    io_out8(PORT_KB_CMD, KBCMD_EN_MOUSE_INTFACE);


    wait_KB_write();
    io_out8(PORT_KB_CMD, KBCMD_SENDTO_MOUSE); // 向鼠标设备发送数据
    wait_KB_write();
    io_out8(PORT_KB_DATA, MOUSE_ENABLE); // 允许鼠标设备发送数据


    // 重新配置8042控制器
    wait_KB_write();
    io_out8(PORT_KB_CMD, KBCMD_WRITE_CMD);
    wait_KB_write();
    io_out8(PORT_KB_DATA, KB_INIT_MODE);
}

void analysis_mousecode()
{
    u8_t x = get_mousecode();
    switch (mouse_count)
    {
    case 0: // 跳过初始化过程中的8042控制器返回的应答信息0xfA
        mouse_count++;
        break;
    case 1:
        mouse.Byte0 = x;
        mouse_count++;
        break;
    case 2:
        mouse.Byte1 = (char_t)x;
        mouse_count++;
        break;
    case 3:
        mouse.Byte2 = (char_t)x;
        mouse_count = 1;
        color_printk(RED, GREEN, "(M:%02x, X:%3d, Y:%3d)\n", mouse.Byte0, mouse.Byte1, mouse.Byte2);
        break;
    default:
        break;
    }
}