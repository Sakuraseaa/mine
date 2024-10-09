#include "toolkit.h"
#include "fskit.h"
#include "devkit.h"
#include "arch_x86kit.h"
#include "mmkit.h"
#include "kernelkit.h"
struct keyboard_inputbuffer *p_kb = NULL;
wait_queue_t keyboard_wait_queue; // 等待队列头

/**
 * @brief 在操作前清空缓冲区
 *
 * @param inode
 * @param filp
 * @return long
 */
s64_t keyboard_open(struct index_node *inode, struct file *filp)
{
    // 给文件描述符filp, 关联上硬盘数据缓冲区keyboard_inputbuffer
    filp->private_data = p_kb;

    p_kb->p_head = p_kb->buf;
    p_kb->p_tail = p_kb->buf;
    memset(p_kb->buf, 0, KB_BUF_SIZE);
    return 1;
}

/**
 * @brief 放弃使用键盘后释放硬盘缓冲区中的脏数据
 *
 * @param inode
 * @param filp
 * @return long
 */
s64_t keyboard_close(struct index_node *inode, struct file *filp)
{
    filp->private_data = NULL;

    p_kb->p_head = p_kb->buf;
    p_kb->p_tail = p_kb->buf;
    p_kb->count = 0;
    memset(p_kb->buf, 0, KB_BUF_SIZE);
    return 1;
}

/**
 * @brief ioctl操作方法用于控制设备功能的开启与关闭。keyboard_ioctl用于控制键盘设备。
 *          目前只有清空键盘缓冲区的功能。
 * @param inode
 * @param filp
 * @param cmd 命令 KEY_CMD_RESET_BUFFER
 * @param arg 执行命令函数的 参数
 * @return long
 */
#define KEY_CMD_RESET_BUFFER 1
s64_t keyboard_ioctl(struct index_node *inode, struct file *filp, u64_t cmd, u64_t arg)
{
    switch (cmd)
    {
    case KEY_CMD_RESET_BUFFER:
        p_kb->p_head = p_kb->buf;
        p_kb->p_tail = p_kb->buf;
        p_kb->count = 0;
        memset(p_kb, 0, KB_BUF_SIZE);
        break;
    default:
        break;
    }
    return 1;
}

s64_t keyboard_read(struct file *flip, char_t *buf, u64_t count, s64_t *position)
{
    s64_t counter = 0;      // 本次实际读取的字节数
    s64_t tail_end_gap = 0; // tail 到键盘缓冲区末尾的距离
    u8_t *tail = NULL;

    if (p_kb->count == 0)
        sleep_on(&keyboard_wait_queue);

    // 使用缓存区内实际字节数，计算本次读取的字节数
    counter = count > (p_kb->count) ? p_kb->count : count;
    tail = p_kb->p_tail;

    // p_kb.buf + BUF_SIZE 是buf数组越界后的第一个字节地址，所以应该-1,才是最后一个数据地址
    // tail 地址处本身有数据，所以应该 +1
    tail_end_gap = (p_kb->buf + KB_BUF_SIZE - 1) - tail + 1;

    if (counter <= tail_end_gap)
    {
        copy_to_user(tail, buf, counter);
        p_kb->p_tail += counter;
    }
    else
    {
        copy_to_user(tail, buf, tail_end_gap);
        copy_to_user(p_kb->buf, buf + tail_end_gap, counter - tail_end_gap);
        p_kb->p_tail = p_kb->buf + (counter - tail_end_gap);
    }

    p_kb->count -= count;
    return counter;
}

s64_t keyboard_write(struct file *flip, char *buf, u64_t count, long *position)
{
    return 0;
}

struct file_operations keyboard_fops = {
    .open = keyboard_open,
    .close = keyboard_close,
    .ioctl = keyboard_ioctl,
    .read = keyboard_read,
    .write = keyboard_write,
};

hw_int_controller keyboard_int_controller = {
    .enable = IOAPIC_enable,
    .disable = IOAPIC_disable,
    .install = IOAPIC_install,
    .uninstall = IOAPIC_uninstall,
    .ack = IOAPIC_edge_ack,
};

// 键盘中断处理函数
void keyboard_handler(u64_t nr, u64_t parameter, pt_regs_t *regs)
{
    u8_t x;
    x = io_in8(PORT_KB_DATA);

    if (p_kb->p_head == p_kb->buf + KB_BUF_SIZE)
        p_kb->p_head = p_kb->buf;

    *p_kb->p_head = x;
    p_kb->count++;
    p_kb->p_head++;

    wakeup(&keyboard_wait_queue, TASK_UNINTERRUPTIBLE);
}

// 键盘初始化函数(挂载函数)
void keyboard_init()
{
    struct IO_APIC_RET_entry entry;
    u64_t i, j;

    p_kb = (struct keyboard_inputbuffer *)knew(sizeof(struct keyboard_inputbuffer), 0);

    wait_queue_init(&keyboard_wait_queue, NULL);

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

    register_irq(0x21, &entry, &keyboard_handler, (u64_t)p_kb, &keyboard_int_controller, "ps/2 keyboard");
}

// sktest 驱动卸载函数 - 什么时候实现动态加载？
void keyboard_exit()
{
    unregister_irq(0x21);
    kdelete(p_kb, sizeof(keyboard_inputbuffer_t));
}
