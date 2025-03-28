#include "usrinit.h"

static unsigned long brk_start_address = 0;
static unsigned long brk_used_address = 0;
static unsigned long brk_end_address = 0;

#define	SIZE_ALIGN	(8 * sizeof(unsigned long))
#define	PAGESIZE	(1UL << 12)
#define	PAGE_SIZE	PAGESIZE

extern unsigned long brk(unsigned long brk);
void * malloc(unsigned long size, int invalid)
{
	unsigned long address = 0;

	if(size <= 0)
	{
		printf("malloc size <= 0\n");
		return nullptr;
	}

	if(brk_start_address == 0)
		brk_end_address = brk_used_address = brk_start_address = brk(0);

	if(brk_end_address <= brk_used_address + SIZE_ALIGN + size) {
		// printf("start:%d used:%d endaddrss:%d, arg:%d", brk_start_address, brk_used_address, brk_end_address, brk_end_address + ((size + SIZE_ALIGN + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)));
		brk_end_address = brk(brk_end_address + ((size + SIZE_ALIGN + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)));
		// printf("start:%d used:%d endaddrss:%d, arg:%d", brk_start_address, brk_used_address, brk_end_address, brk_end_address + ((size + SIZE_ALIGN + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)));
	}
	address = brk_used_address;
	brk_used_address += size + SIZE_ALIGN;

	return (void *)address;
}


void free(void * address)
{
}