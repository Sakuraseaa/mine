#include "mmkit.h"
extern mmgro_t glomm;

uint_t read_cr3()
{
    uint_t regtmp = 0;
    __asm__ __volatile__(
        "movq %%cr3,%0\n\t"
        : "=r"(regtmp)
        :
        : "memory");
    return regtmp;
}

void write_cr3(uint_t r_val)
{
    __asm__ __volatile__(
        "movq %0,%%cr3 \n\t"
        :
        : "r"(r_val)
        : "memory" //, "edx"
    );
    return;
}

void mmudsc_t_init(mmudsc_t* init)
{
    if (nullptr == init) {
        return;
    }

    spin_init(&init->mud_lock);
	init->mud_stus = 0;
	init->mud_flag = 0;
	init->mud_tdirearr = nullptr;
	cr3s_t_init(&init->mud_cr3);
	list_init(&init->mud_tdirhead);	
	list_init(&init->mud_sdirhead);	
	list_init(&init->mud_idirhead);	
	list_init(&init->mud_mdirhead);
	init->mud_tdirmsanr = 0;	
	init->mud_sdirmsanr = 0;	
	init->mud_idirmsanr = 0;	
	init->mud_mdirmsanr = 0;
	return;	
}

msadsc_t* mmu_new_tdirearr(mmudsc_t* mmulocked)
{
    tdirearr_t* tdirearr = nullptr;
	u64_t pages = 1, retpnr = 0;
    msadsc_t* msa = nullptr;
    if(nullptr == mmulocked) {
        return nullptr;
    }
    
	msa = mm_division_pages(&glomm, pages, &retpnr, MA_TYPE_KRNL, DMF_RELDIV);
	if(nullptr == msa) {
		return nullptr;
	}

	tdirearr = (tdirearr_t*)msadsc_ret_vaddr(msa);

    tdirearr_t_init(tdirearr);

    list_add(&mmulocked->mud_tdirhead, &msa->md_list);
    mmulocked->mud_tdirmsanr++;
	mmulocked->mud_tdirearr = tdirearr;

    return msa;
}

bool_t mmu_del_tdirearr(mmudsc_t* mmulocked, tdirearr_t* tdirearr, msadsc_t* msa)
{
	list_h_t* pos;
	msadsc_t* tmpmsa;
	adr_t tblphyadr;
	if(nullptr == mmulocked || nullptr == tdirearr)
	{
		return FALSE;
	}

	tblphyadr = viradr_to_phyadr((adr_t)tdirearr);

	if(nullptr != msa)
	{
		if(msadsc_ret_addr(msa) == tblphyadr)
		{
			list_del(&msa->md_list);
			if(mm_merge_pages(&glomm, msa, onfrmsa_retn_fpagenr(msa)) == FALSE)
			{
				system_error("mmu_del_tdirearr err\n");
				return FALSE;
			}
			mmulocked->mud_tdirmsanr--;
			return TRUE;
		}
	}
	list_for_each(pos, &mmulocked->mud_tdirhead)
	{
		tmpmsa = list_entry(pos, msadsc_t, md_list);
		if(msadsc_ret_addr(tmpmsa) == tblphyadr)
		{
			list_del(&tmpmsa->md_list);
			if(mm_merge_pages(&glomm, tmpmsa, onfrmsa_retn_fpagenr(tmpmsa)) == FALSE)
			{
				system_error("mmu_del_tdirearr err\n");
				return FALSE;
			}
			mmulocked->mud_tdirmsanr--;
			return TRUE;
		}
	}
	return FALSE;
}

