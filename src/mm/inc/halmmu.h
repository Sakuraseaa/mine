#ifndef _HALMMU_H
#define _HALMMU_H


void mmudsc_t_init(mmudsc_t* init);
msadsc_t* mmu_new_tdirearr(mmudsc_t* mmulocked);
bool_t mmu_del_tdirearr(mmudsc_t* mmulocked, L4_ptarr_t* tdirearr, msadsc_t* msa);
msadsc_t* mmu_new_sdirearr(mmudsc_t* mmulocked);
bool_t mmu_del_sdirearr(mmudsc_t* mmulocked, L3_ptarr_t* sdirearr, msadsc_t* msa);
msadsc_t* mmu_new_idirearr(mmudsc_t* mmulocked);
bool_t mmu_del_idirearr(mmudsc_t* mmulocked, L2_ptarr_t* idirearr, msadsc_t* msa);
msadsc_t* mmu_new_mdirearr(mmudsc_t* mmulocked);
bool_t mmu_del_mdirearr(mmudsc_t* mmulocked, L1_ptarr_t* mdirearr, msadsc_t* msa);
adr_t mmu_untransform_msa(mmudsc_t* mmulocked, L1_ptarr_t* mdirearr, adr_t vadrs);
bool_t mmu_transform_msa(mmudsc_t* mmulocked, L1_ptarr_t* mdirearr, adr_t vadrs, adr_t padrs, u64_t flags);
bool_t mmu_untransform_mdire(mmudsc_t* mmulocked, L2_ptarr_t* idirearr, msadsc_t* msa, adr_t vadrs);
L1_ptarr_t* mmu_transform_mdire(mmudsc_t* mmulocked, L2_ptarr_t* idirearr, adr_t vadrs, u64_t flags, msadsc_t** outmsa);
bool_t mmu_untransform_idire(mmudsc_t* mmulocked, L3_ptarr_t* sdirearr, msadsc_t* msa, adr_t vadrs);
L2_ptarr_t* mmu_transform_idire(mmudsc_t* mmulocked, L3_ptarr_t* sdirearr, adr_t vadrs, u64_t flags, msadsc_t** outmsa);
bool_t mmu_untransform_sdire(mmudsc_t* mmulocked, L4_ptarr_t* tdirearr, msadsc_t* msa, adr_t vadrs);
L3_ptarr_t* mmu_transform_sdire(mmudsc_t* mmulocked, L4_ptarr_t* tdirearr, adr_t vadrs, u64_t flags, msadsc_t** outmsa);
bool_t hal_mmu_transform_core(mmudsc_t* mmu, adr_t vadrs, adr_t padrs, u64_t flags);
bool_t hal_mmu_transform(mmudsc_t* mmu, adr_t vadrs, adr_t padrs, u64_t flags);
adr_t mmu_find_msaadr(L1_ptarr_t* mdirearr, adr_t vadrs);
L1_ptarr_t* mmu_find_mdirearr(L2_ptarr_t* idirearr, adr_t vadrs);
L2_ptarr_t* mmu_find_idirearr(L3_ptarr_t* sdirearr, adr_t vadrs);
L3_ptarr_t* mmu_find_sdirearr(L4_ptarr_t* tdirearr, adr_t vadrs);
adr_t hal_mmu_untransform_core(mmudsc_t* mmu, adr_t vadrs);
adr_t hal_mmu_untransform(mmudsc_t* mmu, adr_t vadrs);
void hal_mmu_load(mmudsc_t* mmu);
void hal_mmu_refresh();
bool_t hal_mmu_init(mmudsc_t* mmu);
bool_t mmu_clean_mdirearrmsas(mmudsc_t* mmulocked);
bool_t mmu_clean_idirearrmsas(mmudsc_t* mmulocked);
bool_t mmu_clean_sdirearrmsas(mmudsc_t* mmulocked);
bool_t mmu_clean_tdirearrmsas(mmudsc_t* mmulocked);
bool_t hal_mmu_clean(mmudsc_t* mmu);
void dump_mmu(mmudsc_t* dump);

#if 1
extern adr_t viradr_to_phyadr(adr_t kviradr);
extern adr_t phyadr_to_viradr(adr_t kphyadr);

KLINE uint_t mmu_tdire_index(adr_t vadrs)
{
	return (uint_t)((vadrs >> TDIRE_IV_RSHTBIT) & TDIRE_IV_BITMASK);
}

KLINE uint_t mmu_sdire_index(adr_t vadrs)
{
	return (uint_t)((vadrs >> SDIRE_IV_RSHTBIT) & SDIRE_IV_BITMASK);
}

KLINE uint_t mmu_idire_index(adr_t vadrs)
{
	return (uint_t)((vadrs >> IDIRE_IV_RSHTBIT) & IDIRE_IV_BITMASK);
}

KLINE uint_t mmu_mdire_index(adr_t vadrs)
{
	return (uint_t)((vadrs >> MDIRE_IV_RSHTBIT) & MDIRE_IV_BITMASK);
}


KLINE void cr3s_t_init(cr3s_t* init)
{
    if(nullptr == init)
    {
        return;
    }
    init->c3s_entry = 0;
    return;
}

