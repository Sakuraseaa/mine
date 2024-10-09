#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#define KB_BUF_SIZE 100
typedef struct keyboard_inputbuffer
{
    unsigned char *p_head; // 缓冲区首尾指针
    unsigned char *p_tail;
    int count; // 缓冲数据计数器

    unsigned char buf[KB_BUF_SIZE]; // 循环队列缓存区
}keyboard_inputbuffer_t;

#define PAUSEBREAK 1
#define PRINTSCREEN 2
#define OTHERKEY 4
#define FLAG_BREAK 0x80

#define PORT_KB_DATA 0x60   // 8042的数据寄存器端口
#define PORT_KB_STATUS 0x64 // 读取8042状态的端口
#define PORT_KB_CMD 0x64    // 给8042发送命令的端口

#define KBSTATUS_IBF 0x02 // bit1 :说明键盘输入缓存区已满
#define KBSTATUS_OBF 0x01 // bit0: 说明键盘输出缓存区已满

#define KBCMD_WRITE_CMD 0x60 // 向键盘发送配置命令， KB_INIT_MODE是命令参数
#define KBCMD_READ_CMD 0x20  // 读取键盘的配置值

#define KB_INIT_MODE 0x47 // 给8042芯片的配置， 使能鼠标，键盘。使能鼠标中断IRQ12(MIBF), 使能键盘中断IRQ1(IBF)

// 检测缓存区能不能写
#define wait_KB_write() while (io_in8(PORT_KB_STATUS) & KBSTATUS_IBF)
// 检测缓存区能不能读
#define wait_KB_read() while (io_in8(PORT_KB_STATUS) & KBSTATUS_OBF)

extern struct file_operations keyboard_fops;

#endif