#include "memory.h"
#include "lib.h"
#include "basetype.h"
#include "bitmap.h"
#include "assert.h"

/**
 * @brief 判断位图中指定的位是否位1
 *
 * @param btmp 指向bitmap的指针
 * @param bit_idx 需要判断的位
 * @return true 1
 * @return false 0
 */
bool bitmap_scan_test(bitmap_t *btmp, u64_t bit_idx) {
    
    u64_t byte_idx = bit_idx / BITMAP_DiGIT;
    u64_t bit_odd = bit_idx % BITMAP_DiGIT;
    // return (btmp->bits[byte_idx]) & (BITMAP_MASK << bit_odd);
    return btmp->bits[byte_idx] >> (bit_odd);

}

void bitmap_init(bitmap_t *btmp, u64_t bytes_len) {
    btmp->btmp_bytes_len = bytes_len;
    btmp->bits = knew(bytes_len, 0);
    memset(btmp->bits, 0, bytes_len);
}

void bitmap_destory(bitmap_t *btmp) {
    kdelete(btmp->bits, btmp->btmp_bytes_len);
}

void bitmap_make(bitmap_t *btmp, u8_t* data, u64_t bytes_len) {
    btmp->bits = data;
    btmp->btmp_bytes_len = bytes_len;
}

/**
 * @brief 将bitmap中的bit_idx位设置为value
 *
 * @param btmp 指向bitmap的指针
 * @param bit_idx 要设置的位的索引
 * @param value 要设置的值
 */
void bitmap_set(bitmap_t *btmp, u64_t bit_idx, u8_t value) {
    assert(bit_idx < btmp->btmp_bytes_len * BITMAP_DiGIT)
    if(value == 0)
        btmp->bits[bit_idx / BITMAP_DiGIT]  &= ~(1UL << (bit_idx % BITMAP_DiGIT)); // 设置为0
    else
        btmp->bits[bit_idx / BITMAP_DiGIT]  |= (1UL << (bit_idx % BITMAP_DiGIT)); // 设置为1
}



/**
 * @brief 在位图中申请连续的cnt个位，若成功则返回起始位的下标，失败则返回-1
 *
 * @param btmp 指向bitmap的指针
 * @param cnt 连续的位数
 * @return int 起始位的下标
 */
s64_t bitmap_scan(bitmap_t *btmp, u64_t cnt) {
    
    u64_t i_bytes = 0, i_bit = 0;
    
    assert(cnt >= 0);
    
    // 按字节步进，搜索空闲位
    while((btmp->bits[i_bytes] == 0xff) && (i_bytes < btmp->btmp_bytes_len))
        i_bytes++;
    if(i_bytes == btmp->btmp_bytes_len) // 已经没有空闲位可用，返回-1
        return -1;

    i_bit = 0;
    while ((u8_t)(BITMAP_MASK << i_bit) & btmp->bits[i_bytes])
        i_bit++;
    
    if(cnt == 1) // 如果只申请一个页则返回
        return i_bytes * BITMAP_DiGIT+ i_bit;
    
                
    u64_t all_byte = btmp->btmp_bytes_len * 8; // 位图剩余位
    s64_t count = 1, bit_next = i_bytes * BITMAP_DiGIT + i_bit + 1;                

    while(bit_next <= all_byte) {
        if(bitmap_scan_test(btmp, bit_next))
            count = 1;
        else
            count++; // 是空闲位 增加空闲位计数
        
        if(count == cnt)
            break;

        bit_next++;
    }

    if(bit_next > all_byte) return -1;

    return bit_next - cnt + 1;
}
