#ifndef __LIB_H__
#define __LIB_H__

#define NULL 0

#define container_of(ptr, type, member)                                     \
	({                                                                      \
		typeof(((type *)0)->member) *p = (ptr);                             \
		(type *)((unsigned long)p - (unsigned long)&(((type *)0)->member)); \
	})
#define hlt() __asm__ __volatile__("hlt \n\t" :: \
									   : "memory")
#define sti() __asm__ __volatile__("sti	\n\t" :: \
									   : "memory")
#define cli() __asm__ __volatile__("cli	\n\t" :: \
									   : "memory")
#define nop() __asm__ __volatile__("nop	\n\t")
// 保证系统在后面的memory访问之前，先前的memory访问都以及结束
#define io_mfence() __asm__ __volatile__("mfence	\n\t" :: \
											 : "memory")

struct List
{
	struct List *prev;
	struct List *next;
};

void list_init(struct List *list);

void list_add_to_behind(struct List *entry, struct List *pnew); ////add to entry behind

void list_add_to_before(struct List *entry, struct List *pnew); ////add to entry behind

void list_del(struct List *entry);

// return: 1 = 空 ， 0 = 不空
long list_is_empty(struct List *entry);

struct List *list_prev(struct List *entry);

struct List *list_next(struct List *entry);

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

char *strcpy(char *Dest, char *Src);

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

int strcmp(char *FirstPart, char *SecondPart);

/*
		string compare FirstPart and SecondPart with Count Bytes
		FirstPart = SecondPart =>  0
		FirstPart > SecondPart =>  1
		FirstPart < SecondPart => -1
*/

int strncmp(char *FirstPart, char *SecondPart, long Count);

int strlen(char *String);

unsigned long bit_set(unsigned long *addr, unsigned long nr);

unsigned long bit_get(unsigned long *addr, unsigned long nr);

unsigned long bit_clean(unsigned long *addr, unsigned long nr);

unsigned char io_in8(unsigned short port);

unsigned int io_in32(unsigned short port);

void io_out8(unsigned short port, unsigned char value);

void io_out32(unsigned short port, unsigned int value);

#define port_insw(port, buffer, nr)                                               \
	__asm__ __volatile__("cld;rep;insw;mfence;" ::"d"(port), "D"(buffer), "c"(nr) \
						 : "memory")

#define port_outsw(port, buffer, nr)                                               \
	__asm__ __volatile__("cld;rep;outsw;mfence;" ::"d"(port), "S"(buffer), "c"(nr) \
						 : "memory")

unsigned long rdmsr(unsigned long address);

void wrmsr(unsigned long address, unsigned long value);

unsigned long get_rsp();

unsigned long get_rflags();

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
