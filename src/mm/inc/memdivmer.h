#ifndef _MEMDIVMER_H_
#define _MEMDIVMER_H_

bool_t mm_merge_pages(mmgro_t *mmobjp, msadsc_t *freemsa, uint_t freepgs);
msadsc_t *mm_division_pages(mmgro_t *mmobjp, uint_t pages, uint_t *retrealpnr, uint_t mrtype, uint_t flgs);
msadsc_t *mm_divpages_procmarea(mmgro_t *mmobjp, uint_t pages, uint_t *retrealpnr);
u64_t onfrmsa_retn_fpagenr(msadsc_t* freemsa);
void* umalloc_4k_page(uint_t pages);
void* kmalloc_4k_page(uint_t pages);
void* hmalloc_4k_page(uint_t pages);
void kfree_4k_page(void* addr);

#endif // _MEMDIVMER_H_