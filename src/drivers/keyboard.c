#include "keyboard.h"
#include "lib.h"
#include "interrupt.h"
#include "APIC.h"
#include "memory.h"
#include "printk.h"
#include "VFS.h"
#include "waitqueue.h"
#include "semaphore.h"

// 保存着第一类键盘扫描码:PauseBreak键
unsigned char pausebreak_scode[] = {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5};
// 保存着第三类键盘扫描码: keycode_map_normal
unsigned int keycode_map_normal[NR_SCAN_CODES * MAP_COLS] = {
    /*scan-code	unShift		Shift		*/
    /*--------------------------------------------------------------*/
    /*0x00*/ 0,
    0,
    /*0x01*/ 0,
    0, // ESC
    /*0x02*/ '1',
    '!',
    /*0x03*/ '2',
    '@',
    /*0x04*/ '3',
    '#',
    /*0x05*/ '4',
    '$',
    /*0x06*/ '5',
    '%',
    /*0x07*/ '6',
    '^',
    /*0x08*/ '7',
    '&',
    /*0x09*/ '8',
    '*',
    /*0x0a*/ '9',
    '(',
    /*0x0b*/ '0',
    ')',
    /*0x0c*/ '-',
    '_',
    /*0x0d*/ '=',
    '+',
    /*0x0e*/ 0,
    0, // BACKSPACE
    /*0x0f*/ 0,
    0, // TAB

    /*0x10*/ 'q',
    'Q',
    /*0x11*/ 'w',
    'W',
    /*0x12*/ 'e',
    'E',
    /*0x13*/ 'r',
    'R',
    /*0x14*/ 't',
    'T',
    /*0x15*/ 'y',
    'Y',
    /*0x16*/ 'u',
    'U',
    /*0x17*/ 'i',
    'I',
    /*0x18*/ 'o',
    'O',
    /*0x19*/ 'p',
    'P',
    /*0x1a*/ '[',
    '{',
    /*0x1b*/ ']',
    '}',
    /*0x1c*/ 0,
    0, // ENTER
    /*0x1d*/ 0x1d,
    0x1d, // CTRL Left
    /*0x1e*/ 'a',
    'A',
    /*0x1f*/ 's',
    'S',

    /*0x20*/ 'd',
    'D',
    /*0x21*/ 'f',
    'F',
    /*0x22*/ 'g',
    'G',
    /*0x23*/ 'h',
    'H',
    /*0x24*/ 'j',
    'J',
    /*0x25*/ 'k',
    'K',
    /*0x26*/ 'l',
    'L',
    /*0x27*/ ';',
    ':',
    /*0x28*/ '\'',
    '"',
    /*0x29*/ '`',
    '~',
    /*0x2a*/ 0x2a,
    0x2a, // SHIFT Left
    /*0x2b*/ '\\',
    '|',
    /*0x2c*/ 'z',
    'Z',
    /*0x2d*/ 'x',
    'X',
    /*0x2e*/ 'c',
    'C',
    /*0x2f*/ 'v',
    'V',

    /*0x30*/ 'b',
    'B',
    /*0x31*/ 'n',
    'N',
    /*0x32*/ 'm',
    'M',
    /*0x33*/ ',',
    '<',
    /*0x34*/ '.',
    '>',
    /*0x35*/ '/',
    '?',
    /*0x36*/ 0x36,
    0x36, // SHIFT Right
    /*0x37*/ '*',
    '*',
    /*0x38*/ 0x38,
    0x38, // ALT Left
    /*0x39*/ ' ',
    ' ',
    /*0x3a*/ 0,
    0, // CAPS LOCK
    /*0x3b*/ 0,
    0, // F1
    /*0x3c*/ 0,
    0, // F2
    /*0x3d*/ 0,
    0, // F3
    /*0x3e*/ 0,
    0, // F4
    /*0x3f*/ 0,
    0, // F5

    /*0x40*/ 0,
    0, // F6
    /*0x41*/ 0,
    0, // F7
    /*0x42*/ 0,
    0, // F8
    /*0x43*/ 0,
    0, // F9
    /*0x44*/ 0,
    0, // F10
    /*0x45*/ 0,
    0, // NUM LOCK
    /*0x46*/ 0,
    0, // SCROLL LOCK
    /*0x47*/ '7',
    0, /*PAD HONE*/
    /*0x48*/ '8',
    0, /*PAD UP*/
    /*0x49*/ '9',
    0, /*PAD PAGEUP*/
    /*0x4a*/ '-',
    0, /*PAD MINUS*/
    /*0x4b*/ '4',
    0, /*PAD LEFT*/
    /*0x4c*/ '5',
    0, /*PAD MID*/
    /*0x4d*/ '6',
    0, /*PAD RIGHT*/
    /*0x4e*/ '+',
    0, /*PAD PLUS*/
    /*0x4f*/ '1',
    0, /*PAD END*/

    /*0x50*/ '2',
    0, /*PAD DOWN*/
    /*0x51*/ '3',
    0, /*PAD PAGEDOWN*/
    /*0x52*/ '0',
    0, /*PAD INS*/
    /*0x53*/ '.',
    0, /*PAD DOT*/
    /*0x54*/ 0,
    0,
    /*0x55*/ 0,
    0,
    /*0x56*/ 0,
    0,
    /*0x57*/ 0,
    0, // F11
    /*0x58*/ 0,
    0, // F12
    /*0x59*/ 0,
    0,
    /*0x5a*/ 0,
    0,
    /*0x5b*/ 0,
    0,
    /*0x5c*/ 0,
    0,
    /*0x5d*/ 0,
    0,
    /*0x5e*/ 0,
    0,
    /*0x5f*/ 0,
    0,
    /*0x60*/ 0,
    0,
    /*0x61*/ 0,
    0,
    /*0x62*/ 0,
    0,
    /*0x63*/ 0,
    0,
    /*0x64*/ 0,
    0,
    /*0x65*/ 0,
    0,
    /*0x66*/ 0,
    0,
    /*0x67*/ 0,
    0,
    /*0x68*/ 0,
    0,
    /*0x69*/ 0,
    0,
    /*0x6a*/ 0,
    0,
    /*0x6b*/ 0,
    0,
    /*0x6c*/ 0,
    0,
    /*0x6d*/ 0,
    0,
    /*0x6e*/ 0,
    0,
    /*0x6f*/ 0,
    0,

    /*0x70*/ 0,
    0,
    /*0x71*/ 0,
    0,
    /*0x72*/ 0,
    0,
    /*0x73*/ 0,
    0,
    /*0x74*/ 0,
    0,
    /*0x75*/ 0,
    0,
    /*0x76*/ 0,
    0,
    /*0x77*/ 0,
    0,
    /*0x78*/ 0,
    0,
    /*0x79*/ 0,
    0,
    /*0x7a*/ 0,
    0,
    /*0x7b*/ 0,
    0,
    /*0x7c*/ 0,
    0,
    /*0x7d*/ 0,
    0,
    /*0x7e*/ 0,
    0,
    /*0x7f*/ 0,
    0,
};

