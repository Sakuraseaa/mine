#ifndef _spinlock_h_
#define _spinlock_h_

// 本文件定义自旋锁，自旋锁用来防止多核侵略存储区
typedef struct
{
    __volatile__ u64_t lock;
} spinlock_t;

// 排队自旋锁 
typedef struct equity_spinlock_t{
    union {
        __volatile__ u32_t slock;//真正的锁值变量
        struct {
            volatile u16_t owner;
            volatile u16_t next;
        };
    };
}fair_spinlock_t;

#define RW_LOCK_BIAS     0x01000000
//读写锁的底层数据结构
typedef struct{
    unsigned int lock;
}rw_spinlock_t;

void spin_init(spinlock_t *lock);
void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);
// 尝试加锁 - 本系统并没有用到该函数
long spin_trylock(spinlock_t *lock);

void fair_spin_init(fair_spinlock_t* lock);
void fair_spin_lock(fair_spinlock_t* lock);
void fair_spin_unlock(fair_spinlock_t* lock);


#endif