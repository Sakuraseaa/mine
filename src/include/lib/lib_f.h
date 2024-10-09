#ifndef _LIB_F_H_
#define _LIB_F_H_
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

unsigned long bit_set(unsigned long *addr, unsigned long nr);
unsigned long bit_get(unsigned long *addr, unsigned long nr);
unsigned long bit_clean(unsigned long *addr, unsigned long nr);

unsigned char io_in8(unsigned short port);
unsigned int io_in32(unsigned short port);
void io_out8(unsigned short port, unsigned char value);
void io_out32(unsigned short port, unsigned int value);

s64_t search_64rlbits(u64_t val);

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

#endif // _LIB_F_H_