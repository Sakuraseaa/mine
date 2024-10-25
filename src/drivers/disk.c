#include "toolkit.h"
#include "devkit.h"
#include "arch_x86kit.h"
#include "mmkit.h"
#include "kernelkit.h"

static request_queue_t disk_request;
// 硬盘中断收尾函数，回收硬盘驱动程序为本次中断申请的资源
void end_request(block_buffer_node_t *node)
{
    if (node == nullptr)
        color_printk(RED, BLACK, "end_request error\n");

    // 把任务重新加入到进程就绪队列
    node->wait_queue.tsk->state = TASK_RUNNING;
    insert_task_queue(node->wait_queue.tsk);

    // 给当前进程需要调度标志，使得等待数据的进程抢占当前进程
    current->flags |= NEED_SCHEDULE;

    // 释放硬盘请求队列节点占用的内存
    kdelete(disk_request.in_using, sizeof(block_buffer_node_t));
    disk_request.in_using = nullptr;

    if (disk_request.block_request_count) // 硬盘中断请求队列不为空，则继续处理请求包
        cmd_out();
}

// 给硬盘发送命令
s64_t cmd_out()
{
    wait_queue_t *wait_queue_tmp =
        container_of(list_next(&disk_request.wait_queue_list.wait_list), wait_queue_t, wait_list);

    block_buffer_node_t *node = disk_request.in_using =
        container_of(wait_queue_tmp, block_buffer_node_t, wait_queue);

    // 从队列中，删除本节点
    list_del(&disk_request.in_using->wait_queue.wait_list);
    disk_request.block_request_count--;

    // 硬盘忙则等待
    while (io_in8(PORT_DISK1_STATUS_CMD) & DISK_STATUS_BUSY)
        nop();

    switch (node->cmd)
    {
    case ATA_WRITE_CMD:
        io_out8(PORT_DISK1_DEVICE, 0x40);
        // 写入48位LBA地址
        io_out8(PORT_DISK1_ERR_FEATURE, 0);
        io_out8(PORT_DISK1_SECTOR_CNT, (node->count >> 8) & 0xff);
        io_out8(PORT_DISK1_SECTOR_LOW, (node->LBA >> 24) & 0xff);
        io_out8(PORT_DISK1_SECTOR_MID, (node->LBA >> 32) & 0xff);
        io_out8(PORT_DISK1_SECTOR_HIGH, (node->LBA >> 40) & 0xff);

        io_out8(PORT_DISK1_ERR_FEATURE, 0);
        io_out8(PORT_DISK1_SECTOR_CNT, node->count & 0xff);
        io_out8(PORT_DISK1_SECTOR_LOW, node->LBA & 0xff);
        io_out8(PORT_DISK1_SECTOR_MID, (node->LBA >> 8) & 0xff);
        io_out8(PORT_DISK1_SECTOR_HIGH, (node->LBA >> 16) & 0xff);
        // 硬盘没准备好接收命令，则等待
        while (!(io_in8(PORT_DISK1_STATUS_CMD) & DISK_STATUS_READY))
            nop();
        // 发送写命令
        io_out8(PORT_DISK1_STATUS_CMD, node->cmd);
        // 硬盘没有准备好接受数据，则等待
        while (!(io_in8(PORT_DISK1_STATUS_CMD) & DISK_STATUS_REQ))
            nop();
        // 写512个字节给硬盘，这是要写入的数据
        port_outsw(PORT_DISK1_DATA, node->buffer, 256);
        break;
    case ATA_READ_CMD:
        io_out8(PORT_DISK1_DEVICE, 0x40);
        // 写入48位LBA地址
        io_out8(PORT_DISK1_ERR_FEATURE, 0);
        io_out8(PORT_DISK1_SECTOR_CNT, (node->count >> 8) & 0xff);
        io_out8(PORT_DISK1_SECTOR_LOW, (node->LBA >> 24) & 0xff);
        io_out8(PORT_DISK1_SECTOR_MID, (node->LBA >> 32) & 0xff);
        io_out8(PORT_DISK1_SECTOR_HIGH, (node->LBA >> 40) & 0xff);

        io_out8(PORT_DISK1_ERR_FEATURE, 0);
        io_out8(PORT_DISK1_SECTOR_CNT, node->count & 0xff);
        io_out8(PORT_DISK1_SECTOR_LOW, node->LBA & 0xff);
        io_out8(PORT_DISK1_SECTOR_MID, (node->LBA >> 8) & 0xff);
        io_out8(PORT_DISK1_SECTOR_HIGH, (node->LBA >> 16) & 0xff);
        // 硬盘没准备好接收命令，则等待
        while (!(io_in8(PORT_DISK1_STATUS_CMD) & DISK_STATUS_READY))
            nop();
        // 发送命令
        io_out8(PORT_DISK1_STATUS_CMD, node->cmd);
        break;
    case GET_IDENTIFY_DISK_CMD:
        io_out8(PORT_DISK1_DEVICE, 0xe0);

        io_out8(PORT_DISK1_ERR_FEATURE, 0);
        io_out8(PORT_DISK1_SECTOR_CNT, node->count & 0xff);
        io_out8(PORT_DISK1_SECTOR_LOW, node->LBA & 0xff);
        io_out8(PORT_DISK1_SECTOR_MID, (node->LBA >> 8) & 0xff);
        io_out8(PORT_DISK1_SECTOR_HIGH, (node->LBA >> 16) & 0xff);
        color_printk(ORANGE, WHITE, "test\n");
        // 硬盘没准备好接收命令，则等待
        while (!(io_in8(PORT_DISK1_STATUS_CMD) & DISK_STATUS_READY))
            nop();
        // 发送命令
        io_out8(PORT_DISK1_STATUS_CMD, node->cmd);
        color_printk(ORANGE, WHITE, "test\n");
        break;
    default:
        color_printk(BLACK, WHITE, "ATA CMD Error\n");
        break;
    }

    return 1;
}

