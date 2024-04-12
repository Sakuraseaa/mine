#ifndef _ATOMIC_H__
#define _ATOMIC_H__

typedef struct
{
    __volatile__ long value;
} atomic_T;

#define atomic_read(atomic) ((atomic)->value)
#define atomic_set(atomic, val) (((atomic)->value) = (val))

// ++
static inline void atomic_inc(atomic_T *atomic)
{
    __asm__ __volatile__("lock incq %0 \n\t"
                         : "=m"(atomic->value)
                         : "m"(atomic->value)
                         : "memory");
}

// --
static inline void atomic_dec(atomic_T *atomic)
{
    __asm__ __volatile__("lock decq %0 \n\t"
                         : "=m"(atomic->value)
                         : "m"(atomic->value)
                         : "memory");
}

// a = a - b
static inline void atomic_sub(atomic_T *atomic, long value)
{
    __asm__ __volatile__("lock subq %1, %0 \n\t"
                         : "=m"(atomic->value)
                         : "r"(value)
                         : "memory");
}

// a = a + b
static inline void atomic_add(atomic_T *atomic, long value)
{
    __asm__ __volatile__("lock addq %1, %0 \n\t"
                         : "=m"(atomic->value)
                         : "r"(value)
                         : "memory");
}

// 设置某位为1
static inline void atomic_set_mask(atomic_T *atomic, long mask)
{
    __asm__ __volatile__("lock orq %1, %0\n\t"
                         : "=m"(atomic->value)
                         : "r"(mask)
                         : "memory");
}

// 还原某位为0
static inline void atomic_clear_mask(atomic_T *atomic, long mask)
{
    __asm__ __volatile__("lock andq %1, %0\n\t"
                         : "=m"(atomic->value)
                         : "r"(mask)
                         : "memory");
}

#endif