msadsc_t* mmu_new_sdirearr(mmudsc_t* mmulocked)
{
    sdirearr_t* sdirearr = nullptr;
	u64_t pages = 1, retpnr = 0;
    msadsc_t* msa = nullptr;
    if(nullptr == mmulocked)
    {
        return nullptr;
    }
    
	msa = mm_division_pages(&glomm, pages, &retpnr, MA_TYPE_KRNL, DMF_RELDIV);
	if(nullptr == msa)
	{
		return nullptr;
	}

	sdirearr = (sdirearr_t*)msadsc_ret_vaddr(msa);

    sdirearr_t_init(sdirearr);

    list_add(&mmulocked->mud_sdirhead, &msa->md_list);
    mmulocked->mud_sdirmsanr++;
    return msa;
}

bool_t mmu_del_sdirearr(mmudsc_t* mmulocked, sdirearr_t* sdirearr, msadsc_t* msa)
{
	list_h_t* pos;
	msadsc_t* tmpmsa;
	adr_t tblphyadr;
	if(nullptr == mmulocked || nullptr == sdirearr)
	{
		return FALSE;
	}

	tblphyadr = viradr_to_phyadr((adr_t)sdirearr);

	if(nullptr != msa)
	{
		if(msadsc_ret_addr(msa) == tblphyadr)
		{
			list_del(&msa->md_list);
			if(mm_merge_pages(&glomm, msa, onfrmsa_retn_fpagenr(msa)) == FALSE)
			{
				system_error("mmu_del_tdirearr err\n");
				return FALSE;
			}
			mmulocked->mud_sdirmsanr--;
			return TRUE;
		}
	}
	list_for_each(pos, &mmulocked->mud_sdirhead)
	{
		tmpmsa = list_entry(pos, msadsc_t, md_list);
		if(msadsc_ret_addr(tmpmsa) == tblphyadr)
		{
			list_del(&tmpmsa->md_list);
			if(mm_merge_pages(&glomm, tmpmsa, onfrmsa_retn_fpagenr(tmpmsa)) == FALSE)
			{
				system_error("mmu_del_tdirearr err\n");
				return FALSE;
			}
			mmulocked->mud_sdirmsanr--;
			return TRUE;
		}
	}
	return FALSE;
}

msadsc_t* mmu_new_idirearr(mmudsc_t* mmulocked)
{
    idirearr_t* idirearr = nullptr;
	u64_t pages = 1, retpnr = 0;
    msadsc_t* msa = nullptr;
    if(nullptr == mmulocked)
    {
        return nullptr;
    }
    
	msa = mm_division_pages(&glomm, pages, &retpnr, MA_TYPE_KRNL, DMF_RELDIV);
	if(nullptr == msa)
	{
		return nullptr;
	}

	idirearr = (idirearr_t*)msadsc_ret_vaddr(msa);

    idirearr_t_init(idirearr);

    list_add(&mmulocked->mud_idirhead, &msa->md_list);
    mmulocked->mud_idirmsanr++;
    
    return msa;
}

bool_t mmu_del_idirearr(mmudsc_t* mmulocked, idirearr_t* idirearr, msadsc_t* msa)
{
	list_h_t* pos;
	msadsc_t* tmpmsa;
	adr_t tblphyadr;
	if(nullptr == mmulocked || nullptr == idirearr)
	{
		return FALSE;
	}

	tblphyadr = viradr_to_phyadr((adr_t)idirearr);

	if(nullptr != msa)
	{
		if(msadsc_ret_addr(msa) == tblphyadr)
		{
			list_del(&msa->md_list);
			if(mm_merge_pages(&glomm, msa, onfrmsa_retn_fpagenr(msa)) == FALSE)
			{
				system_error("mmu_del_tdirearr err\n");
				return FALSE;
			}
			mmulocked->mud_idirmsanr--;
			return TRUE;
		}
	}
	list_for_each(pos, &mmulocked->mud_idirhead)
	{
		tmpmsa = list_entry(pos, msadsc_t, md_list);
		if(msadsc_ret_addr(tmpmsa) == tblphyadr)
		{
			list_del(&tmpmsa->md_list);
			if(mm_merge_pages(&glomm, tmpmsa, onfrmsa_retn_fpagenr(tmpmsa)) == FALSE)
			{
				system_error("mmu_del_tdirearr err\n");
				return FALSE;
			}
			mmulocked->mud_idirmsanr--;
			return TRUE;
		}
	}
	return FALSE;
}

