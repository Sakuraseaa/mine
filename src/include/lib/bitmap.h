#ifndef __LIB_BITMAP_H_
#define __LIB_BITMAP_H_
#include "types.h"
#define BITMAP_MASK 1
typedef struct bitmap
{
    u64 btmp_bytes_len;
    /* 在遍历位图时,整体上以字节为单位,细节上是以位为单位,所以此处位图的指针必须是单字节 */
    u8 *bits;
}bitmap_t;

void bitmap_init(bitmap_t *btmp, u64 bytes_len);
int bitmap_scan(bitmap_t *btmp, u64 cnt);
void bitmap_set(bitmap_t *btmp, u64 bit_idx, u8 value);
#endif