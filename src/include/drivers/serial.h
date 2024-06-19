#ifndef _SERIAL_H__
#define _SERIAL_H__

#define COM1_IOBASE 0x3F8 /// 串口 1 基地址
#define COM2_IOBASE 0x2F8 /// 串口 2 基地址

#define COM_DATA 0          // 数据寄存器
#define COM_INTR_ENABLE 1   // 中断允许
#define COM_BAUD_LSB 0      // 波特率低字节
#define COM_BAUD_MSB 1      // 波特率高字节
#define COM_INTR_IDENTIFY 2 // 中断识别
#define COM_LINE_CONTROL 3  // 线控制
#define COM_MODEM_CONTROL 4 // 调制解调器控制
#define COM_LINE_STATUS 5   // 线状态
#define COM_MODEM_STATUS 6  // 调制解调器状态

// 线状态
#define LSR_DR 0x1
#define LSR_OE 0x2
#define LSR_PE 0x4
#define LSR_FE 0x8
#define LSR_BI 0x10
#define LSR_THRE 0x20
#define LSR_TEMT 0x40
#define LSR_IE 0x80

#define SERIAL_BUF_LEN 64

typedef struct serial_t
{
    short iobase;          // 端口号基地址

    unsigned char *p_head; // 缓冲区首尾指针
    unsigned char *p_tail;
    int count; // 缓冲数据计数器

    unsigned char rx_buf[SERIAL_BUF_LEN];  // 读 缓冲
} serial_t;

int serial_read(serial_t *serial, char *buf, unsigned long count);
int serial_write(serial_t *serial, char *buf, unsigned long count);
void serial_init();
#endif