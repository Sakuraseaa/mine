#ifndef _MEMDIVMER_T_H
#define _MEMDIVMER_T_H

#define DMF_RELDIV 0
#define DMF_MAXDIV 1

// typedef struct s_MCHKSTUC
// {
// 	list_h_t mc_list;
// 	u64_t mc_phyadr;
// 	u64_t mc_viradr;
// 	u64_t mc_sz;
// 	u64_t mc_chkval;
// 	msadsc_t* mc_msa; 
// 	u64_t* mc_chksadr;
// 	u64_t* mc_chkeadr; 
// }mchkstuc_t;
#include "msadsc_t.h"
#include "memmgrob.h"

msadsc_t *mm_division_pages(memmgrob_t *mmobjp, uint_t pages, uint_t *retrealpnr, uint_t mrtype, uint_t flgs);
#endif