/**
 * @brief 把硬盘读写命令封装成block_buffer_node包,
 *          block_buffer_node_t 描述了一次硬盘操作的全部信息
 * @param cmd       READ/Write
 * @param blocks  LBA地址
 * @param count   扇区数量
 * @param buffer  调用者传入的读写缓冲区
 * @return block_buffer_node_t*
 */
block_buffer_node_t *make_request(s64_t cmd, u64_t blocks, s64_t count, u8_t *buffer)
{
    block_buffer_node_t *node = (block_buffer_node_t *)knew(sizeof(block_buffer_node_t), 0);
    wait_queue_init(&node->wait_queue, current);

    switch (cmd)
    {
    case ATA_READ_CMD:
        node->cmd = ATA_READ_CMD;
        node->end_handler = read_handler;
        break;
    case ATA_WRITE_CMD:
        node->cmd = ATA_WRITE_CMD;
        node->end_handler = write_handler;
        break;
    default:
        node->end_handler = other_handler;
        node->cmd = cmd;
        break;
    }

    node->LBA = blocks;
    node->count = count; // 扇区数
    node->buffer = buffer;

    return node;
}

// 把请求包，加入到等待队列。这里也许可以添加一些操作硬盘的算法
void add_request(block_buffer_node_t *node)
{
    list_add_to_before(&disk_request.wait_queue_list.wait_list, &node->wait_queue.wait_list);
    disk_request.block_request_count++;
}

// 参见IDE_transfer- 本函数属于子函数
void submit(block_buffer_node_t *node)
{
    add_request(node);
    if (disk_request.in_using == nullptr) // 目前没有硬盘操作, 给硬盘发送命令
    {
        cmd_out();
    }
}

// 参见IDE_transfer- 本函数属于子函数
void wait_for_finish()
{
    current->state = TASK_UNINTERRUPTIBLE;
    schedule();
}

hw_int_controller disk_int_controller =
    {
        .enable = IOAPIC_enable,
        .disable = IOAPIC_disable,
        .install = IOAPIC_install,
        .uninstall = IOAPIC_uninstall,
        .ack = IOAPIC_edge_ack,
};