KLINE bool_t sdirearr_is_allzero(L3_ptarr_t* sdirearr)
{
    for(uint_t i = 0; i < SDIRE_MAX; i++)
    {
        if(0 != sdirearr->sde_arr[i].s_entry)
        {
            return FALSE;
        }
    }
    return TRUE;
}

KLINE bool_t sdire_is_have(L4_pte_t* tdire)
{
    if(0 < tdire->t_flags.t_sdir)
    {
        return TRUE;
    }
    return FALSE;
}

KLINE bool_t sdire_is_presence(L4_pte_t* tdire)
{
    if(1 == tdire->t_flags.t_p)
    {
        return TRUE;
    }
    return FALSE;
}

KLINE adr_t sdire_ret_padr(L4_pte_t* tdire)
{
    return (adr_t)(tdire->t_flags.t_sdir << SDIRE_PADR_LSHTBIT);
}

KLINE adr_t sdire_ret_vadr(L4_pte_t* tdire)
{
    return phyadr_to_viradr(sdire_ret_padr(tdire));
}

KLINE L3_ptarr_t* tdire_ret_sdirearr(L4_pte_t* tdire)
{
    return (L3_ptarr_t*)(sdire_ret_vadr(tdire));
}

KLINE bool_t idirearr_is_allzero(L2_ptarr_t* idirearr)
{
    for(uint_t i = 0; i < IDIRE_MAX; i++)
    {
        if(0 != idirearr->ide_arr[i].i_entry)
        {
            return FALSE;
        }
    }
    return TRUE;
}

KLINE bool_t idire_is_have(L3_pte_t* sdire)
{
    if(0 < sdire->s_flags.s_idir)
    {
        return TRUE;
    }
    return FALSE;
}

KLINE bool_t idire_is_presence(L3_pte_t* sdire)
{
    if(1 == sdire->s_flags.s_p)
    {
        return TRUE;
    }
    return FALSE;
}

KLINE adr_t idire_ret_padr(L3_pte_t* sdire)
{
    return (adr_t)(sdire->s_flags.s_idir << IDIRE_PADR_LSHTBIT);
}

KLINE adr_t idire_ret_vadr(L3_pte_t* sdire)
{
    return phyadr_to_viradr(idire_ret_padr(sdire));
}

KLINE L2_ptarr_t* sdire_ret_idirearr(L3_pte_t* sdire)
{
    return (L2_ptarr_t*)(idire_ret_vadr(sdire));
}

KLINE bool_t mdirearr_is_allzero(L1_ptarr_t* mdirearr)
{
    for(uint_t i = 0; i < MDIRE_MAX; i++)
    {
        if(0 != mdirearr->mde_arr[i].m_entry)
        {
            return FALSE;
        }
    }
    return TRUE;
}

KLINE bool_t mdire_is_have(L2_pte_t* idire)
{
    if(0 < idire->i_flags.i_mdir)
    {
        return TRUE;
    }
    return FALSE;
}

KLINE bool_t mdire_is_presence(L2_pte_t* idire)
{
    if(1 == idire->i_flags.i_p)
    {
        return TRUE;
    }
    return FALSE;
}


KLINE adr_t mdire_ret_padr(L2_pte_t* idire)
{
    return (adr_t)(idire->i_flags.i_mdir << MDIRE_PADR_LSHTBIT);
}

KLINE adr_t mdire_ret_vadr(L2_pte_t* idire)
{
    return phyadr_to_viradr(mdire_ret_padr(idire));
}

KLINE L1_ptarr_t* idire_ret_mdirearr(L2_pte_t* idire)
{
    return (L1_ptarr_t*)(mdire_ret_vadr(idire));
}

KLINE bool_t mmumsa_is_have(L1_pte_t* mdire)
{
    if(0 < mdire->m_flags.m_msa) {
        return TRUE;
    }
    return FALSE;
}

KLINE bool_t mmumsa_is_presence(L1_pte_t* mdire)
{
    if(1 == mdire->m_flags.m_p)
    {
        return TRUE;
    }
    return FALSE;
}

KLINE adr_t mmumsa_ret_padr(L1_pte_t* mdire)
{
    return (adr_t)(mdire->m_flags.m_msa << MSA_PADR_LSHTBIT);
}

KLINE void tdirearr_t_init(L4_ptarr_t* init)
{
    if (nullptr == init) {
        return;
    }
    memset((void*)init, 0, sizeof(L4_ptarr_t));
    return;
}

KLINE void sdirearr_t_init(L3_ptarr_t* init)
{
    if(nullptr == init)
    {
        return;
    }
    memset((void*)init, 0, sizeof(L3_ptarr_t));
    return;
}

KLINE void idirearr_t_init(L2_ptarr_t* init)
{
    if(nullptr == init)
    {
        return;
    }
    memset((void*)init, 0, sizeof(L2_ptarr_t));
    return;
}

KLINE void mdirearr_t_init(L1_ptarr_t* init)
{
    if(nullptr == init)
    {
        return;
    }
    memset((void*)init, 0, sizeof(L1_ptarr_t));
    return;
}
#endif

#endif // HALMMU_H