msadsc_t* mmu_new_mdirearr(mmudsc_t* mmulocked)
{
    mdirearr_t* mdirearr = nullptr;
	u64_t pages = 1, retpnr = 0;
    msadsc_t* msa = nullptr;
    if(nullptr == mmulocked)
    {
        return nullptr;
    }
    
	msa = mm_division_pages(&glomm, pages, &retpnr, MA_TYPE_KRNL, DMF_RELDIV);
	if(nullptr == msa)
	{
		return nullptr;
	}

	mdirearr = (mdirearr_t*)msadsc_ret_vaddr(msa);
	mdirearr_t_init(mdirearr);
    
	list_add(&mmulocked->mud_mdirhead, &msa->md_list);
    mmulocked->mud_mdirmsanr++;

    return msa;
}

bool_t mmu_del_mdirearr(mmudsc_t* mmulocked, mdirearr_t* mdirearr, msadsc_t* msa)
{
	list_h_t* pos;
	msadsc_t* tmpmsa;
	adr_t tblphyadr;
	if(nullptr == mmulocked || nullptr == mdirearr)
	{
		return FALSE;
	}

	tblphyadr = viradr_to_phyadr((adr_t)mdirearr);

	if(nullptr != msa)
	{
		if(msadsc_ret_addr(msa) == tblphyadr)
		{
			list_del(&msa->md_list);
			if(mm_merge_pages(&glomm, msa, onfrmsa_retn_fpagenr(msa)) == FALSE)
			{
				system_error("mmu_del_tdirearr err\n");
				return FALSE;
			}
			mmulocked->mud_mdirmsanr--;
			return TRUE;
		}
	}
	list_for_each(pos, &mmulocked->mud_mdirhead)
	{
		tmpmsa = list_entry(pos, msadsc_t, md_list);
		if(msadsc_ret_addr(tmpmsa) == tblphyadr)
		{
			list_del(&tmpmsa->md_list);
			if(mm_merge_pages(&glomm, tmpmsa, onfrmsa_retn_fpagenr(tmpmsa)) == FALSE)
			{
				system_error("mmu_del_tdirearr err\n");
				return FALSE;
			}
			mmulocked->mud_mdirmsanr--;
			return TRUE;
		}
	}
	return FALSE;
}

adr_t mmu_untransform_msa(mmudsc_t* mmulocked, mdirearr_t* mdirearr, adr_t vadrs)
{
	uint_t mindex;
	mdire_t mdire;
	adr_t retadr;
	if(nullptr == mmulocked || nullptr == mdirearr)
	{
		return INVIALID;
	}

	mindex = mmu_mdire_index(vadrs);
	
	mdire = mdirearr->mde_arr[mindex];
	if(mmumsa_is_have(&mdire) == FALSE)
	{
		return INVIALID;
	}

	retadr = mmumsa_ret_padr(&mdire);
	
	mdirearr->mde_arr[mindex].m_entry = 0;
	return retadr; 
}

bool_t mmu_transform_msa(mmudsc_t* mmulocked, mdirearr_t* mdirearr, adr_t vadrs, adr_t padrs, u64_t flags)
{
	uint_t mindex;	
	if(nullptr == mmulocked || nullptr == mdirearr)
	{
		return FALSE;
	}

	mindex = mmu_mdire_index(vadrs);
	mdirearr->mde_arr[mindex].m_entry = (((u64_t)padrs) | flags);
	
	return TRUE;
}

