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
s64_t list_is_empty(struct List *entry);
struct List *list_prev(struct List *entry);
struct List *list_next(struct List *entry);

u64_t rdmsr(u64_t address);
void wrmsr(u64_t address, u64_t value);

u64_t get_rsp();
void lower(str_t str);
void upper(str_t str);
u64_t get_rflags();
s64_t str_find_char(str_t string, char_t ch, s64_t strlen); // 自己写的 略显丑陋
s64_t verify_area(u8_t *addr, u64_t size);
// 一对常用的数据复制函数，只不过这对函数会检测应用程序提供的应用层操作地址空间是否越界
s64_t copy_from_user(void *from, void *to, u64_t size);
s64_t copy_to_user(void *from, void *to, u64_t size);
s64_t verify_area(u8_t *addr, u64_t size);
s64_t copy_from_user(void *from, void *to, u64_t size);
s64_t copy_to_user(void *from, void *to, u64_t size);
s64_t strncpy_from_user(void *from, void *to, u64_t size);
s64_t strnlen_user(void *src, u64_t maxlen);

u64_t bit_set(u64_t *addr, u64_t nr);
u64_t bit_get(u64_t *addr, u64_t nr);
u64_t bit_clean(u64_t *addr, u64_t nr);

u8_t io_in8(u16_t port);
u32_t io_in32(u16_t port);
void io_out8(u16_t port, u8_t value);
void io_out32(u16_t port, u32_t value);

s64_t search_64rlbits(u64_t val);

/*
		From => To memory copy Num bytes
*/
void *memcpy(void *From, void *To, s64_t Num);
/*
		FirstPart = SecondPart		=>	 0
		FirstPart > SecondPart		=>	 1
		FirstPart < SecondPart		=>	-1
*/
s32_t memcmp(void *FirstPart, void *SecondPart, s64_t Count);
/*
		set memory at Address with C ,number is Count
*/
void *memset(void *Address, u8_t C, s64_t Count);
/*
		string copy
*/
str_t strcpy(str_t Dest,cstr_t Src);
/*
		string copy number bytes
*/
str_t strncpy(str_t Dest, str_t Src, s64_t Count);
/*
		string cat Dest + Src
*/
str_t strcat(str_t Dest, str_t Src);
/*
		string compare FirstPart and SecondPart
		FirstPart = SecondPart =>  0
		FirstPart > SecondPart =>  1
		FirstPart < SecondPart => -1
*/
s32_t strcmp(str_t FirstPart,cstr_t SecondPart);
/*
		string compare FirstPart and SecondPart with Count Bytes
		FirstPart = SecondPart =>  0
		FirstPart > SecondPart =>  1
		FirstPart < SecondPart => -1
*/
s32_t strncmp(str_t FirstPart, str_t SecondPart, s64_t Count);
/* 从左到右查找字符串str中首次出现字符ch的地址(不是下标,是地址) */
str_t strchr(cstr_t str, cchar_t ch);
/* 从后往前查找字符串str中首次出现字符ch的地址(不是下标,是地址) */
str_t strrchr(cstr_t str, cchar_t ch);
s32_t strlen(cstr_t String);

#endif // _LIB_F_H_