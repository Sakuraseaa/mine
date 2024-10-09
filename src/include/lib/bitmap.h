#ifndef __LIB_BITMAP_H_
#define __LIB_BITMAP_H_
#include "basetype.h"
#define BITMAP_MASK 1
#define BITMAP_DiGIT 8

typedef struct bitmap
{
    u64_t btmp_bytes_len;
    /* 在遍历位图时,整体上以字节为单位,细节上是以位为单位,所以此处位图的指针必须是单字节 */
    u8_t *bits;
}bitmap_t;

void bitmap_init(bitmap_t *btmp, u64_t bytes_len);
s64_t bitmap_scan(bitmap_t *btmp, u64_t cnt);
void bitmap_destory(bitmap_t *btmp);
void bitmap_set(bitmap_t *btmp, u64_t bit_idx, u8_t value);
void bitmap_make(bitmap_t *btmp, u8_t* data, u64_t bytes_len);
bool bitmap_scan_test(bitmap_t *btmp, u64_t bit_idx);
#endif