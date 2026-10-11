/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   IEcoLab64
 * </summary>
 *
 * <description>
 *   This header describes the interface IEcoLab64
 * </description>
 *
 * <reference>
 *
 * </reference>
 *
 * <author>
 *   Copyright (c) 2026 . All rights reserved.
 * </author>
 *
 */

#ifndef __I_ECOLAB64_H__
#define __I_ECOLAB64_H__

#include "IEcoBase1.h"

/* IEcoLab64 IID = {63D20705-ABDB-4E2B-8618-02C857FD994D} */
#ifndef __IID_IEcoLab64
static const UGUID IID_IEcoLab64 = {0x01, 0x10, {0x63, 0xD2, 0x07, 0x05, 0xAB, 0xDB, 0x4E, 0x2B, 0x86, 0x18, 0x02, 0xC8, 0x57, 0xFD, 0x99, 0x4D}};
#endif /* __IID_IEcoLab64 */

/* Generic comparator must define a total order and be safe for concurrent calls.
 * Executor runs job(context, lane, workers) once for every lane, then joins all
 * jobs before returning. Jobs in one invocation may run concurrently. */
typedef int (ECOCALLMETHOD *BitonicCompare)(const void* left, const void* right);
typedef void (ECOCALLMETHOD *BitonicJob)(void* context, uint32_t lane, uint32_t workers);
typedef int16_t (ECOCALLMETHOD *BitonicExecutor)(void* context, BitonicJob job,
    void* job_context, uint32_t workers);
typedef struct BitonicStats {
    uint32_t padded_count;
    uint32_t stages;
    uint64_t network_pairs;
} BitonicStats;

typedef struct IEcoLab64* IEcoLab64Ptr_t;

typedef struct IEcoLab64VTbl {

    /* IEcoUnknown */
    int16_t (ECOCALLMETHOD *QueryInterface)(/* in */ IEcoLab64Ptr_t me, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv);
    uint32_t (ECOCALLMETHOD *AddRef)(/* in */ IEcoLab64Ptr_t me);
    uint32_t (ECOCALLMETHOD *Release)(/* in */ IEcoLab64Ptr_t me);

    /* IEcoLab64 */
    int16_t (ECOCALLMETHOD *Sort)(IEcoLab64Ptr_t me, void* base,
        uint32_t count, uint32_t width, BitonicCompare compare, uint32_t workers,
        BitonicExecutor executor, void* executor_context, BitonicStats* stats);

} IEcoLab64VTbl, *IEcoLab64VTblPtr_t;

interface IEcoLab64 {
    struct IEcoLab64VTbl *pVTbl;
} IEcoLab64;


#endif /* __I_ECOLAB64_H__ */

