#ifndef _SERIAL_F_H_
#define _SERIAL_F_H_

s32_t serial_read(serial_t *serial, buf_t buf, u64_t count);
s32_t serial_write(serial_t *serial, buf_t buf, u64_t count);
void serial_init();

#endif // _SERIAL.F_H_