bool_t mmu_untransform_mdire(mmudsc_t* mmulocked, idirearr_t* idirearr, msadsc_t* msa, adr_t vadrs)
{
	uint_t iindex;
	idire_t idire;
	mdirearr_t* mdirearr = nullptr;
	if(nullptr == mmulocked || nullptr == idirearr)
	{
		return FALSE;
	}

	iindex = mmu_idire_index(vadrs);
	
	idire = idirearr->ide_arr[iindex];
	if(mdire_is_have(&idire) == FALSE)
	{
		return TRUE;
	}

	mdirearr = idire_ret_mdirearr(&idire);
	if(mdirearr_is_allzero(mdirearr) == FALSE)
	{
		return TRUE;
	}

	if(mmu_del_mdirearr(mmulocked, mdirearr, msa) == FALSE)
	{
		return FALSE;
	}

	idirearr->ide_arr[iindex].i_entry = 0;
	
	return TRUE; 
}

mdirearr_t* mmu_transform_mdire(mmudsc_t* mmulocked, idirearr_t* idirearr, adr_t vadrs, u64_t flags, msadsc_t** outmsa)
{
	uint_t iindex;
	idire_t idire;
	adr_t dire;
	msadsc_t* msa = nullptr;
	mdirearr_t* mdirearr = nullptr;
	if(nullptr == mmulocked || nullptr == idirearr || nullptr == outmsa)
	{
		return nullptr;
	}
	
	iindex = mmu_idire_index(vadrs);
	
	idire = idirearr->ide_arr[iindex];
	if(mdire_is_have(&idire) == TRUE)
	{
		mdirearr = idire_ret_mdirearr(&idire);
		*outmsa = nullptr;
		return mdirearr;
	}

	msa = mmu_new_mdirearr(mmulocked);
	if(nullptr == msa)
	{
		*outmsa = nullptr;
		return nullptr;
	}

	dire = msadsc_ret_addr(msa);
	mdirearr = (mdirearr_t*)(phyadr_to_viradr(dire));
	idirearr->ide_arr[iindex].i_entry = (((u64_t)dire) | flags);

	*outmsa = msa;
	
	return mdirearr;
}

bool_t mmu_untransform_idire(mmudsc_t* mmulocked, sdirearr_t* sdirearr, msadsc_t* msa, adr_t vadrs)
{
	uint_t sindex;
	sdire_t sdire;
	idirearr_t* idirearr = nullptr;
	if(nullptr == mmulocked || nullptr == sdirearr)
	{
		return FALSE;
	}

	sindex = mmu_sdire_index(vadrs);
	
	sdire = sdirearr->sde_arr[sindex];
	if(idire_is_have(&sdire) == FALSE)
	{
		return TRUE;
	}

	idirearr = sdire_ret_idirearr(&sdire);
	if(idirearr_is_allzero(idirearr) == FALSE)
	{
		return TRUE;
	}

	if(mmu_del_idirearr(mmulocked, idirearr, msa) == FALSE)
	{
		return FALSE;
	}

	sdirearr->sde_arr[sindex].s_entry = 0;
	
	return TRUE; 
}

idirearr_t* mmu_transform_idire(mmudsc_t* mmulocked, sdirearr_t* sdirearr, adr_t vadrs, u64_t flags, msadsc_t** outmsa)
{
	uint_t sindex;
	sdire_t sdire;
	adr_t dire;	
	msadsc_t* msa = nullptr;	
	idirearr_t* idirearr = nullptr;
	if(nullptr == mmulocked || nullptr == sdirearr || nullptr == outmsa)
	{
		return nullptr;
	}
	
	sindex = mmu_sdire_index(vadrs);
	
	sdire = sdirearr->sde_arr[sindex];
	if(idire_is_have(&sdire) == TRUE)
	{
		idirearr = sdire_ret_idirearr(&sdire);
		*outmsa = nullptr;
		return idirearr;
	}

	msa = mmu_new_idirearr(mmulocked);
	if(nullptr == msa)
	{
		*outmsa = nullptr;
		return nullptr;
	}

	dire = msadsc_ret_addr(msa);
	idirearr = (idirearr_t*)(phyadr_to_viradr(dire));
	sdirearr->sde_arr[sindex].s_entry = (((u64_t)dire) | flags);

	*outmsa = msa;
	
	return idirearr;
}

