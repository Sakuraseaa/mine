#ifndef _DISK_F_H_
#define _DISK_F_H_

void disk_init();
void disk_exit();
void write_handler(u64_t nr, u64_t parameter);
void read_handler(u64_t nr, u64_t parameter);
void other_handler(u64_t nr, u64_t parameter);
s64_t IDE_ioctl(s64_t cmd, s64_t arg);
block_buffer_node_t *make_request(s64_t cmd, u64_t blocks, s64_t count, u8_t *buffer);
void add_request(block_buffer_node_t *node);
s64_t cmd_out();
void end_request(block_buffer_node_t *node);

#endif // _DISK_F_H_