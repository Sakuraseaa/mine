#include "toolkit.h"
#include "devkit.h"
#include "arch_x86kit.h"
#include "kernelkit.h"

hw_int_controller serial_int_controller =
    {
        .enable = IOAPIC_enable,
        .disable = IOAPIC_disable,
        .install = IOAPIC_install,
        .uninstall = IOAPIC_uninstall,
        .ack = IOAPIC_edge_ack,
};
serial_t serials[2];

void recv_data(serial_t *serial)
{
    char_t ch = io_in8(serial->iobase);
    if (ch == '\r') // 特殊处理，回车键直接换行
    {
        ch = '\n';
    }

    if (serial->p_head == serial->rx_buf + SERIAL_BUF_LEN)
        serial->p_head = serial->rx_buf;

    *serial->p_head = ch;
    serial->count++;
    serial->p_head++;

}
void serial_handler(u64_t nr, u64_t parameter, pt_regs_t *regs) {
    
    nr = nr == 0x24 ? 0 : 1;

    serial_t *serial = &serials[nr];
    u8_t state = io_in8(serial->iobase + COM_LINE_STATUS);

    if (state & LSR_DR) // 数据可读
    {
        recv_data(serial);
    }

    // 先不编写串口的相关代码
}

s32_t serial_read(serial_t *serial, buf_t buf, u64_t count)
{
    return 0;
}

s32_t serial_write(serial_t *serial, buf_t buf, u64_t count) {
    s32_t nr = 0;
    while (nr < count)
    {
        u8_t state = io_in8(serial->iobase + COM_LINE_STATUS);
        if (state & LSR_THRE) // 如果串口可写
        {
            io_out8(serial->iobase, buf[nr++]);
            continue;
        }
    }
    return nr;
}

void serial_init()
{
    u8_t i = 0;
    for (; i < 2; i++)
    {
        serial_t *serial = &serials[i];

        if (!i) {
            register_irq(0x24, nullptr, serial_handler, 0, &serial_int_controller, "serial_1"); // 注册串口中断
            serial->iobase = COM1_IOBASE;
        } else {
            register_irq(0x23, nullptr, serial_handler, 0, &serial_int_controller, "serial_2");
            serial->iobase = COM2_IOBASE;
        }
        
        // 禁用中断
        io_out8(serial->iobase + COM_INTR_ENABLE, 0);

        // 激活 DLAB
        io_out8(serial->iobase + COM_LINE_CONTROL, 0x80);

        // 设置波特率因子 0x0030
        // 波特率为 115200 / 0x30 = 115200 / 48 = 2400
        io_out8(serial->iobase + COM_BAUD_LSB, 0x30);
        io_out8(serial->iobase + COM_BAUD_MSB, 0x00);

        // 复位 DLAB 位，数据位为 8 位
        io_out8(serial->iobase + COM_LINE_CONTROL, 0x03);

        // 启用 FIFO, 清空 FIFO, 14 字节触发电平
        io_out8(serial->iobase + COM_INTR_IDENTIFY, 0xC7);

        // 设置回环模式，测试串口芯片
        io_out8(serial->iobase + COM_MODEM_CONTROL, 0b11011);

        // 发送字节
        io_out8(serial->iobase, 0xAE);

        // 收到的内容与发送的不一致，则串口不可用
        if (io_in8(serial->iobase) != 0xAE)
        {
            continue;
        }

        // 设置回原来的模式
        io_out8(serial->iobase + COM_MODEM_CONTROL, 0b1011);
        // 0x0d = 0b1101

        io_out8(serial->iobase + COM_INTR_ENABLE, 0x0F);
    }
}
