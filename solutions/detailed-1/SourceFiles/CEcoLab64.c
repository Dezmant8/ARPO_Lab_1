#include "ArpoMethods.h"
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
 *   This source code describes the implementation of the interfaces for CEcoLab64_97DD0B7B
 * </description>
 *
 * <author>
 *   Copyright (c) 2026 . All rights reserved.
 * </author>
 *
 */


#include "IEcoSystem1.h"
#include "IEcoInterfaceBus1.h"
#include "IEcoInterfaceBus1MemExt.h"
#include "CEcoLab64.h"
/*
 *
 * <summary>
 *   QueryInterface Function
 * </summary>
 *
 * <description>
 *   QueryInterface function for the IEcoLab64 interface
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoLab64_97DD0B7B_QueryInterface(/* in */ IEcoLab64Ptr_t me, /* in */ const UGUID* riid, /* out */ void** ppv) {
    CEcoLab64_97DD0B7B* pCMe = (CEcoLab64_97DD0B7B*)me;

    /* Pointer Validation */
    if (me == 0 || ppv == 0 || riid == 0) {
        return ERR_ECO_POINTER;
    }

    /* Validate and retrieve requested interface */
    if ( IsEqualUGUID(riid, &IID_IEcoLab64) ) {
        *ppv = &pCMe->m_pVTblIEcoLab64;
        pCMe->m_pVTblIEcoLab64->AddRef((IEcoLab64*)pCMe);
    }
	
    else if ( IsEqualUGUID(riid, &IID_IEcoSortAndSearch) ) {
        *ppv = &pCMe->m_pVTblSort;
        pCMe->m_pVTblIEcoLab64->AddRef((IEcoLab64*)pCMe);
    }
    else if ( IsEqualUGUID(riid, &IID_IEcoUnknown) ) {
        *ppv = &pCMe->m_pVTblIEcoLab64;
        pCMe->m_pVTblIEcoLab64->AddRef((IEcoLab64*)pCMe);
    }
    else {
        *ppv = 0;
        return ERR_ECO_NOINTERFACE;
    }
    return ERR_ECO_SUCCESS;
}

/*
 *
 * <summary>
 *   AddRef Function
 * </summary>
 *
 * <description>
 *   AddRef function for the IEcoLab64 interface
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoLab64_97DD0B7B_AddRef(/* in */ IEcoLab64Ptr_t me) {
    CEcoLab64_97DD0B7B* pCMe = (CEcoLab64_97DD0B7B*)me;

    /* Pointer Validation */
    if (me == 0 ) {
        return -1; /* ERR_ECO_POINTER */
    }

    return ++pCMe->m_cRef;
}

/*
 *
 * <summary>
 *   Release Function
 * </summary>
 *
 * <description>
 *   Release function for the IEcoLab64 interface
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoLab64_97DD0B7B_Release(/* in */ IEcoLab64Ptr_t me) {
    CEcoLab64_97DD0B7B* pCMe = (CEcoLab64_97DD0B7B*)me;

    /* Pointer Validation */
    if (me == 0 ) {
        return -1; /* ERR_ECO_POINTER */
    }

    /* Decrementing the component's reference count */
    --pCMe->m_cRef;
    /* If the count is zero, free the instance data */
    if ( pCMe->m_cRef == 0 ) {
        pCMe->Delete(pCMe);
		
        return 0;
    }
    return pCMe->m_cRef;
}

/*
 *
 * <summary>
 *   MyFunction Function
 * </summary>
 *
 * <description>
 *   Function
 * </description>
 *
 */





/*
 *
 * <summary>
 *   Init Function
 * </summary>
 *
 * <description>
 *   Instance initialization function
 * </description>
 *
 */
