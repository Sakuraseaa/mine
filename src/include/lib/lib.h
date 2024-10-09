#ifndef __LIB_H__
#define __LIB_H__

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

#define SEEK_SET 0 /* Seek relative to start-of-file */
#define SEEK_CUR 1 /* Seek relative to current position */
#define SEEK_END 2 /* Seek relative to end-of-file */
#define SEEK_MAX 3

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

#define port_insw(port, buffer, nr)                                               \
	__asm__ __volatile__("cld;rep;insw;mfence;" ::"d"(port), "D"(buffer), "c"(nr) \
						 : "memory")

#define port_outsw(port, buffer, nr)                                               \
	__asm__ __volatile__("cld;rep;outsw;mfence;" ::"d"(port), "S"(buffer), "c"(nr) \
						 : "memory")

#endif
