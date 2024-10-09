#ifndef _SERIAL_F_H_
#define _SERIAL_F_H_

int serial_read(serial_t *serial, char *buf, u64_t count);
int serial_write(serial_t *serial, char *buf, u64_t count);
void serial_init();

#endif // _SERIAL.F_H_