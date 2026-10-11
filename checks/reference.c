/* Independent host client: libc is allowed here, not in the component. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <time.h>
#include "IEcoSystem1.h"
#include "IdEcoInterfaceBus1.h"
#include "IdEcoLab64.h"
#include "CEcoLab64.h"
#include "ParallelPool.h"

static unsigned long cases = 0;
static unsigned long errors = 0;
static uint32_t rng_state = 173U;
static int check(int condition, const char* name) {
    cases++;
    if (!condition) { printf("FAIL,%s\n", name); errors++; }
    return condition;
}
static uint32_t random_u32(void) {
    rng_state = rng_state * 1664525U + 1013904223U;
    return rng_state;
}
static int ECOCALLMETHOD cmp_i32(const void* a, const void* b) {
    int32_t x = *(const int32_t*)a, y = *(const int32_t*)b;
    return (x > y) - (x < y);
}
static int ECOCALLMETHOD cmp_desc(const void* a, const void* b) { return cmp_i32(b,a); }
static int ECOCALLMETHOD cmp_i64(const void* a, const void* b) {
    int64_t x = *(const int64_t*)a, y = *(const int64_t*)b;
    return (x > y) - (x < y);
}
static int ECOCALLMETHOD cmp_f(const void* a, const void* b) {
    float x = *(const float*)a, y = *(const float*)b;
    return (x > y) - (x < y);
}
static int ECOCALLMETHOD cmp_d(const void* a, const void* b) {
    double x = *(const double*)a, y = *(const double*)b;
    return (x > y) - (x < y);
}
static int ECOCALLMETHOD cmp_ld(const void* a, const void* b) {
    long double x = *(const long double*)a, y = *(const long double*)b;
    return (x > y) - (x < y);
}
static int ECOCALLMETHOD cmp_byte_record(const void* a, const void* b) {
    const unsigned char* x = (const unsigned char*)a;
    const unsigned char* y = (const unsigned char*)b;
    return (x[0] > y[0]) - (x[0] < y[0]);
}
static int verify_sort(IEcoLab64* lab, ParallelPool* pool, const void* input,
        uint32_t n, uint32_t width, BitonicCompare compare, uint32_t workers,
        const char* name, int compare_bytes) {
    unsigned char* actual;
    unsigned char* reference;
    BitonicStats stats;
    uint32_t i, j, padded = n, depth = 0, stages = 0, calls;
    uint64_t pairs = 0;
    int16_t status;
    int ok;
    size_t bytes = (size_t)n * width;
    actual = (unsigned char*)malloc(bytes + 32);
    reference = (unsigned char*)malloc(bytes ? bytes : 1);
    if (!actual || !reference) exit(90);
    memset(actual, 0xA5, bytes + 32);
    if (bytes) { memcpy(actual + 16, input, bytes); memcpy(reference,input,bytes); }
    qsort(reference,n,width,compare);
    if (n > 1) {
        padded = 1;
        while (padded < n) { padded *= 2; depth++; }
        stages = depth * (depth+1) / 2;
        pairs = (uint64_t)(padded/2) * stages;
    }
    calls = pool->calls;
    status = lab->pVTbl->Sort(lab,n ? actual+16 : 0,n,width,compare,workers,
        workers > 1 ? pool_execute : 0, pool, &stats);
    ok = status == 0 && stats.padded_count == padded && stats.stages == stages
        && stats.network_pairs == pairs && pool->calls-calls == (workers>1 ? stages:0);
    for (i=0; i<16; i++)
        if (actual[i] != 0xA5 || actual[16+bytes+i] != 0xA5) ok=0;
    if (compare_bytes && bytes && memcmp(actual+16,reference,bytes) != 0) ok=0;
    for (i=1; i<n; i++)
        if (compare(actual+16+(i-1)*width, actual+16+i*width)>0) ok=0;
    /* Generic multiset check independent of the ordering of equal-key records. */
    if (!compare_bytes) {
        unsigned char* used = (unsigned char*)calloc(n ? n:1,1);
        if (!used) exit(91);
        for (i=0; i<n; i++) {
            for (j=0; j<n; j++) if (!used[j] &&
                memcmp(actual+16+i*width,(const unsigned char*)input+j*width,width)==0) break;
            if (j==n) ok=0; else used[j]=1;
        }
        free(used);
    }
    if (!ok) printf("DETAIL,%s,n=%u,width=%u,workers=%u,status=%d,padded=%u,stages=%u,pairs=%lu,calls=%u\n",
        name,n,width,workers,status,stats.padded_count,stats.stages,
        (unsigned long)stats.network_pairs,pool->calls-calls);
    check(ok,name);
    free(actual); free(reference);
    return ok;
}

