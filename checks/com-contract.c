/* Independent COM/ACOM contract and ownership tests. Host libc is intentional. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "IEcoSystem1.h"
#include "IEcoInterfaceBus1.h"
#include "IEcoInterfaceBus1MemExt.h"
#include "IdEcoInterfaceBus1.h"
#include "IdEcoMemoryManager1.h"
#include "IdEcoLab64.h"
#include "IEcoSortAndSearch.h"
#include "CEcoLab64.h"

typedef struct MockEnvironment {
    IEcoUnknown unknown;
    IEcoSystem1 system;
    IEcoInterfaceBus1 bus;
    IEcoInterfaceBus1MemExt extension;
    IEcoMemoryAllocator1 memory;
    uint32_t refs;
    uint32_t memory_refs;
    uint32_t live;
    uint32_t system_queries;
    uint32_t bus_queries;
    uint32_t memory_queries;
    uint32_t allocations;
    uint32_t fail_system;
    uint32_t fail_bus;
    uint32_t fail_memory;
    uint32_t fail_alloc;
    uint32_t no_extension;
    uint32_t invalidate_manager;
    UGUID manager;
} MockEnvironment;

static MockEnvironment mock;
static unsigned int cases;
static unsigned int errors;

static int check(int ok, const char* name) {
    cases++;
    if (!ok) { errors++; printf("FAIL,%s\n", name); }
    return ok;
}

static int16_t query_environment(const UGUID* iid, void** out) {
    if (!out) return ERR_ECO_POINTER;
    *out = 0;
    if (!iid) return ERR_ECO_POINTER;
    if (IsEqualUGUID(iid, &IID_IEcoUnknown)) *out = &mock.unknown;
    else if (IsEqualUGUID(iid, &GID_IEcoSystem)) {
        mock.system_queries++;
        if (mock.system_queries == mock.fail_system) return ERR_ECO_NOSYSTEM;
        *out = &mock.system;
    }
    else if (IsEqualUGUID(iid, &IID_IEcoInterfaceBus1)) {
        mock.bus_queries++;
        if (mock.bus_queries == mock.fail_bus) return ERR_ECO_NOBUS;
        *out = &mock.bus;
    }
    else if (IsEqualUGUID(iid, &IID_IEcoInterfaceBus1MemExt) && !mock.no_extension)
        *out = &mock.extension;
    else return ERR_ECO_NOINTERFACE;
    mock.refs++;
    return 0;
}
static int16_t ECOCALLMETHOD unknown_query(IEcoUnknown* me, const UGUID* iid, void** out) {
    if (me != &mock.unknown) return ERR_ECO_POINTER;
    return query_environment(iid, out);
}
static uint32_t ECOCALLMETHOD unknown_addref(IEcoUnknown* me) {
    return me == &mock.unknown ? ++mock.refs : 0;
}
static uint32_t ECOCALLMETHOD unknown_release(IEcoUnknown* me) {
    return me == &mock.unknown && mock.refs ? --mock.refs : 0;
}
static int16_t ECOCALLMETHOD system_query(IEcoSystem1* me, const UGUID* iid, void** out) {
    if (me != &mock.system) return ERR_ECO_POINTER;
    return query_environment(iid, out);
}
static uint32_t ECOCALLMETHOD system_addref(IEcoSystem1* me) {
    return me == &mock.system ? ++mock.refs : 0;
}
static uint32_t ECOCALLMETHOD system_release(IEcoSystem1* me) {
    return me == &mock.system && mock.refs ? --mock.refs : 0;
}
static void ECOCALLMETHOD system_set(IEcoSystem1* me, void* data) { (void)me; (void)data; }
static void* ECOCALLMETHOD system_get(IEcoSystem1* me) { (void)me; return 0; }
static int16_t ECOCALLMETHOD bus_query(IEcoInterfaceBus1* me, const UGUID* iid, void** out) {
    if (me != &mock.bus) return ERR_ECO_POINTER;
    return query_environment(iid, out);
}
static uint32_t ECOCALLMETHOD bus_addref(IEcoInterfaceBus1* me) {
    return me == &mock.bus ? ++mock.refs : 0;
}
static uint32_t ECOCALLMETHOD bus_release(IEcoInterfaceBus1* me) {
    return me == &mock.bus && mock.refs ? --mock.refs : 0;
}
static int16_t ECOCALLMETHOD bus_init(IEcoInterfaceBus1* me) { (void)me; return 0; }
static int16_t ECOCALLMETHOD bus_init_with(IEcoInterfaceBus1* me, void* p, uint32_t n) {
    (void)me; (void)p; (void)n; return 0;
}
static int16_t ECOCALLMETHOD bus_register(IEcoInterfaceBus1* me, const UGUID* cid, IEcoUnknown* f) {
    (void)me; (void)cid; (void)f; return 0;
}
static int16_t ECOCALLMETHOD bus_unregister(IEcoInterfaceBus1* me, const UGUID* cid) {
    (void)me; (void)cid; return 0;
}
static int16_t ECOCALLMETHOD extension_query(IEcoInterfaceBus1MemExt* me, const UGUID* iid, void** out) {
    if (me != &mock.extension) return ERR_ECO_POINTER;
    return query_environment(iid, out);
}
static uint32_t ECOCALLMETHOD extension_addref(IEcoInterfaceBus1MemExt* me) {
    return me == &mock.extension ? ++mock.refs : 0;
}
static uint32_t ECOCALLMETHOD extension_release(IEcoInterfaceBus1MemExt* me) {
    if (me == &mock.extension && mock.invalidate_manager)
        ((unsigned char*)&mock.manager)[sizeof(mock.manager)-1] ^= 1;
    return me == &mock.extension && mock.refs ? --mock.refs : 0;
}
static int16_t ECOCALLMETHOD extension_set(IEcoInterfaceBus1MemExt* me, const UGUID* cid) {
    (void)me; (void)cid; return 0;
}
static const UGUID* ECOCALLMETHOD extension_get(IEcoInterfaceBus1MemExt* me) {
    (void)me; return &mock.manager;
}
static int16_t ECOCALLMETHOD extension_expand(IEcoInterfaceBus1MemExt* me, bool_t value) {
    (void)me; (void)value; return 0;
}
static int16_t ECOCALLMETHOD memory_query(IEcoMemoryAllocator1* me, const UGUID* iid, void** out) {
    if (!out) return ERR_ECO_POINTER;
    *out = 0;
    if (me != &mock.memory || !iid) return ERR_ECO_POINTER;
    if (!IsEqualUGUID(iid, &IID_IEcoMemoryAllocator1) && !IsEqualUGUID(iid, &IID_IEcoUnknown))
        return ERR_ECO_NOINTERFACE;
    *out = me; mock.memory_refs++; return 0;
}
static uint32_t ECOCALLMETHOD memory_addref(IEcoMemoryAllocator1* me) {
    return me == &mock.memory ? ++mock.memory_refs : 0;
}
static uint32_t ECOCALLMETHOD memory_release(IEcoMemoryAllocator1* me) {
    return me == &mock.memory && mock.memory_refs ? --mock.memory_refs : 0;
}
static void* ECOCALLMETHOD memory_alloc(IEcoMemoryAllocator1* me, uint32_t n) {
    void* p;
    if (me != &mock.memory) return 0;
    mock.allocations++;
    if (mock.allocations == mock.fail_alloc) return 0;
    p = malloc(n); if (p) mock.live++; return p;
}
static void ECOCALLMETHOD memory_free(IEcoMemoryAllocator1* me, void* p) {
    if (me == &mock.memory && p) { mock.live--; free(p); }
}
static void* ECOCALLMETHOD memory_realloc(IEcoMemoryAllocator1* me, void* p, uint32_t n) {
    (void)me; return realloc(p, n);
}
static void* ECOCALLMETHOD memory_copy(IEcoMemoryAllocator1* me, void* dst, void* src, uint32_t n) {
    (void)me; return memcpy(dst, src, n);
}
static void* ECOCALLMETHOD memory_fill(IEcoMemoryAllocator1* me, void* dst, char_t v, uint32_t n) {
    (void)me; return memset(dst, v, n);
}
static int16_t ECOCALLMETHOD memory_compare(IEcoMemoryAllocator1* me, void* a, void* b, uint32_t n) {
    int result;
    (void)me; result = memcmp(a, b, n); return (int16_t)((result > 0) - (result < 0));
}
static uint32_t ECOCALLMETHOD memory_size(IEcoMemoryAllocator1* me, void* p) {
    (void)me; (void)p; return 0;
}
static int16_t ECOCALLMETHOD bus_component(IEcoInterfaceBus1* me, const UGUID* cid,
        IEcoUnknown* outer, const UGUID* iid, void** out) {
    (void)outer;
    if (!out) return ERR_ECO_POINTER;
    *out = 0;
    if (me != &mock.bus || !iid) return ERR_ECO_POINTER;
    if (!cid || !IsEqualUGUID(cid, &CID_EcoMemoryManager1)) return ERR_ECO_GET_MEMORY_ALLOCATOR;
    mock.memory_queries++;
    if (mock.memory_queries == mock.fail_memory) return ERR_ECO_GET_MEMORY_ALLOCATOR;
    return memory_query(&mock.memory, iid, out);
}
static IEcoUnknownVTbl unknown_vtable = {unknown_query, unknown_addref, unknown_release};
static IEcoSystem1VTbl system_vtable = {
    system_query, system_addref, system_release, system_set, system_get
};
static IEcoInterfaceBus1VTbl bus_vtable = {
    bus_query, bus_addref, bus_release, bus_init, bus_init_with,
    bus_register, bus_unregister, bus_component
};
static IEcoMemoryAllocator1VTbl memory_vtable = {
    memory_query, memory_addref, memory_release, memory_alloc, memory_free,
    memory_realloc, memory_copy, memory_fill, memory_compare, memory_size
};
static IEcoInterfaceBus1MemExtVTbl extension_vtable = {
    extension_query, extension_addref, extension_release, extension_set, extension_get, extension_expand
};

static void reset_mock(void) {
    memset(&mock, 0, sizeof(mock));
    mock.unknown.pVTbl = &unknown_vtable;
    mock.system.pVTbl = &system_vtable;
    mock.bus.pVTbl = &bus_vtable;
    mock.extension.pVTbl = &extension_vtable;
    mock.memory.pVTbl = &memory_vtable;
    mock.refs = 1; mock.memory_refs = 1;
    mock.manager = CID_EcoMemoryManager1;
}
static void release_interface(void* p) {
    IEcoUnknown* u = (IEcoUnknown*)p;
    if (u) u->pVTbl->Release(u);
}
static void check_balanced(void) {
    check(mock.refs == 1, "system_and_bus_references_balanced");
    check(mock.memory_refs == 1, "allocator_references_balanced");
    check(mock.live == 0, "all_component_allocations_freed");
}
static void matrix(IEcoLab64* lab) {
    const UGUID* ids[3];
    IEcoUnknown* source;
    IEcoUnknown* destination;
    IEcoUnknown* unknown;
    IEcoUnknown* canonical = 0;
    UGUID missing = IID_IEcoLab64;
    unsigned int i, j, repeat;
    void* out;
    int16_t result;
    ids[0] = &IID_IEcoLab64; ids[1] = &IID_IEcoSortAndSearch; ids[2] = &IID_IEcoUnknown;
    ((unsigned char*)&missing)[sizeof(missing)-1] ^= 1;
    result = lab->pVTbl->QueryInterface(lab, &IID_IEcoUnknown, (void**)&canonical);
    if (!check(result == 0 && canonical != 0, "canonical_unknown_available")) return;
    for (i = 0; i < 3; i++) {
        source = 0;
        result = lab->pVTbl->QueryInterface(lab, ids[i], (void**)&source);
        if (!check(result == 0 && source != 0, "source_interface_available")) continue;
        for (j = 0; j < 3; j++) for (repeat = 0; repeat < 2; repeat++) {
            destination = 0;
            result = source->pVTbl->QueryInterface(source, ids[j], (void**)&destination);
            if (!check(result == 0 && destination != 0, "all_pairs_reflexive_symmetric_transitive_static")) continue;
            unknown = 0;
            result = destination->pVTbl->QueryInterface(destination, &IID_IEcoUnknown, (void**)&unknown);
            check(result == 0 && unknown == canonical, "same_unknown_identity_from_every_interface");
            release_interface(unknown); release_interface(destination);
        }
        for (repeat = 0; repeat < 2; repeat++) {
            out = source;
            result = source->pVTbl->QueryInterface(source, &missing, &out);
            check(result != 0 && out == 0, "unsupported_iid_static_and_output_cleared");
        }
        release_interface(source);
    }
    release_interface(canonical);
}

int16_t EcoMain(IEcoUnknown* unused_system) {
    IEcoComponentFactory* factory = GetIEcoComponentFactoryPtr_AEBA731EF9FB4A7B8B40055A97DD0B7B;
    const char* mode = getenv("ARPO_COM_CASE");
    IEcoLab64* lab = 0;
    IEcoSortAndSearch* sorter = 0;
    IEcoUnknown* identity = 0;
    CEcoLab64_97DD0B7B* object;
    void* out;
    int16_t result;
    uint32_t count;
    (void)unused_system;
    setvbuf(stdout, 0, _IONBF, 0);
    reset_mock();
    if (!mode) mode = "matrix";
    printf("COM_CASE,%s\n", mode);
    if (strcmp(mode, "factory_null_output") == 0) {
        check(factory->pVTbl->QueryInterface(factory, &IID_IEcoComponentFactory, 0) != 0,
            "factory_qi_null_output_is_safe");
    }
    else if (strcmp(mode, "factory_null_iid") == 0) {
        out = factory;
        result = factory->pVTbl->QueryInterface(factory, 0, &out);
        check(result != 0 && out == 0, "factory_qi_null_iid_clears_output");
    }
    else if (strcmp(mode, "factory_null_me") == 0) {
        out = factory;
        result = factory->pVTbl->QueryInterface(0, &IID_IEcoComponentFactory, &out);
        check(result != 0 && out == 0, "factory_qi_null_me_clears_output");
    }
    else if (strcmp(mode, "aggregation") == 0) {
        out = &mock.unknown;
        result = factory->pVTbl->Alloc(factory, &mock.unknown, &mock.unknown, &IID_IEcoUnknown, &out);
        check(result == ERR_ECO_NOAGGREGATION && out == 0, "unsupported_aggregation_rejected_even_for_unknown");
        if (result == 0) {
            ((IEcoUnknown*)out)->pVTbl->QueryInterface((IEcoUnknown*)out,
                &IID_IEcoSortAndSearch, (void**)&sorter);
            if (sorter) {
                sorter->pVTbl->QueryInterface(sorter, &IID_IEcoUnknown, (void**)&identity);
                check(identity == &mock.unknown, "accepted_aggregation_must_delegate_unknown_to_outer");
                release_interface(identity); sorter->pVTbl->Release(sorter);
            }
            release_interface(out);
        }
    }
    else if (strcmp(mode, "factory_refcount") == 0) {
        count = factory->pVTbl->AddRef(factory);
        check(count == 1, "factory_starts_without_outstanding_references");
        check(factory->pVTbl->Release(factory) == 0, "factory_balanced_release");
        check(factory->pVTbl->Release(factory) == 0, "factory_refcount_does_not_underflow");
    }
    else if (strcmp(mode, "alloc_null_system") == 0 || strcmp(mode, "alloc_null_iid") == 0
            || strcmp(mode, "alloc_null_output") == 0 || strcmp(mode, "alloc_null_me") == 0) {
        out = factory;
        result = factory->pVTbl->Alloc(strcmp(mode, "alloc_null_me") == 0 ? 0 : factory,
            strcmp(mode, "alloc_null_system") == 0 ? 0 : &mock.unknown, 0,
            strcmp(mode, "alloc_null_iid") == 0 ? 0 : &IID_IEcoLab64,
            strcmp(mode, "alloc_null_output") == 0 ? 0 : &out);
        check(result != 0, "invalid_factory_allocation_rejected");
        if (strcmp(mode, "alloc_null_output") != 0) check(out == 0, "failed_allocation_output_cleared");
    }
    else {
        if (strcmp(mode, "system_failure") == 0) mock.fail_system = 1;
        if (strcmp(mode, "bus_failure") == 0) mock.fail_bus = 1;
        if (strcmp(mode, "memory_failure") == 0) mock.fail_memory = 1;
        if (strcmp(mode, "allocation_failure") == 0) mock.fail_alloc = 1;
        if (strcmp(mode, "second_bus_query_failure") == 0) mock.fail_bus = 2;
        if (strcmp(mode, "optional_extension_absent") == 0) mock.no_extension = 1;
        if (strcmp(mode, "manager_identifier_lifetime") == 0) mock.invalidate_manager = 1;
        if (strcmp(mode, "second_memory_query_failure") == 0) mock.fail_memory = 2;
        out = factory;
        result = factory->pVTbl->Alloc(factory, &mock.unknown, 0, &IID_IEcoLab64, &out);
        printf("CREATE_RESULT,status=%d,system_queries=%u,bus_queries=%u,memory_queries=%u,allocations=%u\n",
            result, mock.system_queries, mock.bus_queries, mock.memory_queries, mock.allocations);
        if (mock.fail_system || mock.fail_bus == 1 || mock.fail_memory == 1 || mock.fail_alloc) {
            check(result != 0 && out == 0, "failed_creation_returns_error_and_null");
        }
        else if (check(result == 0 && out != 0, "factory_creates_object")) {
            lab = (IEcoLab64*)out;
            if (strcmp(mode, "matrix") == 0) matrix(lab);
            if (strcmp(mode, "ownership") == 0) {
                object = (CEcoLab64_97DD0B7B*)lab;
                check(object->m_pISys == &mock.system, "system_pointer_obtained_via_queryinterface");
                check(mock.refs == 2, "exactly_one_owned_system_reference");
                check(mock.memory_refs == 2, "exactly_one_owned_allocator_reference");
            }
            if (strcmp(mode, "second_bus_query_failure") == 0)
                check(mock.bus_queries == 1, "no_redundant_unchecked_bus_query");
            if (strcmp(mode, "null_iid_output") == 0) {
                out = lab;
                result = lab->pVTbl->QueryInterface(lab, 0, &out);
                check(result != 0 && out == 0, "base_null_iid_clears_output");
                result = lab->pVTbl->QueryInterface(lab, &IID_IEcoSortAndSearch, (void**)&sorter);
                if (check(result == 0 && sorter != 0, "sort_interface_available")) {
                    out = sorter;
                    result = sorter->pVTbl->QueryInterface(sorter, 0, &out);
                    check(result != 0 && out == 0, "secondary_null_iid_clears_output");
                    sorter->pVTbl->Release(sorter);
                }
            }
            if (strcmp(mode, "secondary_lifetime") == 0) {
                result = lab->pVTbl->QueryInterface(lab, &IID_IEcoSortAndSearch, (void**)&sorter);
                if (check(result == 0 && sorter != 0, "secondary_reference_acquired")) {
                    lab->pVTbl->Release(lab); lab = 0;
                    check(mock.live == 1, "object_lives_while_secondary_reference_exists");
                    check(sorter->pVTbl->Configure(sorter, 1, 0, 0) == 0, "secondary_works_after_primary_release");
                    sorter->pVTbl->Release(sorter);
                }
            }
            release_interface(lab);
        }
    }
    check_balanced();
    printf("COM_SUMMARY,case=%s,checks=%u,failures=%u,refs=%u,memory_refs=%u,live=%u\n",
        mode, cases, errors, mock.refs, mock.memory_refs, mock.live);
    return errors ? 1 : 0;
}
