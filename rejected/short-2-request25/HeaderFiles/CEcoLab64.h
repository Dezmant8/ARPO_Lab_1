/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   CEcoLab64_97DD0B7B
 * </summary>
 *
 * <description>
 *   This header describes the implementation of the CEcoLab64_97DD0B7B component
 * </description>
 *
 * <author>
 *   Copyright (c) 2018 . All rights reserved.
 * </author>
 *
 */

#ifndef __C_ECOLAB64_H__
#define __C_ECOLAB64_H__

#include "IEcoLab64.h"
#include "IEcoSortAndSearch.h"
extern IEcoSortAndSearchVTbl g_sort_vtable;
#include "IEcoSystem1.h"
#include "IdEcoMemoryManager1.h"

typedef struct CEcoLab64_97DD0B7B* CEcoLab64_97DD0B7BPtr_t;

typedef struct CEcoLab64_97DD0B7B {

    /* IEcoLab64 interface function table */
    IEcoLab64VTbl* m_pVTblIEcoLab64;


    /* Instance initialization */
    int16_t (ECOCALLMETHOD *Init)(/*in*/ CEcoLab64_97DD0B7BPtr_t pCMe, /* in */ IEcoUnknownPtr_t pIUnkSystem);
    /* Instance creation */
    int16_t (ECOCALLMETHOD *Create)(/*in*/ CEcoLab64_97DD0B7BPtr_t pCMe, /* in */ IEcoUnknownPtr_t pIUnkSystem, /* in */ IEcoUnknownPtr_t pIUnkOuter);
    /* Deletion */
    void (ECOCALLMETHOD *Delete)(/*in*/ CEcoLab64_97DD0B7BPtr_t pCMe);


    /* Reference counter */
    uint32_t m_cRef;

    /* Interface for memory operations */
    IEcoMemoryAllocator1* m_pIMem;

    /* System interface */
    IEcoSystem1* m_pISys;

    /* Instance data */
    char_t* m_Name;
    IEcoSortAndSearchVTbl* m_pVTblSort;
    uint32_t m_workers;
    BitonicExecutor m_executor;
    void* m_executor_context;
    int16_t m_last_status;
    BitonicStats m_last_stats;

} CEcoLab64_97DD0B7B;

#endif /* __C_ECOLAB64_H__ */
