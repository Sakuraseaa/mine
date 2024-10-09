#include "toolkit.h"

s64_t search_64rlbits(u64_t val)
{
    s64_t retbitnr = -1;
    __asm__ __volatile__(
        "bsrq %1,%q0 \t\n"
        : "+r"(retbitnr)
        : "rm"(val));
    return retbitnr + 1;
}

// verify: 核实， 检测数据是否越过用户层界限
long verify_area(u8_t *addr, u64_t size)
{
    return 1;
    // if (((u64_t)addr + size) <= (u64_t)0x00007fffffffffff)
    //     return 1;
    // else
    //     return 0;
}

long copy_from_user(void *from, void *to, u64_t size)
{
    u64_t d0, d1;
    if (!verify_area(from, size))
        return 0;
    // 输出描述符&: 不为任何输入操作表达式分配该寄存器
    __asm__ __volatile__("rep \n\t"
                         "movsq \n\t"
                         "movq %3, %0 \n\t"
                         "rep \n\t"
                         "movsb \n\t"
                         : "=&c"(size), "=&D"(d0), "=&S"(d1)
                         : "r"(size & 7), "0"(size / 8), "1"(to), "2"(from)
                         : "memory");

    return size;
}

long copy_to_user(void *from, void *to, u64_t size)
{
    if (!verify_area(to, size))
        return 0;
    // + 代表这个输出操作数，既是输出，也是输入
    // = 表示 这只是输出
    // & 表示输入部分的q,r,g等模糊约束，不能把变量约束到&修饰的寄存器
    // 输入部分的0, 表示使用第一个输出部分的寄存器，ecx，作为输入
    // "memory"表明asm语句修改的内存，但在输入输出部分没有显示出来，

    // 每一条 __asm__语句就像一个函数，输入部分是函数参数，输出部分是函数返回值
    // 输入部分指定，把传入的值 绑定到 寄存器
    // 输出部分指定，把寄存器处理好的值，保存到指定变量
    __asm__ __volatile__("rep \n\t"
                         "movsq \n\t"
                         "movq %3, %0 \n\t"
                         "rep \n\t"
                         "movsb \n\t"
                         : "=&c"(size), "+&D"(to), "+&S"(from)
                         : "r"(size & 7), "0"(size / 8)
                         : "memory");

    return size;
}

long strncpy_from_user(void *from, void *to, u64_t size)
{
    if (!verify_area(from, size))
        return 0;
    strncpy(to, from, size);
    return size;
}

/**
 * @brief 在内核态，判断用户字符串src的长度是否越界
 *
 * @return long 字符串长度。 如果字符串越界，返回的是 0
 */
long strnlen_user(void *src, u64_t maxlen)
{
    u64_t size = strlen(src);
    if (!verify_area(src, size))
        return 0;

    return size <= maxlen ? size : maxlen;
}

void list_init(struct List *list)
{
    list->prev = list;
    list->next = list;
}
/**
 * @brief 
 * 
 * @param list 
 * @param node 
 * @return true 找到了 则返回真
 * @return false 
 */
bool list_search(list_t *list, list_t *node) {
    
    list_t* temp = list->next;
    for(;temp != list; temp = temp->next) {
        if(temp == node)
            return true;
    }
    return false;
}
void list_add_to_behind(struct List *entry, struct List *pnew) ////add to entry behind
{
    pnew->next = entry->next;
    pnew->prev = entry;
    pnew->next->prev = pnew;
    entry->next = pnew;
}

void list_add_to_before(struct List *entry, struct List *pnew) ////add to entry before
{
    pnew->next = entry;
    entry->prev->next = pnew;
    pnew->prev = entry->prev;
    entry->prev = pnew;
}

void list_del(struct List *entry)
{
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
}

// return: 1 = 空 ， 0 = 不空
long list_is_empty(struct List *entry)
{
    if (entry == entry->next && entry->prev == entry)
        return 1;
    else
        return 0;
}