s64_t IDE_open()
{
    color_printk(BLACK, WHITE, "DISK0 Opened\n");
    return 1;
}

s64_t IDE_close()
{
    color_printk(BLACK, WHITE, "DISK0 Closed\n");
    return 1;
}

// 给硬盘发送除了读写外的命令，目前只实现了identify命令
s64_t IDE_ioctl(s64_t cmd, s64_t arg)
{

    block_buffer_node_t *node = nullptr;

    if (cmd == GET_IDENTIFY_DISK_CMD)
    {
        node = make_request(cmd, 0, 0, (u8_t *)arg);

        submit(node);
        wait_for_finish();
        return 1;
    }

    return 0;
}

/**
 * @brief 硬盘(块)读写函数
 *
 * @param cmd       读 or 写的命令
 * @param blocks    LBA地址，寻址硬盘的
 * @param count     请求的扇区数
 * @param buffer    要读写的缓存区
 * @return long     succeed return 1 OR Failed return 0
 */
s64_t IDE_transfer(s64_t cmd, u64_t blocks, s64_t count, u8_t *buffer)
{
    block_buffer_node_t *node = nullptr;
    if (cmd == ATA_READ_CMD || cmd == ATA_WRITE_CMD)
    {
        // a. 把 Read / Write 操作封装成请求包, 根据命令的不同指定不同的回调函数
        node = make_request(cmd, blocks, count, buffer);
        // b. 把操作请求项加入硬盘操作请求队列, 发送请求信息给硬盘控制器
        submit(node);
        // c. 挂起IO线程，等到硬盘操作完成，恢复进程调度
        wait_for_finish();
    }
    else
        return 0;

    return 1;
}

block_dev_opt_t IDE_device_operation = {
    .open = IDE_open,
    .close = IDE_close,
    .ioctl = IDE_ioctl,
    .transfer = IDE_transfer,
};

// do_IQR-函数会跳转到disk_handler
void disk_handler(u64_t nr, u64_t parameter, pt_regs_t *regs)
{
    block_buffer_node_t *node = ((request_queue_t *)parameter)->in_using;
    node->end_handler(nr, parameter);
}

void disk_init()
{
    /* Get the number of drives from the BIOS data area */
    u64_t *pNrDrives = (u64_t *)(0xffff800000000475);
    DEBUGK("NrDrives:%d.\n", *pNrDrives & 0xff);
    // color_printk(ORANGE, WHITE, "NrDrives:%d.\n", *pNrDrives & 0xff);
    /*在IO_APIC中，注册硬盘中断函数*/
    io_apic_ret_entry_t entry;
    // dev_t dev;

    // if(pNrDrives > 1){
    //     dev = device_install(DEV_BLOCK, DEV_IDE_DISK, 0, "hd_1", 0, &IDE_device_operation);
    // } else
    //     dev = device_install(DEV_BLOCK, DEV_IDE_DISK, 0, "hd_1", 0, &IDE_device_operation);
    
    entry.vector = 0x2f;
    entry.deliver_mode = APIC_ICR_IOAPIC_Fixed;
    entry.dest_mode = ICR_IOAPIC_DELV_PHYSICAL;
    entry.deliver_status = APIC_ICR_IOAPIC_Idle;
    entry.polarity = APIC_IOAPIC_POLARITY_HIGH;
    entry.irr = APIC_IOAPIC_IRR_RESET;
    entry.trigger = APIC_ICR_IOAPIC_Edge;
    entry.mask = APIC_ICR_IOAPIC_Masked;
    entry.reserved = 0;

    entry.destination.physical.reserved1 = 0;
    entry.destination.physical.phy_dest = 0;
    entry.destination.physical.reserved2 = 0;
    register_irq(entry.vector, &entry, &disk_handler, (u64_t)&disk_request, &disk_int_controller, "disk1");
    io_out8(PORT_DISK1_ALT_STA_CTL, 0);

    wait_queue_init(&disk_request.wait_queue_list, nullptr);
    disk_request.in_using = nullptr;
    disk_request.block_request_count = 0;
}