bool_t mmu_untransform_sdire(mmudsc_t* mmulocked, tdirearr_t* tdirearr, msadsc_t* msa, adr_t vadrs)
{
	uint_t tindex;
	tdire_t tdire;
	sdirearr_t* sdirearr = nullptr;
	if(nullptr == mmulocked || nullptr == tdirearr)
	{
		return FALSE;
	}

	tindex = mmu_tdire_index(vadrs);
	
	tdire = tdirearr->tde_arr[tindex];
	if(sdire_is_have(&tdire) == FALSE)
	{
		return TRUE;
	}

	sdirearr = tdire_ret_sdirearr(&tdire);
	if(sdirearr_is_allzero(sdirearr) == FALSE)
	{
		return TRUE;
	}

	if(mmu_del_sdirearr(mmulocked, sdirearr, msa) == FALSE)
	{
		return FALSE;
	}

	tdirearr->tde_arr[tindex].t_entry = 0;
	
	return TRUE; 
}

sdirearr_t* mmu_transform_sdire(mmudsc_t* mmulocked, tdirearr_t* tdirearr, adr_t vadrs, u64_t flags, msadsc_t** outmsa)
{
	uint_t tindex;
	tdire_t tdire;
	adr_t dire;
	sdirearr_t* sdirearr = nullptr;
	msadsc_t* msa = nullptr;
	if (nullptr == mmulocked || nullptr == tdirearr || nullptr == outmsa) {
		return nullptr;
	}
	
	tindex = mmu_tdire_index(vadrs);
	
	tdire = tdirearr->tde_arr[tindex];
	if (sdire_is_have(&tdire) == TRUE) {
		sdirearr = tdire_ret_sdirearr(&tdire);
		*outmsa = nullptr;
		return sdirearr;
	}

	msa = mmu_new_sdirearr(mmulocked);
	if(nullptr == msa)
	{
		*outmsa = nullptr;
		return nullptr;
	}

	dire = msadsc_ret_addr(msa);
	sdirearr = (sdirearr_t*)(phyadr_to_viradr(dire));
	tdirearr->tde_arr[tindex].t_entry = (((u64_t)dire) | flags);

	*outmsa = msa;
	
	return sdirearr;
}

bool_t hal_mmu_transform_core(mmudsc_t* mmu, adr_t vadrs, adr_t padrs, u64_t flags)
{
	bool_t rets = FALSE;
	tdirearr_t* tdirearr = nullptr;
	sdirearr_t* sdirearr = nullptr;
	idirearr_t* idirearr = nullptr;
	mdirearr_t* mdirearr = nullptr;
	msadsc_t* smsa = nullptr;
	msadsc_t* imsa = nullptr;
	msadsc_t* mmsa = nullptr;

	// knl_spinlock(&mmu->mud_lock);
	
	tdirearr = mmu->mud_tdirearr;
	if(nullptr == tdirearr) {
		rets = FALSE;
		goto out;
	}

	sdirearr = mmu_transform_sdire(mmu, tdirearr, vadrs, flags, &smsa);
	if(nullptr == sdirearr) {
		rets = FALSE;
		goto untf_sdire;		
	}

	idirearr = mmu_transform_idire(mmu, sdirearr, vadrs, flags, &imsa);
	if(nullptr == idirearr)
	{
		rets = FALSE;
		goto untf_idire;		
	}

	mdirearr = mmu_transform_mdire(mmu, idirearr, vadrs, flags, &mmsa);
	if(nullptr == mdirearr)
	{
		rets = FALSE;
		goto untf_mdire;
	}

	rets = mmu_transform_msa(mmu, mdirearr, vadrs, padrs, flags);
	if(TRUE == rets)
	{
		rets = TRUE;
		hal_mmu_refresh();
		goto out;
	}

	rets = FALSE;

	mmu_untransform_msa(mmu, mdirearr, vadrs);
untf_mdire:
	mmu_untransform_mdire(mmu, idirearr, mmsa, vadrs);
untf_idire:
	mmu_untransform_idire(mmu, sdirearr, imsa, vadrs);
untf_sdire:
	mmu_untransform_sdire(mmu, tdirearr, smsa, vadrs);	
out:
	// knl_spinunlock(&mmu->mud_lock);
	return rets;
}