struct List *list_prev(struct List *entry)
{
    if (entry->prev != NULL)
        return entry->prev;
    else
        return NULL;
}

struct List *list_next(struct List *entry)
{
    if (entry->next != NULL)
        return entry->next;
    else
        return NULL;
}

bool_t list_is_empty_careful(const list_h_t *head)
{
	list_h_t *next = head->next;
	if (next == head && next == head->prev)
	{
		return TRUE;
	}
	return FALSE;
}

bool_t list_is_last(const list_h_t* list, const list_h_t* head)
{
	if(list->next == head)
	{
		return TRUE;
	}
	return FALSE;
}

bool_t list_is_first(const list_h_t* list, const list_h_t* head)
{
	if(list->prev == head)
	{
		return TRUE;
	}
	return FALSE;
}

void list_move(list_h_t *list, list_h_t *head)
{
	list_del(list);
	list_add(list, head);
	return;
}

void list_move_tail(list_h_t *list, list_h_t *head)
{
	list_del(list);
	list_add_tail(list, head);
	return;
}
/*
        From => To memory copy Num bytes
*/

void *memcpy(void *From, void *To, s64_t Num)
{
    s32_t d0, d1, d2;
    __asm__ __volatile__("cld	\n\t"
                         "rep	\n\t"
                         "movsq	\n\t"
                         "testb	$4,%b4	\n\t"
                         "je	1f	\n\t"
                         "movsl	\n\t"
                         "1:\ttestb	$2,%b4	\n\t"
                         "je	2f	\n\t"
                         "movsw	\n\t"
                         "2:\ttestb	$1,%b4	\n\t"
                         "je	3f	\n\t"
                         "movsb	\n\t"
                         "3:	\n\t"
                         : "=&c"(d0), "=&D"(d1), "=&S"(d2)
                         : "0"(Num / 8), "q"(Num), "1"(To), "2"(From)
                         : "memory");
    return To;
}

/*
        FirstPart = SecondPart		=>	 0
        FirstPart > SecondPart		=>	 1
        FirstPart < SecondPart		=>	-1
*/

s32_t memcmp(void *FirstPart, void *SecondPart, s64_t Count)
{
    register int __res;

    __asm__ __volatile__("cld	\n\t" // clean direct
                         "repe	\n\t" // repeat if equal
                         "cmpsb	\n\t"
                         "je	1f	\n\t"
                         "movl	$1,	%%eax	\n\t"
                         "jl	1f	\n\t"
                         "negl	%%eax	\n\t"
                         "1:	\n\t"
                         : "=a"(__res)
                         : "0"(0), "D"(FirstPart), "S"(SecondPart), "c"(Count)
                         :);
    return __res;
}

/*
        set memory at Address with C ,number is Count
*/

void *memset(void *Address, u8_t C, s64_t Count)
{
    s32_t d0, d1;
    u64_t tmp = C * 0x0101010101010101UL;
    __asm__ __volatile__("cld	\n\t"
                         "rep	\n\t"
                         "stosq	\n\t"
                         "testb	$4, %b3	\n\t"
                         "je	1f	\n\t"
                         "stosl	\n\t"
                         "1:\ttestb	$2, %b3	\n\t"
                         "je	2f\n\t"
                         "stosw	\n\t"
                         "2:\ttestb	$1, %b3	\n\t"
                         "je	3f	\n\t"
                         "stosb	\n\t"
                         "3:	\n\t"
                         : "=&c"(d0), "=&D"(d1)
                         : "a"(tmp), "q"(Count), "0"(Count / 8), "1"(Address)
                         : "memory");
    return Address;
}

/*
        string copy
*/

str_t strcpy(str_t Dest, cstr_t Src)
{
    __asm__ __volatile__("cld	\n\t"
                         "1:	\n\t"
                         "lodsb	\n\t"
                         "stosb	\n\t"
                         "testb	%%al,	%%al	\n\t"
                         "jne	1b	\n\t"
                         :
                         : "S"(Src), "D"(Dest)
                         :

    );
    return Dest;
}

