#ifndef _ATOMIC_H__
#define _ATOMIC_H__

typedef struct
{
    __volatile__ s64_t value;
} atomic_t;

typedef atomic_t refcount_t;

#define atomic_read(atomic) ((atomic)->value)
#define atomic_set(atomic, val) (((atomic)->value) = (val))

// ++
static inline void atomic_inc(atomic_t *atomic)
{
    // lock指令 使得处理器在执行主体指令时会锁住硬件系统平台的前端总线，从而防止其他处理器访问物理内存
    __asm__ __volatile__("lock incq %0 \n\t"
                         : "=m"(atomic->value)
                         : "m"(atomic->value)
                         : "memory");
}

// --
static inline void atomic_dec(atomic_t *atomic)
{
    __asm__ __volatile__("lock decq %0 \n\t"
                         : "=m"(atomic->value)
                         : "m"(atomic->value)
                         : "memory");
}

// a = a - b
static inline void atomic_sub(atomic_t *atomic, s64_t value)
{
    __asm__ __volatile__("lock subq %1, %0 \n\t"
                         : "=m"(atomic->value)
                         : "r"(value)
                         : "memory");
}

// a = a + b
static inline void atomic_add(atomic_t *atomic, s64_t value)
{
    __asm__ __volatile__("lock addq %1, %0 \n\t"
                         : "=m"(atomic->value)
                         : "r"(value)
                         : "memory");
}

// 设置某位为1
static inline void atomic_set_mask(atomic_t *atomic, s64_t mask)
{
    __asm__ __volatile__("lock orq %1, %0\n\t"
                         : "=m"(atomic->value)
                         : "r"(mask)
                         : "memory");
}

// 还原某位为0
static inline void atomic_clear_mask(atomic_t *atomic, s64_t mask)
{
    __asm__ __volatile__("lock andq %1, %0\n\t"
                         : "=m"(atomic->value)
                         : "r"(mask)
                         : "memory");
}

#endif
