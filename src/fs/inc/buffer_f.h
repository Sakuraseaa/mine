#ifndef _BUFFER_F_H_
#define _BUFFER_F_H_

err_t bwrite(buffer_t *buf);
err_t brelse(buffer_t *buf);
buffer_t *bread(u64_t dev, u64_t block, u64_t size);
void buffer_init(void);

#endif // _BUFFER_F_H_