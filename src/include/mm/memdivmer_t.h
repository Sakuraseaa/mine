#ifndef _MEMDIVMER_T_H
#define _MEMDIVMER_T_H

#define DMF_RELDIV 0    // 0 表示获取 指定大小 的内存块 
#define DMF_MAXDIV 1 	// 1 表示获取该内存区中，尺寸最大的一个内存块

typedef struct s_MCHKSTUC
{
	list_h_t mc_list;
	u64_t mc_phyadr;
	u64_t mc_viradr;
	u64_t mc_sz;
	u64_t mc_chkval;
	msadsc_t* mc_msa; 
	u64_t* mc_chksadr;
	u64_t* mc_chkeadr; 
}mchkstuc_t;

#endif