/* Static Eco factory. Instance and factory lifetimes are separate. */
#include "CEcoLab64.h"
#include "CEcoLab64Factory.h"

static uint32_t ECOCALLMETHOD factory_addref(IEcoComponentFactory* me) {
    CEcoLab64_97DD0B7BFactory* factory = (CEcoLab64_97DD0B7BFactory*)me;
    if (!factory) return 0;
    return ++factory->m_cRef;
}
static uint32_t ECOCALLMETHOD factory_release(IEcoComponentFactory* me) {
    CEcoLab64_97DD0B7BFactory* factory = (CEcoLab64_97DD0B7BFactory*)me;
    if (!factory || !factory->m_cRef) return 0;
    /* Static factory storage remains when no clients hold references. */
    return --factory->m_cRef;
}
static int16_t ECOCALLMETHOD factory_query(IEcoComponentFactory* me, const UGUID* iid, void** out) {
    if (!out) return ERR_ECO_POINTER;
    *out = 0;
    if (!me || !iid) return ERR_ECO_POINTER;
    if (!IsEqualUGUID(iid, &IID_IEcoUnknown) && !IsEqualUGUID(iid, &IID_IEcoComponentFactory))
        return ERR_ECO_NOINTERFACE;
    *out = me;
    factory_addref(me);
    return 0;
}
static int16_t ECOCALLMETHOD factory_alloc(IEcoComponentFactory* me, IEcoUnknown* system,
        IEcoUnknown* outer, const UGUID* iid, void** out) {
    if (!out) return ERR_ECO_POINTER;
    *out = 0;
    if (!me) return ERR_ECO_POINTER;
    return createCEcoLab64_97DD0B7B(system, outer, iid, out);
}
static int16_t ECOCALLMETHOD factory_init(IEcoComponentFactory* me, IEcoUnknown* system, void* value) {
    (void)value;
    /* Alloc returns a fully initialized object. */
    return me && system ? ERR_ECO_SUCCESS : ERR_ECO_POINTER;
}
static char_t* ECOCALLMETHOD factory_name(IEcoComponentFactory* me) {
    return me ? ((CEcoLab64_97DD0B7BFactory*)me)->m_Name : 0;
}
static char_t* ECOCALLMETHOD factory_version(IEcoComponentFactory* me) {
    return me ? ((CEcoLab64_97DD0B7BFactory*)me)->m_Version : 0;
}
static char_t* ECOCALLMETHOD factory_manufacturer(IEcoComponentFactory* me) {
    return me ? ((CEcoLab64_97DD0B7BFactory*)me)->m_Manufacturer : 0;
}
static IEcoComponentFactoryVTbl g_factory_vtable = {
    factory_query, factory_addref, factory_release, factory_alloc, factory_init,
    factory_name, factory_version, factory_manufacturer
};
static CEcoLab64_97DD0B7BFactory g_factory = {
    &g_factory_vtable, 0, "EcoLab64", "1.0.0.1", ""
};
#ifdef ECO_DLL
ECO_EXPORT IEcoComponentFactory* ECOCALLMETHOD GetIEcoComponentFactoryPtr(void) {
    return (IEcoComponentFactory*)&g_factory;
}
#elif ECO_LIB
IEcoComponentFactory* GetIEcoComponentFactoryPtr_AEBA731EF9FB4A7B8B40055A97DD0B7B =
    (IEcoComponentFactory*)&g_factory;
#endif
