#ifndef _BITMAP_F_H_
#define _BITMAP_F_H_

s64_t bitmap_scan(bitmap_t *btmp, u64_t cnt);
void bitmap_destory(bitmap_t *btmp);
void bitmap_set(bitmap_t *btmp, u64_t bit_idx, u8_t value);
void bitmap_make(bitmap_t *btmp, u8_t* data, u64_t bytes_len);
bool bitmap_scan_test(bitmap_t *btmp, u64_t bit_idx);

#endif // _BITMAP_F_H_