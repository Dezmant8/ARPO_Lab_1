/* Private implementation. Public interface contracts are unchanged. */
#ifndef __C_ECOLAB64_H__
#define __C_ECOLAB64_H__
#include "IEcoLab64.h"
#include "IEcoSortAndSearch.h"
#include "IEcoSystem1.h"
#include "IdEcoMemoryManager1.h"

typedef struct CEcoLab64_97DD0B7B {
    /* Two interface views of one instance; IEcoLab64 is the identity view. */
    IEcoLab64VTbl* m_pVTblIEcoLab64;
    IEcoSortAndSearchVTbl* m_pVTblSort;
    uint32_t m_cRef;
    /* Each stored interface owns one reference acquired during creation. */
    IEcoMemoryAllocator1* m_pIMem;
    IEcoSystem1* m_pISys;
    uint32_t m_workers;
    BitonicExecutor m_executor;
    void* m_executor_context;
    int16_t m_last_status;
    BitonicStats m_last_stats;
} CEcoLab64_97DD0B7B;

/* Internal constructor and adapter setup, not COM interface methods. */
int16_t ECOCALLMETHOD createCEcoLab64_97DD0B7B(IEcoUnknown* system,
    IEcoUnknown* outer, const UGUID* iid, void** out);
void initialize_sort_interface(CEcoLab64_97DD0B7B* object);
#endif
