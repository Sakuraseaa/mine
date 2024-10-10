#ifndef __LIB_BITMAP_H_
#define __LIB_BITMAP_H_
#define BITMAP_MASK 1
#define BITMAP_DiGIT 8

typedef struct bitmap
{
    u64_t btmp_bytes_len;
    /* 在遍历位图时,整体上以字节为单位,细节上是以位为单位,所以此处位图的指针必须是单字节 */
    u8_t *bits;
}bitmap_t;

#endif