/*
        string copy number bytes
*/

str_t strncpy(str_t Dest, str_t Src, long Count)
{
    __asm__ __volatile__("cld	\n\t"
                         "1:	\n\t"
                         "decq	%2	\n\t"
                         "js	2f	\n\t"
                         "lodsb	\n\t"
                         "stosb	\n\t"
                         "testb	%%al,	%%al	\n\t"
                         "jne	1b	\n\t"
                         "rep	\n\t"
                         "stosb	\n\t"
                         "2:	\n\t"
                         :
                         : "S"(Src), "D"(Dest), "c"(Count)
                         :);
    return Dest;
}

/*
        string cat Dest + Src
*/

str_t strcat(str_t Dest, str_t Src)
{
    __asm__ __volatile__("cld	\n\t"
                         "repne	\n\t"
                         "scasb	\n\t"
                         "decq	%1	\n\t"
                         "1:	\n\t"
                         "lodsb	\n\t"
                         "stosb	\n\r"
                         "testb	%%al,	%%al	\n\t"
                         "jne	1b	\n\t"
                         :
                         : "S"(Src), "D"(Dest), "a"(0), "c"(0xffffffff)
                         :);
    return Dest;
}

/*
        string compare FirstPart and SecondPart
        FirstPart = SecondPart =>  0
        FirstPart > SecondPart =>  1
        FirstPart < SecondPart => -1
*/

s32_t strcmp(str_t FirstPart, cstr_t SecondPart)
{
    register int __res;
    __asm__ __volatile__("cld	\n\t"
                         "1:	\n\t"
                         "lodsb	\n\t"
                         "scasb	\n\t"
                         "jne	2f	\n\t"
                         "testb	%%al,	%%al	\n\t"
                         "jne	1b	\n\t"
                         "xorl	%%eax,	%%eax	\n\t"
                         "jmp	3f	\n\t"
                         "2:	\n\t"
                         "movl	$1,	%%eax	\n\t"
                         "jl	3f	\n\t"
                         "negl	%%eax	\n\t"
                         "3:	\n\t"
                         : "=a"(__res)
                         : "D"(FirstPart), "S"(SecondPart)
                         :);
    return __res;
}

/*
        string compare FirstPart and SecondPart with Count Bytes
        FirstPart = SecondPart =>  0
        FirstPart > SecondPart =>  1
        FirstPart < SecondPart => -1
*/

s32_t strncmp(str_t FirstPart, str_t SecondPart, s64_t Count)
{
    register int __res;
    __asm__ __volatile__("cld	\n\t"
                         "1:	\n\t"
                         "decq	%3	\n\t"
                         "js	2f	\n\t"
                         "lodsb	\n\t"
                         "scasb	\n\t"
                         "jne	3f	\n\t"
                         "testb	%%al,	%%al	\n\t"
                         "jne	1b	\n\t"
                         "2:	\n\t"
                         "xorl	%%eax,	%%eax	\n\t"
                         "jmp	4f	\n\t"
                         "3:	\n\t"
                         "movl	$1,	%%eax	\n\t"
                         "jl	4f	\n\t"
                         "negl	%%eax	\n\t"
                         "4:	\n\t"
                         : "=a"(__res)
                         : "D"(FirstPart), "S"(SecondPart), "c"(Count)
                         :);
    return __res;
}

s32_t strlen(cstr_t String)
{
    register int __res;
    __asm__ __volatile__("cld	\n\t"
                         "repne	\n\t"
                         "scasb	\n\t"
                         "notl	%0	\n\t"
                         "decl	%0	\n\t"
                         : "=c"(__res)
                         : "D"(String), "a"(0), "0"(0xffffffff)
                         :);
    return __res;
}

/* 从左到右查找字符串str中首次出现字符ch的地址(不是下标,是地址) */
char_t *strchr(cstr_t str, cchar_t ch)
{
    while (*str != 0)
    {
        if (*str == ch)
        {
         return (char_t *)str; // 需要强制转化成和返回值类型一样,否则编译器会报const属性丢失,下同.
        }
        str++;
    }
    return nullptr;
}