struct keyboard_inputbuffer *p_kb = NULL;
wait_queue_T keyboard_wait_queue; // 等待队列头
static int shift_l, shift_r, ctrl_l, ctrl_r, alt_l, alt_r;

/**
 * @brief 在操作前清空缓冲区
 *
 * @param inode
 * @param filp
 * @return long
 */
long keyboard_open(struct index_node *inode, struct file *filp)
{
    // 给文件描述符filp, 关联上硬盘数据缓冲区keyboard_inputbuffer
    filp->private_data = p_kb;

    p_kb->p_head = p_kb->buf;
    p_kb->p_head = p_kb->buf;
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
long keyboard_close(struct index_node *inode, struct file *filp)
{
    filp->private_data = NULL;

    p_kb->p_head = p_kb->buf;
    p_kb->p_head = p_kb->buf;
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
long keyboard_ioctl(struct index_node *inode, struct file *filp, unsigned long cmd, unsigned long arg)
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

long keyboard_read(struct file *flip, char *buf, unsigned long count, long *position)
{
    long counter = 0;      // 本次实际读取的字节数
    long tail_end_gap = 0; // tail 到键盘缓冲区末尾的距离
    unsigned char *tail = NULL;

    if (p_kb->count == 0)
        sleep_on(&keyboard_wait_queue);

    // 使用缓存区内实际字节数，计算本次读取的字节数
    counter = count > (p_kb->count) ? p_kb->count : count;
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

long keyboard_write(struct file *flip, char *buf, unsigned long count, long *position)
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
void keyboard_handler(unsigned long nr, unsigned long parameter, struct pt_regs *regs)
{
    unsigned char x;
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
    unsigned long i, j;

    p_kb = (struct keyboard_inputbuffer *)kmalloc(sizeof(struct keyboard_inputbuffer), 0);
    // 初始化键盘缓存区的队列
    p_kb->p_head = p_kb->buf;
    p_kb->p_tail = p_kb->buf;
    p_kb->count = 0;
    memset(p_kb->buf, 0, KB_BUF_SIZE);

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
        }
        if (!make) // 忽略断码
            key = 0;
    }

    if (key)
        color_printk(RED, BLACK, "%c", key);
}