bool_t hal_mmu_transform(mmudsc_t* mmu, adr_t vadrs, adr_t padrs, u64_t flags)
{
	if(nullptr == mmu)
	{
		return FALSE;
	}
	return hal_mmu_transform_core(mmu, vadrs, padrs, flags);
}

adr_t mmu_find_msaadr(mdirearr_t* mdirearr, adr_t vadrs)
{
	uint_t mindex;
	mdire_t dire;
	if (nullptr == mdirearr) {
		return INVIALID;
	}

	mindex = mmu_mdire_index(vadrs);

	dire = mdirearr->mde_arr[mindex];

	if(mmumsa_is_have(&dire) == FALSE)
	{
		return INVIALID;
	}

	return mmumsa_ret_padr(&dire);
}


mdirearr_t* mmu_find_mdirearr(idirearr_t* idirearr, adr_t vadrs)
{
	uint_t iindex;
	idire_t dire;
	if(nullptr == idirearr)
	{
		return nullptr;
	}

	iindex = mmu_idire_index(vadrs);

	dire = idirearr->ide_arr[iindex];

	if(mdire_is_have(&dire) == FALSE)
	{
		return nullptr;
	}

	return idire_ret_mdirearr(&dire);
}


idirearr_t* mmu_find_idirearr(sdirearr_t* sdirearr, adr_t vadrs)
{
	uint_t sindex;
	sdire_t dire;
	if(nullptr == sdirearr)
	{
		return nullptr;
	}

	sindex = mmu_sdire_index(vadrs);

	dire = sdirearr->sde_arr[sindex];

	if(idire_is_have(&dire) == FALSE)
	{
		return nullptr;
	}

	return sdire_ret_idirearr(&dire);
}

sdirearr_t* mmu_find_sdirearr(tdirearr_t* tdirearr, adr_t vadrs)
{
	uint_t tindex;
	tdire_t dire;
	if(nullptr == tdirearr)
	{
		return nullptr;
	}

	tindex = mmu_tdire_index(vadrs);

	dire = tdirearr->tde_arr[tindex];

	if(sdire_is_have(&dire) == FALSE)
	{
		return nullptr;
	}

	return tdire_ret_sdirearr(&dire);
}

adr_t hal_mmu_untransform_core(mmudsc_t* mmu, adr_t vadrs)
{
	adr_t retadr;
	sdirearr_t* sdirearr;
	idirearr_t* idirearr;
	mdirearr_t* mdirearr;
//	knl_spinlock(&mmu->mud_lock);
	sdirearr = mmu_find_sdirearr(mmu->mud_tdirearr, vadrs);
	if(nullptr == sdirearr)
	{
		retadr = INVIALID;
		goto out;
	}

	idirearr = mmu_find_idirearr(sdirearr, vadrs);
	if(nullptr == idirearr)
	{
		retadr = INVIALID;
		goto untf_sdirearr;
	}

	mdirearr = mmu_find_mdirearr(idirearr, vadrs);
	if(nullptr == mdirearr)
	{
		retadr = INVIALID;
		goto untf_idirearr; 
	}
	
	retadr = mmu_untransform_msa(mmu, mdirearr, vadrs);

	mmu_untransform_mdire(mmu, idirearr, nullptr, vadrs);
untf_idirearr:
	mmu_untransform_idire(mmu, sdirearr, nullptr, vadrs);
untf_sdirearr:
	mmu_untransform_sdire(mmu, mmu->mud_tdirearr, nullptr, vadrs);
out:	
//	knl_spinunlock(&mmu->mud_lock);
	return retadr;
}

