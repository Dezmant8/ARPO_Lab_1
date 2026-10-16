/* Common interface adapter: delegates all sorting to Qwen's method. */
#include "CEcoLab64.h"
#include "ArpoMethods.h"
#include <stddef.h>
static CEcoLab64_97DD0B7B* owner(IEcoSortAndSearch* me) {
    if(!me) return 0;
    return (CEcoLab64_97DD0B7B*)((char*)me - offsetof(CEcoLab64_97DD0B7B,m_pVTblSort));
}
static int16_t ECOCALLMETHOD sort_query(IEcoSortAndSearch* me,const UGUID* iid,void** out) {
    CEcoLab64_97DD0B7B* obj=owner(me);
    if(!out) return ERR_ECO_POINTER;
    *out=0;
    if(!obj || !iid) return ERR_ECO_POINTER;
    return obj->m_pVTblIEcoLab64->QueryInterface((IEcoLab64*)obj,iid,out);
}
static uint32_t ECOCALLMETHOD sort_addref(IEcoSortAndSearch* me) {
    CEcoLab64_97DD0B7B* obj=owner(me);
    if(!obj) return 0;
    return obj->m_pVTblIEcoLab64->AddRef((IEcoLab64*)obj);
}
static uint32_t ECOCALLMETHOD sort_release(IEcoSortAndSearch* me) {
    CEcoLab64_97DD0B7B* obj=owner(me);
    if(!obj) return 0;
    return obj->m_pVTblIEcoLab64->Release((IEcoLab64*)obj);
}
static void ECOCALLMETHOD sort_call(IEcoSortAndSearch* me,void* base,size_t count,
        size_t width,int (*compare)(const void*,const void*)) {
    CEcoLab64_97DD0B7B* obj=owner(me);
    if(!obj) return;
    obj->m_last_stats.padded_count=0;
    obj->m_last_stats.stages=0;
    obj->m_last_stats.network_pairs=0;
    if(count>1048576 || width>(size_t)0xFFFFFFFFU) { obj->m_last_status=-1; return; }
    obj->m_last_status=lab_bitonic_sort((IEcoLab64*)obj,base,(uint32_t)count,(uint32_t)width,
        compare,obj->m_workers,obj->m_executor,obj->m_executor_context,&obj->m_last_stats);
}
static int16_t ECOCALLMETHOD sort_configure(IEcoSortAndSearch* me,uint32_t workers,
        BitonicExecutor executor,void* context) {
    CEcoLab64_97DD0B7B* obj=owner(me);
    if(!obj || workers==0 || workers>16 || (workers>1 && !executor)) return -1;
    obj->m_workers=workers; obj->m_executor=executor; obj->m_executor_context=context;
    return 0;
}
static int16_t ECOCALLMETHOD sort_result(IEcoSortAndSearch* me,BitonicStats* stats) {
    CEcoLab64_97DD0B7B* obj=owner(me);
    if(!obj || !stats) return -1;
    *stats=obj->m_last_stats;
    return obj->m_last_status;
}
static IEcoSortAndSearchVTbl g_sort_vtable = {
    sort_query,sort_addref,sort_release,sort_call,sort_configure,sort_result
};
void initialize_sort_interface(CEcoLab64_97DD0B7B* object) {
    object->m_pVTblSort=&g_sort_vtable;
    object->m_workers=1;
    object->m_executor=0;
    object->m_executor_context=0;
    object->m_last_status=0;
    object->m_last_stats.padded_count=0;
    object->m_last_stats.stages=0;
    object->m_last_stats.network_pairs=0;
}