typedef struct TrackingAllocator {
    IEcoMemoryAllocator1VTbl* pVTbl;
    IEcoMemoryAllocator1* real;
    unsigned int allocations;
    unsigned int frees;
    unsigned int live;
    unsigned int fail_at;
} TrackingAllocator;
static voidptr_t ECOCALLMETHOD track_alloc(IEcoMemoryAllocator1* me,uint32_t size) {
    TrackingAllocator* m = (TrackingAllocator*)me;
    void* p;
    m->allocations++;
    if (m->fail_at && m->allocations==m->fail_at) return 0;
    p=m->real->pVTbl->Alloc(m->real,size);
    if (p) m->live++;
    return p;
}
static void ECOCALLMETHOD track_free(IEcoMemoryAllocator1* me,voidptr_t p) {
    TrackingAllocator* m = (TrackingAllocator*)me;
    if (p) { m->frees++; m->live--; m->real->pVTbl->Free(m->real,p); }
}
static voidptr_t ECOCALLMETHOD track_copy(IEcoMemoryAllocator1* me,voidptr_t dst,voidptr_t src,uint32_t size) {
    TrackingAllocator* m = (TrackingAllocator*)me;
    return m->real->pVTbl->Copy(m->real,dst,src,size);
}
static voidptr_t ECOCALLMETHOD track_fill(IEcoMemoryAllocator1* me,voidptr_t dst,char_t value,uint32_t size) {
    TrackingAllocator* m = (TrackingAllocator*)me;
    return m->real->pVTbl->Fill(m->real,dst,value,size);
}
static void test_failures(IEcoLab64* lab, ParallelPool* pool) {
    int32_t values[8] = {8,7,6,5,4,3,2,1};
    int32_t saved[8];
    BitonicStats stats;
    uint32_t i;
    int16_t status;
    CEcoLab64_97DD0B7B* object = (CEcoLab64_97DD0B7B*)lab;
    TrackingAllocator tracking;
    IEcoMemoryAllocator1VTbl vtable;
    memcpy(saved,values,sizeof(values));
    check(lab->pVTbl->Sort(0,values,8,4,cmp_i32,1,0,0,&stats)==-1,"null_me");
    check(lab->pVTbl->Sort(lab,0,8,4,cmp_i32,1,0,0,&stats)==-1,"null_base");
    check(lab->pVTbl->Sort(lab,values,8,0,cmp_i32,1,0,0,&stats)==-1,"zero_width");
    check(lab->pVTbl->Sort(lab,values,8,4,0,1,0,0,&stats)==-1,"null_comparator");
    check(lab->pVTbl->Sort(lab,values,8,4,cmp_i32,1,0,0,0)==-1,"null_stats");
    check(lab->pVTbl->Sort(lab,values,8,4,cmp_i32,0,0,0,&stats)==-1,"zero_workers");
    check(lab->pVTbl->Sort(lab,values,8,4,cmp_i32,17,0,0,&stats)==-1,"too_many_workers");
    check(lab->pVTbl->Sort(lab,values,8,4,cmp_i32,4,0,0,&stats)==-1,"missing_executor");
    check(lab->pVTbl->Sort(lab,values,1048577,4,cmp_i32,1,0,0,&stats)==-1,"count_limit");
    check(lab->pVTbl->Sort(lab,values,8,0xFFFFFFFFU,cmp_i32,1,0,0,&stats)==-1,"size_overflow");
    check(memcmp(values,saved,sizeof(values))==0,"invalid_arguments_preserve_input");
    pool->fail_at=pool->calls+3;
    status=lab->pVTbl->Sort(lab,values,8,4,cmp_i32,4,pool_execute,pool,&stats);
    check(status==-3 && memcmp(values,saved,sizeof(values))==0,"executor_failure_preserves_input");
    pool->fail_at=0;
    memset(&tracking,0,sizeof(tracking));
    tracking.real=object->m_pIMem;
    vtable=*tracking.real->pVTbl;
    vtable.Alloc=track_alloc; vtable.Free=track_free; vtable.Copy=track_copy; vtable.Fill=track_fill;
    tracking.pVTbl=&vtable;
    object->m_pIMem=(IEcoMemoryAllocator1*)&tracking;
    for(i=1;i<=2;i++) {
        tracking.allocations=0; tracking.frees=0; tracking.live=0; tracking.fail_at=i;
        status=lab->pVTbl->Sort(lab,values,8,4,cmp_i32,1,0,0,&stats);
        check(status==-2 && tracking.live==0 && memcmp(values,saved,sizeof(values))==0,
            i==1 ? "first_allocation_failure":"second_allocation_failure");
    }
    tracking.allocations=0; tracking.frees=0; tracking.live=0; tracking.fail_at=0;
    status=lab->pVTbl->Sort(lab,values,8,4,cmp_i32,1,0,0,&stats);
    check(status==0 && tracking.allocations>=2 && tracking.live==0,"success_frees_memory");
    tracking.allocations=0; tracking.live=0;
    status=lab->pVTbl->Sort(lab,0,0,4,cmp_i32,1,0,0,&stats);
    check(status==0 && tracking.allocations==0,"empty_no_allocation");
    status=lab->pVTbl->Sort(lab,values,1,4,cmp_i32,1,0,0,&stats);
    check(status==0 && tracking.allocations==0,"singleton_no_allocation");
    object->m_pIMem=tracking.real;
}
static void tests(IEcoLab64* lab,ParallelPool* pool) {
    int32_t values[4097];
    int64_t longs[11]={0,1,-1,2147483647L,-2147483647L,9223372036854775807L,
        (-9223372036854775807L-1L),10,-10,100,-100};
    float floats[11]={0.0f,-0.0f,1.0f,-1.0f,FLT_MAX,-FLT_MAX,FLT_MIN,-FLT_MIN,1.25f,-1.25f,99.0f};
    double doubles[11]={0.0,-0.0,1.0,-1.0,DBL_MAX,-DBL_MAX,DBL_MIN,-DBL_MIN,1.25,-1.25,99.0};
    long double extended[11]={0.0L,-0.0L,1.0L,-1.0L,LDBL_MAX,-LDBL_MAX,LDBL_MIN,-LDBL_MIN,1.25L,-1.25L,99.0L};
    unsigned char records[37][13];
    uint32_t n, mask, i, k, workers, pattern;
    void* queried = 0;
    UGUID unknown=IID_IEcoLab64;
    ((unsigned char*)&unknown)[sizeof(unknown)-1] ^= 1;
    check(lab->pVTbl->QueryInterface(lab,&IID_IEcoLab64,&queried)==0 && queried==lab,"QueryInterface_known");
    if(queried) ((IEcoLab64*)queried)->pVTbl->Release((IEcoLab64*)queried);
    queried=(void*)lab;
    check(lab->pVTbl->QueryInterface(lab,&unknown,&queried)!=0 && !queried,"QueryInterface_unknown");
    check(lab->pVTbl->QueryInterface(lab,&IID_IEcoLab64,0)!=0,"QueryInterface_null_out");
    check(lab->pVTbl->AddRef(lab)==2 && lab->pVTbl->Release(lab)==1,"AddRef_Release");
    test_failures(lab,pool);
    for(n=0;n<=8;n++) for(mask=0;mask<(1U<<n);mask++) {
        for(i=0;i<n;i++) values[i]=(mask>>i)&1U;
        verify_sort(lab,pool,values,n,4,cmp_i32,1,"zero_one_sequential",1);
        verify_sort(lab,pool,values,n,4,cmp_i32,4,"zero_one_parallel",1);
    }
    for(n=0;n<=257;n++) {
        for(i=0;i<n;i++) values[i]=(int32_t)(random_u32()%2001U)-1000;
        verify_sort(lab,pool,values,n,4,cmp_i32,1,"random_all_lengths_sequential",1);
        verify_sort(lab,pool,values,n,4,cmp_i32,4,"random_all_lengths_parallel",1);
    }
    for(pattern=0;pattern<5;pattern++) {
        for(i=0;i<4097;i++) {
            if(pattern==0) values[i]=(int32_t)i;
            else if(pattern==1) values[i]=4097-(int32_t)i;
            else if(pattern==2) values[i]=7;
            else if(pattern==3) values[i]=(i%2)?2147483647:(-2147483647-1);
            else values[i]=(int32_t)(random_u32()%100000U)-50000;
        }
        for(workers=1;workers<=8;workers*=2) {
            verify_sort(lab,pool,values,4097,4,cmp_i32,workers,"large_patterns",1);
            verify_sort(lab,pool,values,4093,4,cmp_desc,workers,"descending_comparator",1);
        }
    }
    for(workers=1;workers<=4;workers*=4) {
        verify_sort(lab,pool,longs,11,sizeof(int64_t),cmp_i64,workers,"int64_extremes",1);
        verify_sort(lab,pool,floats,11,sizeof(float),cmp_f,workers,"float_extremes",0);
        verify_sort(lab,pool,doubles,11,sizeof(double),cmp_d,workers,"double_extremes",0);
        verify_sort(lab,pool,extended,11,sizeof(long double),cmp_ld,workers,"long_double_extremes",0);
        for(i=0;i<37;i++) { records[i][0]=(unsigned char)(i%7); for(k=1;k<13;k++) records[i][k]=(unsigned char)(i*13+k); }
        verify_sort(lab,pool,records,37,13,cmp_byte_record,workers,"records_13_bytes_duplicate_keys",0);
    }
    for(i=0;i<8;i++) check(pool->jobs[i]>0,"worker_executed_jobs");
    for(i=0;i<8;i++) for(k=i+1;k<8;k++) check(!pthread_equal(pool->threads[i],pool->threads[k]),"distinct_worker_threads");
    printf("SUMMARY,cases=%lu,failures=%lu,executor_calls=%u,workers=8\n",cases,errors,pool->calls);
}
static double wall_time(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC,&t);
    return (double)t.tv_sec+(double)t.tv_nsec*1e-9;
}
static void benchmark(IEcoLab64* lab,ParallelPool* pool) {
    uint32_t sizes[4]={1024,8192,65536,262144};
    const char* patterns[4]={"random","sorted","reverse","equal"};
    const char* methods[3]={"bitonic_seq","bitonic_p4","qsort"};
    int32_t* input;
    int32_t* actual;
    int32_t* expected;
    uint32_t s,pattern,rep,method,i,n;
    double start,elapsed;
    int16_t status;
    BitonicStats stats;
    input=(int32_t*)malloc(262144*sizeof(int32_t));
    actual=(int32_t*)malloc(262144*sizeof(int32_t));
    expected=(int32_t*)malloc(262144*sizeof(int32_t));
    if(!input || !actual || !expected) exit(92);
    puts("BENCH_HEADER,n,pattern,method,repeat,wall_s,exact");
    for(s=0;s<4;s++) for(pattern=0;pattern<4;pattern++) {
        n=sizes[s];
        rng_state=12345U+n;
        for(i=0;i<n;i++) {
            if(pattern==0) input[i]=(int32_t)(random_u32()%1000001U)-500000;
            else if(pattern==1) input[i]=(int32_t)i;
            else if(pattern==2) input[i]=(int32_t)(n-i);
            else input[i]=42;
        }
        memcpy(expected,input,n*sizeof(int32_t)); qsort(expected,n,sizeof(int32_t),cmp_i32);
        /* One unmeasured warmup and seven measurements. Rotate method order. */
        for(rep=0;rep<8;rep++) for(i=0;i<3;i++) {
            method=(i+rep)%3;
            memcpy(actual,input,n*sizeof(int32_t)); status=0;
            start=wall_time();
            if(method==2) qsort(actual,n,sizeof(int32_t),cmp_i32);
            else status=lab->pVTbl->Sort(lab,actual,n,sizeof(int32_t),cmp_i32,
                method==0?1:4,method==0?0:pool_execute,pool,&stats);
            elapsed=wall_time()-start;
            if(status || memcmp(actual,expected,n*sizeof(int32_t))) { puts("BENCH_FAIL"); errors++; }
            if(rep) printf("BENCH,%u,%s,%s,%u,%.9f,%d\n",n,patterns[pattern],methods[method],rep,elapsed,
                status==0 && memcmp(actual,expected,n*sizeof(int32_t))==0);
        }
    }
    free(input); free(actual); free(expected);
}
int16_t EcoMain(IEcoUnknown* system) {
    IEcoSystem1* sys=0;
    IEcoInterfaceBus1* bus=0;
    IEcoLab64* lab=0;
    ParallelPool pool;
    int16_t result;
    if(!system || system->pVTbl->QueryInterface(system,&GID_IEcoSystem,(void**)&sys)!=0 || !sys) return 1;
    if(sys->pVTbl->QueryInterface(sys,&IID_IEcoInterfaceBus1,(void**)&bus)!=0 || !bus) return 2;
    if(bus->pVTbl->RegisterComponent(bus,&CID_EcoLab64,(IEcoUnknown*)GetIEcoComponentFactoryPtr_AEBA731EF9FB4A7B8B40055A97DD0B7B)!=0
        || bus->pVTbl->QueryComponent(bus,&CID_EcoLab64,0,&IID_IEcoLab64,(void**)&lab)!=0 || !lab) return 3;
    if(pool_init(&pool,8)!=0) return 4;
    if(getenv("ARPO_BITONIC_BENCH")) benchmark(lab,&pool); else tests(lab,&pool);
    pool_destroy(&pool);
    result=errors?5:0;
    lab->pVTbl->Release(lab); bus->pVTbl->Release(bus); sys->pVTbl->Release(sys);
    return result;
}
