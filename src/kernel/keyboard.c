#include "keyboard.h"
#include "lib.h"
#include "interrupt.h"
#include "APIC.h"
#include "memory.h"
#include "printk.h"

struct keyboard_inputbuffer *p_kb = NULL;
static int shift_l, shift_r, ctrl_l, ctrl_r, alt_l, alt_r;

hw_int_controller keyboard_int_controller =
    {
        .enable = IOAPIC_enable,
        .disable = IOAPIC_disable,
        .install = IOAPIC_install,
        .uninstall = IOAPIC_uninstall,
        .ack = IOAPIC_edge_ack,
};

// 键盘中断处理函数
void keyboard_handler(unsigned long nr, unsigned long parameter, struct pt_regs *regs)
{
    unsigned char x;
    x = io_in8(PORT_KB_DATA);

    if (p_kb->p_head == p_kb->buf + KB_BUF_SIZE)
        p_kb->p_head = p_kb->buf;

    *p_kb->p_head = x;
    p_kb->count++;
    p_kb->p_head++;
}

// 键盘初始化函数(挂载函数)
void keyboard_init()
{
    struct IO_APIC_RET_entry entry;
    unsigned long i, j;

    p_kb = (struct keyboard_inputbuffer *)kmalloc(sizeof(struct keyboard_inputbuffer), 0);
    // 初始化键盘缓存区的队列
    p_kb->p_head = p_kb->buf;
    p_kb->p_tail = p_kb->buf;
    p_kb->count = 0;
    memset(p_kb->buf, 0, KB_BUF_SIZE);

    entry.vector = 0x21;
    entry.deliver_mode = APIC_ICR_IOAPIC_Fixed;  // Fixed = LVT寄存器的向量号区域指定中断向量号
    entry.dest_mode = ICR_IOAPIC_DELV_PHYSICAL;  // 使用 APIC ID号来确定接受中断消息的处理器
    entry.deliver_status = APIC_ICR_IOAPIC_Idle; // 不启用投递模式
    entry.polarity = APIC_IOAPIC_POLARITY_HIGH;  // 电平触发极性，高电平触发
    entry.irr = APIC_IOAPIC_IRR_RESET;           // Local APIC处理中断请求时置位，在收到处理器发来的EOI命令时复位
    entry.trigger = APIC_ICR_IOAPIC_Edge;        // 用于设置LINT0 和 LINT1引脚的触发模式，0为边沿触发模式，1为电平触发模式
    entry.mask = APIC_ICR_IOAPIC_Masked;         // 中断屏蔽标志位，1表示中断未屏蔽
    entry.reserved = 0;

    entry.destination.physical.reserved1 = 0;
    entry.destination.physical.phy_dest = 0; // 物理模式
    entry.destination.physical.reserved2 = 0;

    // 初始化8042寄存器
    wait_KB_write();
    io_out8(PORT_KB_CMD, KBCMD_WRITE_CMD);
    wait_KB_write();
    io_out8(PORT_KB_DATA, KB_INIT_MODE);

    for (i = 0; i < 1000; i++)
        for (j = 0; j < 1000; j++)
            nop();

    shift_l = 0;
    shift_r = 0;
    ctrl_l = 0;
    ctrl_r = 0;
    alt_l = 0;
    alt_r = 0;

    register_irq(0x21, &entry, &keyboard_handler, (unsigned long)p_kb, &keyboard_int_controller, "ps/2 keyboard");
}

// 驱动卸载函数 - 什么时候实现动态加载？
void keyboard_exit()
{
    unregister_irq(0x21);
    kfree((unsigned long *)p_kb);
}

// 从键盘缓冲区里得到一个字符
unsigned char get_scancode()
{
    unsigned char ret = 0;
    if (p_kb->count == 0)
        while (!p_kb->count)
            nop();
    if (p_kb->p_tail == p_kb->buf + KB_BUF_SIZE)
        p_kb->p_tail = p_kb->buf;

    ret = *p_kb->p_tail;
    p_kb->count--;
    p_kb->p_tail++;

    return ret;
}

void analysis_keycode()
{
    unsigned char x = 0;
    int i, key = 0, make = 0;

    // 从键盘缓存区里面得到一个断码/通码
    x = get_scancode();

    if (x == 0xE1) // 一个字符一个字符的匹配 Pause 键, 我一般不会按 Pause 键
    {
        key = PAUSEBREAK;
        for (i = 1; i < 6; i++)
            if (get_scancode() != pausebreak_scode[i])
            {
                key = 0;
                break;
            }
    }
    else if (x == 0xE0)
    { // 这里只对Print Screen，Right Ctrl, Right Alt三个第二类键盘扫描码进行检测, 我一般也不会按这几个
        x = get_scancode();
        switch (x)
        {
        case 0x2A: // press printscreen
            if (get_scancode() == 0xE0)
                if (get_scancode() == 0x37)
                {
                    key = PRINTSCREEN;
                    make = 1;
                }
            break;
        case 0xB7: // UNpress printscreen
            if (get_scancode() == 0xE0)
                if (get_scancode() == 0xAA)
                {
                    key = PRINTSCREEN;
                    make = 0;
                }
            break;
        case 0x1d: // press right ctrl
            ctrl_r = 1;
            key = OTHERKEY;
            break;
        case 0x9d: // UNpress right ctrl
            ctrl_r = 0;
            key = OTHERKEY;
            break;
        case 0x38: // press right alt
            alt_r = 1;
            key = OTHERKEY;
            break;
        case 0xb8: // UNpress right alt
            alt_r = 0;
            key = OTHERKEY;
            break;
        default:
            key = OTHERKEY;
            break;
        }
    }

    if (key == 0) // key == 0, 则对x进行第三类键盘扫描码的匹配 包括我常用的那些键
    {
        unsigned int *keyrow = NULL;
        int column = 0;

        // 判断键盘扫描码描述的是按下状态还是抬起状态，通码(1) or 断码(0)
        make = (x & FLAG_BREAK ? 0 : 1);

        // 计算出键盘扫描码在数组中的位置, &7F的过程屏蔽的断码的0x80
        keyrow = &keycode_map_normal[(x & 0x7F) * MAP_COLS];

        // 检测到shift按下, 按下就改变k
        if (shift_l || shift_r)
            column = 1;
        key = keyrow[column]; // 现在的k 已经是真正的键盘上对应的那个字符

        switch (x & 0x7F)
        {
        case 0x2a: // SHIFT_L
            shift_l = make;
            key = 0;
            break;
        case 0x36: // SHIFT_R
            shift_r = make;
            key = 0;
            break;
        case 0x1d: // CTRL_L
            ctrl_l = make;
            key = 0;
            break;
        case 0x38: // ALT_L
            alt_l = make;
            key = 0;
            break;
        default:
            if (!make) // 忽略断码
                key = 0;
            break;
        }
    }

    if (key)
        color_printk(RED, BLACK, "%c", key);
}
