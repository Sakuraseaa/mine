#ifndef __LIB_H__
#define __LIB_H__

#include "basetype.h"
#include "basetype.h"

#define container_of(ptr, type, member)                                     \
	({                                                                      \
		typeof(((type *)0)->member) *p = (ptr);                             \
		(type *)((unsigned long)p - (unsigned long)&(((type *)0)->member)); \
	})
#define hlt() __asm__ __volatile__("hlt \n\t" ::: "memory")

// 允许中断发生
#define sti() __asm__ __volatile__("sti	\n\t" ::: "memory")

// clear interrupt 禁止中断发生
#define cli() __asm__ __volatile__("cli	\n\t" ::: "memory")

#define nop() __asm__ __volatile__("nop	\n\t")
// 保证系统在后面的memory访问之前，先前的memory访问都以及结束
#define io_mfence() __asm__ __volatile__("mfence	\n\t" ::: "memory")

typedef struct List
{
	struct List *prev;
	struct List *next;
}list_t;

typedef list_t list_h_t;
typedef list_t list_n_t;

void list_init(struct List *list);

void list_add_to_behind(struct List *entry, struct List *pnew); ////add to entry behind

void list_add_to_before(struct List *entry, struct List *pnew); ////add to entry before

#define list_add(x, y) list_add_to_before((x), (y)) 			// 在链表头的前面添加 == 在整条链表后添加一个元素
#define list_add_tail(x, y) list_add_to_behind((x), (y))		// 在链表头的后面添加 == 在整条链表前插入一个元素
bool_t list_is_last(const list_h_t* list, const list_h_t* head);
bool_t list_is_first(const list_h_t* list, const list_h_t* head);
void list_move(list_h_t *list, list_h_t *head);
void list_move_tail(list_h_t *list, list_h_t *head);
bool_t list_is_empty_careful(const list_h_t *head);

void list_del(struct List *entry);
bool list_search(list_t *list, list_t *node);
// return: 1 = 空 ， 0 = 不空
long list_is_empty(struct List *entry);

struct List *list_prev(struct List *entry);
struct List *list_next(struct List *entry);


#define SEEK_SET 0 /* Seek relative to start-of-file */
#define SEEK_CUR 1 /* Seek relative to current position */
#define SEEK_END 2 /* Seek relative to end-of-file */

#define SEEK_MAX 3

/*
		From => To memory copy Num bytes
*/
void *memcpy(void *From, void *To, long Num);

/*
		FirstPart = SecondPart		=>	 0
		FirstPart > SecondPart		=>	 1
		FirstPart < SecondPart		=>	-1
*/
int memcmp(void *FirstPart, void *SecondPart, long Count);
/*
		set memory at Address with C ,number is Count
*/
void *memset(void *Address, unsigned char C, long Count);

/*
		string copy
*/
char *strcpy(char *Dest,const char *Src);

/*
		string copy number bytes
*/
char *strncpy(char *Dest, char *Src, long Count);

/*
		string cat Dest + Src
*/
char *strcat(char *Dest, char *Src);

/*
		string compare FirstPart and SecondPart
		FirstPart = SecondPart =>  0
		FirstPart > SecondPart =>  1
		FirstPart < SecondPart => -1
*/
int strcmp(char *FirstPart,const char *SecondPart);

/*
		string compare FirstPart and SecondPart with Count Bytes
		FirstPart = SecondPart =>  0
		FirstPart > SecondPart =>  1
		FirstPart < SecondPart => -1
*/
int strncmp(char *FirstPart, char *SecondPart, long Count);
/* 从左到右查找字符串str中首次出现字符ch的地址(不是下标,是地址) */
char *strchr(const char *str, const char ch);

/* 从后往前查找字符串str中首次出现字符ch的地址(不是下标,是地址) */
char *strrchr(const char *str, const char ch);

int strlen(const char *String);

#define list_for_each(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)
/* 	一直for，直到到达head 
	keep doing for loop until head is reached */

#define list_for_each_head_dell(pos, head) for (pos = (head)->next; pos != (head); pos = (head)->next)
/* 	一直for，直到head的下一个等于head（在循环的过程中需要改变head，否则死循环） 
	keep doing for loop until "next" of head equals to head (head need to be changed during the loop, otherwise that will be a infinite loop) */

#define list_entry(ptr, type, member) \
	((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))
/* list_t的指针减去它在整个结构体中的位置(&((type *)0)->member) (地址为0的type型数据中的member成员，就是member成员在type型数据中的偏移量），等于该结构体的开始
   The pointer of list_t minus its in the structure (&((type *)0)->member) (The member "member" in the data of type "type" at address 0, equals to the offset of member "member" in type "type"), you will get the start address of that structure*/

#define list_first_oneobj(head, o_type, o_member) list_entry((head)->next, o_type, o_member)

#define list_next_entry(pos, type, member) \
	list_entry((pos)->member.next, type, member)

#define list_prev_entry(pos, type, member) \
	list_entry((pos)->member.prev, type, member)

unsigned long bit_set(unsigned long *addr, unsigned long nr);

unsigned long bit_get(unsigned long *addr, unsigned long nr);

unsigned long bit_clean(unsigned long *addr, unsigned long nr);

unsigned char io_in8(unsigned short port);

unsigned int io_in32(unsigned short port);

void io_out8(unsigned short port, unsigned char value);

void io_out32(unsigned short port, unsigned int value);

//获得64位数中最高有效位（即最高的设置为1的位）的位置的函数
s64_t search_64rlbits(u64_t val)
{
    s64_t retbitnr = -1;
    __asm__ __volatile__(
        "bsrq %1,%q0 \t\n"
        : "+r"(retbitnr)
        : "rm"(val));
    return retbitnr + 1;
}

#define port_insw(port, buffer, nr)                                               \
	__asm__ __volatile__("cld;rep;insw;mfence;" ::"d"(port), "D"(buffer), "c"(nr) \
						 : "memory")

#define port_outsw(port, buffer, nr)                                               \
	__asm__ __volatile__("cld;rep;outsw;mfence;" ::"d"(port), "S"(buffer), "c"(nr) \
						 : "memory")

unsigned long rdmsr(unsigned long address);

void wrmsr(unsigned long address, unsigned long value);

unsigned long get_rsp();
void lower(char *str);
void upper(char *str);
unsigned long get_rflags();
long str_find_char(char *string, char ch, long strlen); // 自己写的 略显丑陋
long verify_area(unsigned char *addr, unsigned long size);
// 一对常用的数据复制函数，只不过这对函数会检测应用程序提供的应用层操作地址空间是否越界
long copy_from_user(void *from, void *to, unsigned long size);
long copy_to_user(void *from, void *to, unsigned long size);
long verify_area(unsigned char *addr, unsigned long size);
long copy_from_user(void *from, void *to, unsigned long size);
long copy_to_user(void *from, void *to, unsigned long size);
long strncpy_from_user(void *from, void *to, unsigned long size);
long strnlen_user(void *src, unsigned long maxlen);

#endif