void disk_exit()
{
    unregister_irq(0x2f);
}

void read_handler(u64_t nr, u64_t parameter)
{
    block_buffer_node_t *node = ((request_queue_t *)parameter)->in_using;

    if (io_in8(PORT_DISK1_STATUS_CMD) & DISK_STATUS_ERROR) // 检测硬盘控制器，是否发生了错误
        color_printk(RED, BLACK, "read_handler:%#010x\n", io_in8(PORT_DISK1_ERR_FEATURE));
    else
        port_insw(PORT_DISK1_DATA, node->buffer, 256); // 硬盘操作成功，读取数据到缓存区

    // 在PIO模式下, 当使用控制命令一次性访问多个连续的扇区时，
    // 硬盘会在每个扇区操作结束后向处理器发送一个中断信号，以通知处理器为操作下一个扇区做准备
    // 每次中断递减操作的扇区数count, 递增缓冲区基址

    node->count--; // 要读的扇区数减少
    if (node->count)
    {
        node->buffer += 512;
        return;
    }

    end_request(node);
}

void write_handler(u64_t nr, u64_t parameter)
{
    block_buffer_node_t *node = ((request_queue_t *)parameter)->in_using;
    if (io_in8(PORT_DISK1_STATUS_CMD) & DISK_STATUS_ERROR) // 检测硬盘控制器，是否发生了错误
        color_printk(RED, BLACK, "write_handler:%#010x\n", io_in8(PORT_DISK1_ERR_FEATURE));

    // 写扇区, 递增缓冲区函数
    node->count--;
    if (node->count)
    {
        node->buffer += 512;
        while (!(io_in8(PORT_DISK1_STATUS_CMD) & DISK_STATUS_REQ))
            nop();
        port_outsw(PORT_DISK1_DATA, node->buffer, 256); // 再次给硬盘传送要写入的数据
        return;
    }

    end_request(node);
}

// identify IDE
void other_handler(u64_t nr, u64_t parameter)
{
    block_buffer_node_t *node = ((request_queue_t *)parameter)->in_using;
    if (io_in8(PORT_DISK1_STATUS_CMD) & DISK_STATUS_ERROR) // 检测硬盘控制器，是否发生了错误
        color_printk(RED, BLACK, "write_handler:%#010x\n", io_in8(PORT_DISK1_ERR_FEATURE));

    
    s32_t i = 0;
    struct Disk_Identify_Info a;
    u16_t *p = nullptr;
    port_insw(PORT_DISK1_DATA, &a, 256);

    color_printk(ORANGE, WHITE, "\nSerial Number:"); // 序列号
    for (i = 0; i < 10; i++)
        color_printk(ORANGE, WHITE, "%c%c", (a.Serial_Number[i] >> 8) & 0xff, a.Serial_Number[i] & 0xff);

    color_printk(ORANGE, WHITE, "\nFirmware revision:"); // 固件版本
    for (i = 0; i < 4; i++)
        color_printk(ORANGE, WHITE, "%c%c", (a.Firmware_Version[i] >> 8) & 0xff, a.Firmware_Version[i] & 0xff);

    color_printk(ORANGE, WHITE, "\nModel number:"); // 型号
    for (i = 0; i < 20; i++)
        color_printk(ORANGE, WHITE, "%c%c", (a.Model_Number[i] >> 8) & 0xff, a.Model_Number[i] & 0xff);
    color_printk(ORANGE, WHITE, "\n");

    p = (u16_t *)&a;
    // for (i = 0; i < 256; i++)
    //     color_printk(ORANGE, WHITE, "%04x ", *(p + i));
    // 是否支持LBA寻址
    s32_t capabilities = *(p + 49);
    color_printk(ORANGE, WHITE, "LBA supported: %s\n", (capabilities & 0x0200) ? "Yes" : "No");

    // 是否支持LBA48寻址
    s32_t cmd_set_supported = *(p + 83);
    color_printk(ORANGE, WHITE, "LBA48 supported: %s\n", (cmd_set_supported & 0x0400) ? "Yes" : "No");
    end_request(node);
}
