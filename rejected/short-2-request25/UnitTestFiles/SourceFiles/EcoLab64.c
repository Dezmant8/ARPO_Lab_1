#include <stdio.h>
#include "IEcoSystem1.h"
#include "IdEcoInterfaceBus1.h"
#include "IdEcoLab64.h"
#include "ParallelPool.h"
#include "IEcoSortAndSearch.h"
static int ECOCALLMETHOD cmp(const void* a,const void* b) {
    int32_t x=*(const int32_t*)a, y=*(const int32_t*)b;
    return (x>y)-(x<y);
}
int16_t EcoMain(IEcoUnknown* system) {
    IEcoSystem1* sys=0;
    IEcoInterfaceBus1* bus=0;
    IEcoLab64* lab=0;
    IEcoSortAndSearch* sorter=0;
    int32_t a[7]={8,-3,0,9,1,1,-7};
    ParallelPool pool;
    BitonicStats stats;
    int16_t result=0;
    uint32_t i;
    if (!system || system->pVTbl->QueryInterface(system,&GID_IEcoSystem,(void**)&sys)!=0 || !sys) return 1;
    if (sys->pVTbl->QueryInterface(sys,&IID_IEcoInterfaceBus1,(void**)&bus)!=0 || !bus) return 2;
    if (bus->pVTbl->RegisterComponent(bus,&CID_EcoLab64,(IEcoUnknown*)GetIEcoComponentFactoryPtr_AEBA731EF9FB4A7B8B40055A97DD0B7B)!=0
        || bus->pVTbl->QueryComponent(bus,&CID_EcoLab64,0,&IID_IEcoLab64,(void**)&lab)!=0 || !lab) return 3;
    if (pool_init(&pool,4)!=0) return 4;
    result=lab->pVTbl->QueryInterface(lab,&IID_IEcoSortAndSearch,(void**)&sorter);
    if(!result && sorter) {
        result=sorter->pVTbl->Configure(sorter,4,pool_execute,&pool);
        if(!result) {
            sorter->pVTbl->Sort(sorter,a,7,sizeof(int32_t),cmp);
            result=sorter->pVTbl->GetResult(sorter,&stats);
        }
        sorter->pVTbl->Release(sorter);
    } else result=6;
    for(i=1;i<7;i++) if(a[i-1]>a[i]) result=5;
    printf("status=%d padded=%u stages=%u network_pairs=%lu workers=4\n",result,stats.padded_count,stats.stages,(unsigned long)stats.network_pairs);
    for(i=0;i<7;i++) printf("%d%s",a[i],i==6?"\n":",");
    pool_destroy(&pool);
    lab->pVTbl->Release(lab); bus->pVTbl->Release(bus); sys->pVTbl->Release(sys);
    return result;
}