/* 从后往前查找字符串str中首次出现字符ch的地址(不是下标,是地址) */
char_t *strrchr(cstr_t str, cchar_t ch)
{

    cstr_t last_char = nullptr;
    /* 从头到尾遍历一次,若存在ch字符,last_char总是该字符最后一次出现在串中的地址(不是下标,是地址)*/
    while (*str != 0)
    {
        if (*str == ch)
        {
            last_char = str;
        }
        str++;
    }
    return (char_t *)last_char;
}

/**
 * @brief 反向在string之中寻找，ch字符第一次出现的位置
 *
 * @param string 字符串
 * @param ch   字符
 * @param strlen  字符串长度
 * @return int 位置信息, ch不存在则返回 -1
 */
s64_t str_find_char(str_t string, char_t ch, s64_t strlen)
{
    s64_t ret = -1;
    s64_t i;
    for (i = strlen - 1; i >= 0; i--)
        if (string[i] == ch)
        {
            ret = i;
            break;
        }

    return ret;
}

void upper(str_t str)
{
    while (*str)
    {
        if (*str >= 'a' && *str <= 'z')
        {
            *str = *str - 32;
        }
        str++;
    }
}

void lower(str_t str)
{
    while (*str)
    {
        if (*str >= 'A' && *str <= 'Z')
        {
            *str = *str + 32;
        }
        str++;
    }
}

u64_t bit_set(u64_t *addr, u64_t nr)
{
    return *addr | (1UL << nr);
}

u64_t bit_get(u64_t *addr, u64_t nr)
{
    return *addr & (1UL << nr);
}

u64_t bit_clean(u64_t *addr, u64_t nr)
{
    return *addr & (~(1UL << nr));
}

u8_t io_in8(u16_t port)
{
    u8_t ret = 0;
    __asm__ __volatile__("inb	%%dx,	%0	\n\t"
                         "mfence			\n\t"
                         : "=a"(ret)
                         : "d"(port)
                         : "memory");
    return ret;
}

u32_t io_in32(u16_t port)
{
    u32_t ret = 0;
    __asm__ __volatile__("inl	%%dx,	%0	\n\t"
                         "mfence			\n\t"
                         : "=a"(ret)
                         : "d"(port)
                         : "memory");
    return ret;
}

void io_out8(u16_t port, u8_t value)
{
    __asm__ __volatile__("outb	%0,	%%dx	\n\t"
                         "mfence			\n\t"
                         :
                         : "a"(value), "d"(port)
                         : "memory");
}

void io_out32(u16_t port, u32_t value)
{
    __asm__ __volatile__("outl	%0,	%%dx	\n\t"
                         "mfence			\n\t"
                         :
                         : "a"(value), "d"(port)
                         : "memory");
}

u64_t rdmsr(u64_t address)
{
    u32_t tmp0 = 0;
    u32_t tmp1 = 0;
    __asm__ __volatile__("rdmsr	\n\t"
                         : "=d"(tmp0), "=a"(tmp1)
                         : "c"(address)
                         : "memory");
    return (u64_t)tmp0 << 32 | tmp1;
}

void wrmsr(u64_t address, u64_t value)
{
    __asm__ __volatile__("wrmsr	\n\t" ::"d"(value >> 32), "a"(value & 0xffffffff), "c"(address)
                         : "memory");
}

u64_t get_rsp()
{
    u64_t tmp = 0;
    __asm__ __volatile__("movq	%%rsp, %0	\n\t"
                         : "=r"(tmp)::"memory");
    return tmp;
}

u64_t get_rflags()
{
    u64_t tmp = 0;
    __asm__ __volatile__("pushfq	\n\t"
                         "movq	(%%rsp), %0	\n\t"
                         "popfq	\n\t"
                         : "=r"(tmp)::"memory");
    return tmp;
}
