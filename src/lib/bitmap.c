#include "memory.h"
#include "lib.h"
#include "types.h"
#include "bitmap.h"

/**
 * @brief 判断位图中指定的位是否位1
 *
 * @param btmp 指向bitmap的指针
 * @param bit_idx 需要判断的位
 * @return true 1
 * @return false 0
 */
static bool bitmap_scan_test(bitmap_t *btmp, u64 bit_idx) {
    
    u64 byte_idx = bit_idx / 8;
    u64 bit_odd = bit_idx % 8;
    // return (btmp->bits[byte_idx]) & (BITMAP_MASK << bit_odd);
    return btmp->bits[byte_idx] >> (bit_odd);

}

void bitmap_init(bitmap_t *btmp, u64 bytes_len) {
    btmp->btmp_bytes_len = bytes_len;
    btmp->bits = kmalloc(bytes_len, 0);
    memset(btmp->bits, 0, bytes_len);
}

/**
 * @brief 在位图中申请连续的cnt个位，若成功则返回起始位的下标，失败则返回-1
 *
 * @param btmp 指向bitmap的指针
 * @param cnt 连续的位数
 * @return int 起始位的下标
 */
int bitmap_scan(bitmap_t *btmp, u64 cnt) {
    int i = 0;
    while(btmp->bits[i] == 0xff)
        i++;
}

/**
 * @brief 将bitmap中的bit_idx位设置为value
 *
 * @param btmp 指向bitmap的指针
 * @param bit_idx 要设置的位的索引
 * @param value 要设置的值
 */
void bitmap_set(bitmap_t *btmp, u64 bit_idx, u8 value) {

}