static int16_t ECOCALLMETHOD initCEcoLab64_97DD0B7B(/*in*/ CEcoLab64_97DD0B7BPtr_t me, /* in */ IEcoUnknownPtr_t pIUnkSystem) {
    CEcoLab64_97DD0B7B* pCMe = (CEcoLab64_97DD0B7B*)me;
    IEcoInterfaceBus1* pIBus = 0;

    IEcoInterfaceBus1MemExt* pIMemExt = 0;
    int16_t result = ERR_ECO_POINTER;
    UGUID* rcid = (UGUID*)&CID_EcoMemoryManager1;	

    /* Pointer Validation */
    if (me == 0 ) {
        return result;
    }

    /* Storing the pointer to the system interface */
    pCMe->m_pISys = (IEcoSystem1*)pIUnkSystem;

    /* Getting the interface for working with the interface bus */
    result = pCMe->m_pISys->pVTbl->QueryInterface(pCMe->m_pISys, &IID_IEcoInterfaceBus1, (void **)&pIBus);

    /* Getting the component ID for working with memory */
    result = pIBus->pVTbl->QueryInterface(pIBus, &IID_IEcoInterfaceBus1MemExt, (void**)&pIMemExt);
    if (result == 0 && pIMemExt != 0) {
        rcid = (UGUID*)pIMemExt->pVTbl->get_Manager(pIMemExt);
        pIMemExt->pVTbl->Release(pIMemExt);
    }

    /* Getting the memory allocator interface */
    result = pIBus->pVTbl->QueryComponent(pIBus, rcid, 0, &IID_IEcoMemoryAllocator1, (void**) &pCMe->m_pIMem);
    /* Check */
    if (result != 0 || pCMe->m_pIMem == 0) {
        result = ERR_ECO_GET_MEMORY_ALLOCATOR;
    }



    /* Freeing */
    pIBus->pVTbl->Release(pIBus);

    return result;
}

/*
 *
 * <summary>
 *   Create Function
 * </summary>
 *
 * <description>
 *   Instance creation function
 * </description>
 *
 */
static int16_t ECOCALLMETHOD createCEcoLab64_97DD0B7B(/* in */ CEcoLab64_97DD0B7BPtr_t pCMe, /* in */ IEcoUnknownPtr_t pIUnkSystem, /* in */ IEcoUnknownPtr_t pIUnkOuter) {
    int16_t result = ERR_ECO_POINTER;

    /* Pointer Validation */
    if (pCMe == 0) {
        return result; /* ERR_ECO_POINTER */
    }



    return ERR_ECO_SUCCESS;
}

/*
 *
 * <summary>
 *   Delete Function
 * </summary>
 *
 * <description>
 *   Instance freeing function
 * </description>
 *
 */
static void ECOCALLMETHOD deleteCEcoLab64_97DD0B7B(/* in */ CEcoLab64_97DD0B7BPtr_t pCMe) {
    IEcoMemoryAllocator1* pIMem = 0;

    if (pCMe != 0 ) {
        pIMem = pCMe->m_pIMem;
        /* Freeing */
        if ( pCMe->m_Name != 0 ) {
            pIMem->pVTbl->Free(pIMem, pCMe->m_Name);
        }
        if ( pCMe->m_pISys != 0 ) {
            pCMe->m_pISys->pVTbl->Release(pCMe->m_pISys);
        }
        pIMem->pVTbl->Free(pIMem, pCMe);
        pIMem->pVTbl->Release(pIMem);
    }
}

/* IEcoLab64 Virtual Table */
IEcoLab64VTbl g_x63D20705ABDB4E2B861802C857FD994DVTbl_97DD0B7B = {
    CEcoLab64_97DD0B7B_QueryInterface,
    CEcoLab64_97DD0B7B_AddRef,
    CEcoLab64_97DD0B7B_Release,
    lab_bitonic_sort
};



/* Object Instance */
CEcoLab64_97DD0B7B g_xCEcoLab64_97DD0B7B = {
    &g_x63D20705ABDB4E2B861802C857FD994DVTbl_97DD0B7B,
   
    initCEcoLab64_97DD0B7B,
    createCEcoLab64_97DD0B7B,
    deleteCEcoLab64_97DD0B7B,
    1, /* m_cRef */
    0, /* m_pISys */
    0, /* m_pISys */
    0, /* m_Name */
    &g_sort_vtable, 1, 0, 0, 0, {0,0,0}
};

IEcoMemoryAllocator1* ECOCALLMETHOD lab_allocator(IEcoLab64* me) {
    if (me == 0) return 0;
    return ((CEcoLab64_97DD0B7B*)me)->m_pIMem;
}
