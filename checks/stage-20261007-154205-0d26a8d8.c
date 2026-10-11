/* Independent unit client for Qwen's generated stage callback only. */
#define lab_bitonic_sort callback_probe_stub
#include "/Users/egorbulanov/Documents/GitHub/ARPO_solver_macos/labs/bitonic-64-20261007/runs/experiment-20261007-153303-5cad0ff0/20261007-154205-0d26a8d8/workspace/SourceFiles/BitonicMethods.c"
#undef lab_bitonic_sort
#include <stdio.h>
#include <stdlib.h>
#include "ParallelPool.h"
static int32_t* original;
static uint32_t real_count;
static unsigned long comparator_errors=0;
static int ECOCALLMETHOD reference_compare(const void* a,const void* b) {
    int32_t x=*(const int32_t*)a,y=*(const int32_t*)b;
    return x<y?(-2147483647-1):(x>y?2147483647:0);
}
static int ECOCALLMETHOD probe_compare(const void* a,const void* b) {
    unsigned long pa=(unsigned long)a,pb=(unsigned long)b;
    unsigned long start=(unsigned long)original,end=start+real_count*sizeof(int32_t);
    int32_t x,y;
    if(pa<start || pb<start || pa>=end || pb>=end) { comparator_errors++; return 0; }
    x=*(const int32_t*)a; y=*(const int32_t*)b;
    return x<y?(-2147483647-1):(x>y?2147483647:0);
}
int16_t EcoMain(IEcoUnknown* system) {
    int32_t data[8],expected[8];
    uint32_t storage[18],used[8];
    uint32_t n,N,i,k,j,p,mask,cases=0,failures=0;
    int ok;
    StageContext context;
    ParallelPool pool;
    (void)system;
    if(pool_init(&pool,4)) return 2;
    for(n=0;n<=8;n++) for(mask=0;mask<(1U<<n);mask++) for(p=1;p<=4;p*=4) {
        for(i=0;i<n;i++) { data[i]=(mask>>i)&1; expected[i]=data[i]; used[i]=0; }
        original=data; real_count=n;
        qsort(expected,n,sizeof(int32_t),reference_compare);
        N=1; while(N<n) N*=2;
        storage[0]=123456; storage[N+1]=654321;
        for(i=0;i<N;i++) storage[i+1]=i<n?i:n;
        context.indices=storage+1; context.base=(const unsigned char*)data;
        context.width=sizeof(int32_t); context.count=n; context.N=N; context.compare=probe_compare;
        for(k=2;k<=N;k*=2) for(j=k/2;j;j/=2) {
            context.k=k; context.j=j;
            if(p==1) stage_job(&context,0,1); else if(pool_execute(&pool,stage_job,&context,p)) return 3;
        }
        ok=storage[0]==123456 && storage[N+1]==654321;
        for(i=0;i<n;i++) {
            if(context.indices[i]>=n) ok=0;
            else {
                if(data[context.indices[i]]!=expected[i] || used[context.indices[i]]) ok=0;
                used[context.indices[i]]=1;
            }
        }
        for(i=n;i<N;i++) if(context.indices[i]!=n) ok=0;
        cases++;
        if(!ok) { if(failures<12) printf("STAGE_FAIL,n=%u,mask=%u,workers=%u\n",n,mask,p); failures++; }
    }
    pool_destroy(&pool);
    printf("STAGE_SUMMARY,cases=%u,failures=%u,padding_comparator_calls=%lu\n",cases,failures,comparator_errors);
    return failures || comparator_errors?5:0;
}
