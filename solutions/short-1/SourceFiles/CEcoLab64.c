/* Common ACOM object implementation; the measured algorithm is unchanged. */
#include "ArpoMethods.h"
#include "CEcoLab64.h"
#include "IEcoInterfaceBus1.h"
#include "IEcoInterfaceBus1MemExt.h"

static int16_t ECOCALLMETHOD lab_query(IEcoLab64* me, const UGUID* iid, void** out) {
    CEcoLab64_97DD0B7B* object = (CEcoLab64_97DD0B7B*)me;
    if (!out) return ERR_ECO_POINTER;
    *out = 0;
    if (!me || !iid) return ERR_ECO_POINTER;
    if (IsEqualUGUID(iid, &IID_IEcoLab64) || IsEqualUGUID(iid, &IID_IEcoUnknown))
        *out = &object->m_pVTblIEcoLab64;
    else if (IsEqualUGUID(iid, &IID_IEcoSortAndSearch))
        *out = &object->m_pVTblSort;
    else return ERR_ECO_NOINTERFACE;
    object->m_pVTblIEcoLab64->AddRef(me);
    return ERR_ECO_SUCCESS;
}
static uint32_t ECOCALLMETHOD lab_addref(IEcoLab64* me) {
    CEcoLab64_97DD0B7B* object = (CEcoLab64_97DD0B7B*)me;
    if (!object) return 0;
    return ++object->m_cRef;
}
static uint32_t ECOCALLMETHOD lab_release(IEcoLab64* me) {
    CEcoLab64_97DD0B7B* object = (CEcoLab64_97DD0B7B*)me;
    IEcoMemoryAllocator1* memory;
    IEcoSystem1* system;
    uint32_t refs;
    if (!object || !object->m_cRef) return 0;
    refs = --object->m_cRef;
    if (refs) return refs;
    memory = object->m_pIMem;
    system = object->m_pISys;
    memory->pVTbl->Free(memory, object);
    system->pVTbl->Release(system);
    memory->pVTbl->Release(memory);
    return 0;
}
static IEcoLab64VTbl g_lab_vtable = {
    lab_query, lab_addref, lab_release, lab_bitonic_sort
};

int16_t ECOCALLMETHOD createCEcoLab64_97DD0B7B(IEcoUnknown* system,
        IEcoUnknown* outer, const UGUID* iid, void** out) {
    IEcoSystem1* sys = 0;
    IEcoInterfaceBus1* bus = 0;
    IEcoInterfaceBus1MemExt* extension = 0;
    IEcoMemoryAllocator1* memory = 0;
    CEcoLab64_97DD0B7B* object = 0;
    const UGUID* selected;
    UGUID manager = CID_EcoMemoryManager1;
    int16_t result;
    if (!out) return ERR_ECO_POINTER;
    *out = 0;
    if (!system || !iid) return ERR_ECO_POINTER;
    /* No delegating/nondelegating IUnknown: reject all aggregation. */
    if (outer) return ERR_ECO_NOAGGREGATION;
    if (!IsEqualUGUID(iid, &IID_IEcoLab64) && !IsEqualUGUID(iid, &IID_IEcoUnknown)
            && !IsEqualUGUID(iid, &IID_IEcoSortAndSearch)) return ERR_ECO_NOINTERFACE;

    result = system->pVTbl->QueryInterface(system, &GID_IEcoSystem, (void**)&sys);
    if (result != 0 || !sys) { result = ERR_ECO_NOSYSTEM; goto cleanup; }
    result = sys->pVTbl->QueryInterface(sys, &IID_IEcoInterfaceBus1, (void**)&bus);
    if (result != 0 || !bus) { result = ERR_ECO_NOBUS; goto cleanup; }
    result = bus->pVTbl->QueryInterface(bus, &IID_IEcoInterfaceBus1MemExt, (void**)&extension);
    if (result == 0 && extension) {
        selected = extension->pVTbl->get_Manager(extension);
        if (!selected) { result = ERR_ECO_GET_MEMORY_ALLOCATOR; goto cleanup; }
        /* Copy the identifier before releasing its owner. */
        manager = *selected;
        extension->pVTbl->Release(extension);
        extension = 0;
    }
    else if (result != ERR_ECO_NOINTERFACE) {
        if (result == 0) result = ERR_ECO_GET_MEMORY_ALLOCATOR;
        goto cleanup;
    }
    result = bus->pVTbl->QueryComponent(bus, &manager, 0,
        &IID_IEcoMemoryAllocator1, (void**)&memory);
    if (result != 0 || !memory) { result = ERR_ECO_GET_MEMORY_ALLOCATOR; goto cleanup; }
    object = (CEcoLab64_97DD0B7B*)memory->pVTbl->Alloc(memory, sizeof(CEcoLab64_97DD0B7B));
    if (!object) { result = ERR_ECO_OUTOFMEMORY; goto cleanup; }

    object->m_pVTblIEcoLab64 = &g_lab_vtable;
    object->m_cRef = 1;
    /* Transfer the two acquired references to the new instance. */
    object->m_pIMem = memory; memory = 0;
    object->m_pISys = sys; sys = 0;
    initialize_sort_interface(object);
    result = lab_query((IEcoLab64*)object, iid, out);
    lab_release((IEcoLab64*)object);

cleanup:
    if (extension) extension->pVTbl->Release(extension);
    if (bus) bus->pVTbl->Release(bus);
    if (memory) memory->pVTbl->Release(memory);
    if (sys) sys->pVTbl->Release(sys);
    return result;
}
IEcoMemoryAllocator1* ECOCALLMETHOD lab_allocator(IEcoLab64* me) {
    if (!me) return 0;
    return ((CEcoLab64_97DD0B7B*)me)->m_pIMem;
}
