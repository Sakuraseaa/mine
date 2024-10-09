#ifndef _KEYBOARD_F_H_
#define _KEYBOARD_F_H_

void keyboard_init();
void keyboard_exit();
long keyboard_open(struct index_node *inode, struct file *filp);
long keyboard_close(struct index_node *inode, struct file *filp);
long keyboard_ioctl(struct index_node *inode, struct file *filp, unsigned long cmd, unsigned long arg);
long keyboard_read(struct file *flip, char *buf, unsigned long count, long *position);
long keyboard_write(struct file *flip, char *buf, unsigned long count, long *position);

#endif // _KEYBOARD_F_H_