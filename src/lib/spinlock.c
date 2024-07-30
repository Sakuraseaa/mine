#include "task.h"
#include "preempt.h"
#include "spinlock.h"
#include "types.h"

/**
 * @brief 保存当前中断状态 并 关闭中断
 * 
 * @param flags 保存中断状态的地址。 
 */
void save_flags_cli(cpuflg_t* flags)
{
    __asm__ __volatile__("pushfq	\n\t"   //把eflags寄存器压入当前栈顶
                        "cli \n\t"          //关闭中断
                        "popq %0\n\t"       //把当前栈顶弹出到flags为地址的内存中
                        :"=m"(*flags):: "memory");
}

/**
 * @brief 回复eflags寄存器的值
 * 
 * @param flags 
 */
void restore_flags(cpuflg_t* flags)
{
    __asm__ __volatile__("pushq %0\n\t" // 把flags为地址处的值寄存器压入当前栈顶
                         "popfq \n\t"   // 把当前栈顶弹出到eflags寄存器中
                         ::"m"(*flags) : "memory");
}

void fair_spin_init(fair_spinlock_t* lock) { lock->slock = 1;}

// 公平的自旋锁，加锁的对象会排成一个有序队列
void fair_spin_lock(fair_spinlock_t* lock) {
    u32 inc = 0x00010000UL;
    u32 tmp;
__asm__ __volatile__ ("lock; xaddl %0, %1 \n\t" // 将 inc 和 slock 的值交换，然后 slock = inc + slock. 相当于原子读取next和owner并对 next + 1
                    "movzwl %w0, %2     \n\t"   // 将inc的低16位做0扩展后送tmp tmp=(u16)inc = owner
                    "shrl   $16, %0      \n\t"  // 将inc右移16位 inc = inc>>16 = next
                    "1: \n\t"
                    "cmpl  %0, %2       \n\t"   // 比较inc 和 tmp, 即比较 next 和 owner
                    "je 2f              \n\t"   // 相等则跳转到标号2处返回
                    "rep; nop           \n\t"   // 
                    "movzwl %1, %2      \n\t"   //将slock的低16位做0扩展后送tmp, tmp = slock.owner
                    "jmp 1b             \n\t"
                    "2:                 \n\t"
                    :"+m"(inc), "+m"(lock->slock), "=r"(tmp)::"memory","cc");

}

void fair_spin_lock(fair_spinlock_t* lock) {
__asm__ __volatile__("lock; incw %0":"+m"(lock->slock)::"memory", "cc");
}

void spin_init(spinlock_T *lock)
{
    lock->lock = 1;
}

void spin_lock(spinlock_T *lock)
{
    preempt_disable();
    __asm__ __volatile__("1: \n\t"
                         "lock decq %0 \n\t" // --lock
                         "jns 3f       \n\t" // 结果为正则跳转
                         "2:           \n\t"
                         "pause        \n\t" // pause是一个空转指令，于nop指令相似，但pause更低耗
                         "cmpq $0, %0  \n\t"
                         "jle 2b       \n\t" // lock小于等于0，则跳转到2
                         "jmp 1b       \n\t"
                         "3:           \n\t"
                         : "=m"(lock->lock)::"memory");
}

// 解锁
void spin_unlock(spinlock_T *lock)
{
    __asm__ __volatile__("movq $1, %0\n\t"
                         : "=m"(lock->lock)::"memory");
    preempt_enable();
}


// 尝试加锁 - 本系统并没有用到该函数
long spin_trylock(spinlock_T *lock)
{
    unsigned long tmp_value = 0;
    preempt_disable();
    __asm__ __volatile__("xchgq	%0,	%1	\n\t"
                         : "=q"(tmp_value), "=m"(lock->lock)
                         : "0"(0)
                         : "memory");
    if (!tmp_value)
        preempt_enable();
    return tmp_value;
}

