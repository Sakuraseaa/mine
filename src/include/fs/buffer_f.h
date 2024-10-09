#ifndef _BUFFER_F_H_
#define _BUFFER_F_H_

err_t bwrite(buffer_t *buf);
err_t brelse(buffer_t *buf);
buffer_t *bread(unsigned long dev, unsigned long block, unsigned long size);
void buffer_init(void);

#endif // _BUFFER_F_H_