adr_t hal_mmu_untransform(mmudsc_t* mmu, adr_t vadrs)
{
	if(nullptr == mmu)
	{
		return EPARAM;
	}
	return hal_mmu_untransform_core(mmu, vadrs);
}

void hal_mmu_load(mmudsc_t* mmu)
{
	if(nullptr == mmu) {
		return;
	}
	
	// knl_spinlock(&mmu->mud_lock);
	if(nullptr == mmu->mud_tdirearr || 0 != (((u64_t)(mmu->mud_tdirearr)) & 0xfff))
	{
		goto out;
	}

	mmu->mud_cr3.c3s_entry = viradr_to_phyadr((adr_t)mmu->mud_tdirearr);
	write_cr3((uint_t)(mmu->mud_cr3.c3s_entry));

out:
	// knl_spinunlock(&mmu->mud_lock);	
	return;
}

void hal_mmu_refresh()
{
	cr3s_t cr3;
	cr3.c3s_entry = (u64_t)read_cr3();
	write_cr3(cr3.c3s_entry);
	return;
}

bool_t hal_mmu_init(mmudsc_t* mmu)
{
	bool_t  rets = FALSE;
	adr_t pcr3 = INVIALID, vcr3 = INVIALID;
	cr3s_t cr3;
	if(nullptr == mmu) {
		return FALSE;
	}

	// knl_spinlock(&mmu->mud_lock);

	if(mmu_new_tdirearr(mmu) == nullptr)
	{
		rets = FALSE;
		goto out;
	}

	cr3.c3s_entry = (u64_t)read_cr3();

	pcr3 = (adr_t)(cr3.c3s_c3sflgs.c3s_plm4a << 12);
	vcr3 = phyadr_to_viradr(pcr3);

	memcpy((void*)vcr3, (void*)mmu->mud_tdirearr, sizeof(tdirearr_t));
	
	mmu->mud_cr3.c3s_entry = (u64_t)viradr_to_phyadr((adr_t)mmu->mud_tdirearr);
	mmu->mud_tdirearr->tde_arr[0].t_entry = 0;
	rets = TRUE;

out:	
	// knl_spinunlock(&mmu->mud_lock);
	return rets;
}

bool_t mmu_clean_mdirearrmsas(mmudsc_t* mmulocked)
{
	list_h_t* pos;
	msadsc_t* msa = nullptr;
	if(nullptr == mmulocked)
	{
		return FALSE;
	}
	list_for_each_head_dell(pos, &mmulocked->mud_mdirhead)
	{
		msa = list_entry(pos, msadsc_t, md_list);
		list_del(&msa->md_list);
		if(mm_merge_pages(&glomm, msa, onfrmsa_retn_fpagenr(msa)) == FALSE)
		{
			system_error("mmu_clean_mdirearrmsas");
			return FALSE;
		}
		mmulocked->mud_mdirmsanr--;
	}
	return TRUE;
}

bool_t mmu_clean_idirearrmsas(mmudsc_t* mmulocked)
{
	list_h_t* pos;
	msadsc_t* msa = nullptr;
	if(nullptr == mmulocked)
	{
		return FALSE;
	}
	list_for_each_head_dell(pos, &mmulocked->mud_idirhead)
	{
		msa = list_entry(pos, msadsc_t, md_list);
		list_del(&msa->md_list);
		if(mm_merge_pages(&glomm, msa, onfrmsa_retn_fpagenr(msa)) == FALSE)
		{
			system_error("mmu_clean_idirearrmsas");
			return FALSE;
		}
		mmulocked->mud_idirmsanr--;
	}
	return TRUE;
}

