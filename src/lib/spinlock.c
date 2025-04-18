/**
 * @file spinlock.c
 * @author your name (you@domain.com)
 * @brief 这里的实现需要使用汇编代码进行重写
 *  读写锁没有进行测试
 * @version 0.1
 * @date 2024-07-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "kernelkit.h"
#include "preempt.h"
#include "spinlock.h"
#include "basetype.h"

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

void fair_spin_init(fair_spinlock_t* lock) { lock->slock = 0;}

void fair_spin_lock(fair_spinlock_t* lock) {
    preempt_disable();
    u32_t inc = 0x00010000;
    u32_t tmp;
    __asm__ __volatile__ (
        "lock; xaddl %0, %1 \n\t"   // 将 inc 和 slock 的值交换，然后 inc = inc + slock. 相当于原子读取next和owner并对 next + 1
        "movzwl %w0, %2     \n\t"   // 将inc的低16位做0扩展后送tmp tmp=(u16)inc = owner
        "shrl   $16, %0     \n\t"   // 将inc右移16位 inc = inc>>16 = next
        "1:                 \n\t"
        "cmpl  %0, %2       \n\t"   // 比较inc 和 tmp, 即比较 next 和 owner
        "je 2f              \n\t"   // 相等则跳转到标号2处返回
        "rep; nop           \n\t"   // 
        "movzwl %1, %2      \n\t"   // 将slock的低16位做0扩展后送tmp, tmp = slock.owner
        "jmp 1b             \n\t"
        "2:                 \n\t"   :"+Q"(inc), "+m"(lock->slock), "=r"(tmp)::"memory","cc");

}

void fair_spin_unlock(fair_spinlock_t* lock) {
    __asm__ __volatile__("lock; incw %0":"+m"(lock->slock)::"memory", "cc");
    preempt_enable();
}

void spin_init(spinlock_t *lock)
{
    lock->lock = 0;
}

void spin_lock(spinlock_t *lock)
{
    preempt_disable();
    __asm__ __volatile__(
        "1:         \n"
        "lock; xchg  %0, %1 \n"
        "cmpl   $0, %0      \n"
        "jnz    2f      \n"
        ".section .spinlock.text,"
        "\"ax\""
        "\n"                    // 重新定义一个代码段所以jnz 2f下面并不是
        "2:         \n"         //cmpl $0,%1 事实上下面的代码不会常常执行,
        "pause  \n"
        "cmpl   $0, %1      \n" //这是为了不在cpu指令高速缓存中填充无用代码
        "jne    2b      \n"     //要知道那可是用每位6颗晶体管做的双极性静态
        "jmp    1b      \n"     //储存器,比内存条快几千个数量级
        ".previous      \n"
        :
        : "r"(1), "m"(*lock));
}

// 解锁
void spin_unlock(spinlock_t *lock)
{
    __asm__ __volatile__("movq $0, %0\n\t":"=m"(lock->lock)::"memory");
    preempt_enable();
}

void spinlock_storeflg_cli(spinlock_t *lock, cpuflg_t *cpuflg)
{
    __asm__ __volatile__(
        "pushfq             \n\t"
        "cli                \n\t"
        "popq %0            \n\t"

        "1:                 \n\t"
        "lock; xchg  %1, %2 \n\t"
        "cmpl   $0,%1       \n\t"
        "jnz    2f          \n\t"
        ".section .spinlock.text,"
        "\"ax\""
        "\n\t"                    //重新定义一个代码段所以jnz 2f下面并不是
        "2:                 \n\t" //cmpl $0,%1 事实上下面的代码不会常常执行,
        "cmpl   $0,%2       \n\t" //这是为了不在cpu指令高速缓存中填充无用代码
        "jne    2b          \n\t"
        "jmp    1b          \n\t"
        ".previous          \n\t"
        : "=m"(*cpuflg)
        : "r"(1), "m"(*lock));
    return;
}

void spinunlock_restoreflg(spinlock_t *lock, cpuflg_t *cpuflg)
{
    __asm__ __volatile__(
        "movl   $0, %0\n\t"
        "pushq %1 \n\t"
        "popfq \n\t"
        :
        : "m"(*lock), "m"(*cpuflg));
    return;
}


// 尝试加锁 - 本系统并没有用到该函数
s64_t spin_trylock(spinlock_t *lock)
{
    u64_t tmp_value = 0;
    preempt_disable();
    __asm__ __volatile__(
        "xchgq	%0,	%1	\n\t"
        : "=q"(tmp_value), "=m"(lock->lock)
        : "0"(0)
        : "memory");
    if (!tmp_value)
        preempt_enable();
    return tmp_value;
}


/**
 * @brief 此处的读写锁，如果在加写锁时，一直有读锁的申请，会造成写锁饥饿
 *        改善写法是，有线程尝试加写锁的时候，终止读锁的申请。 可以使用一个原子变量完成此操作
 */
//释放读锁 
void read_unlock(rw_spinlock_t*rw){ 
    __asm__ __volatile__(
        "incl %0        \n\t" //原子对lock加1
        :"+m"(rw->lock)::"memory");
}

//释放写锁
void write_unlock(rw_spinlock_t*rw){
    __asm__ __volatile__(
        "addl %1, %0    \n\t" //原子对lock加上RW_LOCK_BIAS
        :"+m"(rw->lock):"i"(RW_LOCK_BIAS):"memory");
}

//获取读锁
void read_lock(rw_spinlock_t*rw){
    __asm__ __volatile__(
        "subl $1, %0    \n\t" //原子对lock减1
        "jns 1f         \n\t" //不为小于0则跳转标号1处，表示获取读锁成功
        "2:             \n\t"
        "lock; incl %0  \n\t" //原子加1
        "3:             \n\t"
        "pause          \n\t" //空指令
        "cmpl  $1, %0   \n\t" //和1比较 小于0则
        "js 3b          \n\t" //为负则继续循环比较
        "lock; decl %0  \n\t" //加读锁
        "js 2b          \n\t" //为负则继续加1并比较，否则返回
        "1:             \n\t"::"m"(rw):"memory");
}
//获取写锁
void write_lock(rw_spinlock_t*rw){
    __asm__ __volatile__(
        "subl %1, %0    \n\t" //原子对lock减去RW_LOCK_BIAS
        "jz 1f          \n\t" //为0则跳转标号1处
        "2:             \n\t" 
        "addl %1, %0    \n\t"
        "1:             \n\t"
        "pause          \n\t"//空指令
        "cmpl %1, %0    \n\t" //不等于初始值则循环比较，相等则表示有进程释放了写锁
        "jne 1b         \n\t"
        "subl %1, %0    \n\t" //加写锁
        "jnz 2b         \n\t" //不为0则继续测试，为0则表示加写锁成功
        "1:             \n\t"
        ::"m"(rw),"i"(RW_LOCK_BIAS):"memory");
}
