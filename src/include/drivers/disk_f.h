#ifndef _DISK_F_H_
#define _DISK_F_H_

void disk_init();
void disk_exit();
void write_handler(unsigned long nr, unsigned long parameter);
void read_handler(unsigned long nr, unsigned long parameter);
void other_handler(unsigned long nr, unsigned long parameter);
long IDE_ioctl(long cmd, long arg);
struct block_buffer_node *make_request(long cmd, unsigned long blocks, long count, unsigned char *buffer);
void add_request(struct block_buffer_node *node);
long cmd_out();
void end_request(struct block_buffer_node *node);

#endif // _DISK_F_H_