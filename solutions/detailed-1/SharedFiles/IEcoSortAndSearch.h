/* ACOM adds only me; Sort has the return and four arguments of C qsort.
 * Configure controls the caller-owned synchronous executor separately.
 * Array storage remains caller-owned. Use one caller at a time per instance. */
#ifndef I_ECO_SORT_AND_SEARCH_H
#define I_ECO_SORT_AND_SEARCH_H
#include <stddef.h>
#include "IEcoLab64.h"
static const UGUID IID_IEcoSortAndSearch = {0x01,0x10,{0xE9,0xC3,0xE4,0xC7,0x38,0x53,0x43,0xEB,0xAA,0x64,0x64,0x67,0xCC,0x71,0xA5,0xA8}};
typedef struct IEcoSortAndSearch* IEcoSortAndSearchPtr_t;
typedef struct IEcoSortAndSearchVTbl {
    int16_t (ECOCALLMETHOD *QueryInterface)(IEcoSortAndSearchPtr_t me,const UGUID* iid,void** out);
    uint32_t (ECOCALLMETHOD *AddRef)(IEcoSortAndSearchPtr_t me);
    uint32_t (ECOCALLMETHOD *Release)(IEcoSortAndSearchPtr_t me);
    void (ECOCALLMETHOD *Sort)(IEcoSortAndSearchPtr_t me,void* base,size_t count,
        size_t width,int (*compare)(const void*,const void*));
    int16_t (ECOCALLMETHOD *Configure)(IEcoSortAndSearchPtr_t me,uint32_t workers,
        BitonicExecutor executor,void* executor_context);
    int16_t (ECOCALLMETHOD *GetResult)(IEcoSortAndSearchPtr_t me,BitonicStats* stats);
} IEcoSortAndSearchVTbl;
typedef struct IEcoSortAndSearch { IEcoSortAndSearchVTbl* pVTbl; } IEcoSortAndSearch;
#endif