bool_t mmu_clean_sdirearrmsas(mmudsc_t* mmulocked)
{
	list_h_t* pos;
	msadsc_t* msa = nullptr;
	if(nullptr == mmulocked)
	{
		return FALSE;
	}
	list_for_each_head_dell(pos, &mmulocked->mud_sdirhead)
	{
		msa = list_entry(pos, msadsc_t, md_list);
		list_del(&msa->md_list);
		if(mm_merge_pages(&glomm, msa, onfrmsa_retn_fpagenr(msa)) == FALSE)
		{
			system_error("mmu_clean_sdirearrmsas");
			return FALSE;
		}
		mmulocked->mud_sdirmsanr--;
	}
	return TRUE;
}

bool_t mmu_clean_tdirearrmsas(mmudsc_t* mmulocked)
{
	list_h_t* pos;
	msadsc_t* msa = nullptr;
	if(nullptr == mmulocked)
	{
		return FALSE;
	}
	list_for_each_head_dell(pos, &mmulocked->mud_tdirhead)
	{
		msa = list_entry(pos, msadsc_t, md_list);
		list_del(&msa->md_list);
		if(mm_merge_pages(&glomm, msa, onfrmsa_retn_fpagenr(msa)) == FALSE)
		{
			system_error("mmu_clean_tdirearrmsas");
			return FALSE;
		}
		mmulocked->mud_tdirmsanr--;
	}
	return TRUE;
}

bool_t hal_mmu_clean(mmudsc_t* mmu)
{
	bool_t  rets = FALSE;
	adr_t pcr3 = INVIALID, vcr3 = INVIALID;
	cr3s_t cr3;
	if(nullptr == mmu)
	{
		return FALSE;
	}

	// knl_spinlock(&mmu->mud_lock);

	cr3.c3s_entry = (u64_t)read_cr3();

	pcr3 = (adr_t)(cr3.c3s_c3sflgs.c3s_plm4a << 12);
	vcr3 = phyadr_to_viradr(pcr3);

	if(vcr3 == (adr_t)(mmu->mud_tdirearr))
	{
		rets = FALSE;
		goto out;
	}

	if(mmu_clean_mdirearrmsas(mmu) == FALSE)
	{
		rets = FALSE;
		goto out;
	}

	if(mmu_clean_idirearrmsas(mmu) == FALSE)
	{
		rets = FALSE;
		goto out;
	}

	if(mmu_clean_sdirearrmsas(mmu) == FALSE)
	{
		rets = FALSE;
		goto out;
	}

	if(mmu_clean_tdirearrmsas(mmu) == FALSE)
	{
		rets = FALSE;
		goto out;
	}

	rets = TRUE;

out:	
	//knl_spinunlock(&mmu->mud_lock);
	return rets;
}

void dump_mmu(mmudsc_t* dump)
{
	if(nullptr == dump)
	{
		return;
	}
	// kprint("mmudsc_t.mud_tdirearr:%x\n", dump->mud_tdirearr);
	// kprint("mmudsc_t.mud_cr3:%x\n", dump->mud_cr3.c3s_entry);
	// kprint("mmudsc_t.mud_tdirmsanr:%x\n", dump->mud_tdirmsanr);
	// kprint("mmudsc_t.mud_sdirmsanr:%x\n", dump->mud_sdirmsanr);
	// kprint("mmudsc_t.mud_idirmsanr:%x\n", dump->mud_idirmsanr);
	// kprint("mmudsc_t.mud_mdirmsanr:%x\n", dump->mud_mdirmsanr);
	// kprint("mmudsc_t.mud_tdirhead:%x\n", list_is_empty_careful(&dump->mud_tdirhead));
	// kprint("mmudsc_t.mud_sdirhead:%x\n", list_is_empty_careful(&dump->mud_sdirhead));
	// kprint("mmudsc_t.mud_idirhead:%x\n", list_is_empty_careful(&dump->mud_idirhead));
	// kprint("mmudsc_t.mud_mdirhead:%x\n", list_is_empty_careful(&dump->mud_mdirhead));
	return;
}

