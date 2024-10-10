#ifndef _KEYBOARD_F_H_
#define _KEYBOARD_F_H_

void keyboard_init();
void keyboard_exit();
s64_t keyboard_open(inode_t *inode, file_t *filp);
s64_t keyboard_close(inode_t *inode, file_t *filp);
s64_t keyboard_ioctl(inode_t *inode, file_t *filp, u64_t cmd, u64_t arg);
s64_t keyboard_read(file_t *flip, buf_t buf, u64_t count, s64_t *position);
s64_t keyboard_write(file_t *flip, buf_t buf, u64_t count, s64_t *position);

#endif // _KEYBOARD_F_H_