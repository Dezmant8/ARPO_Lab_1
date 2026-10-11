#include "ArpoMethods.h"

/* Define the StageContext structure */
typedef struct {
    uint32_t* indices;
    const unsigned char* base;
    uint32_t width;
    uint32_t count;
    uint32_t N;
    uint32_t k;
    uint32_t j;
    BitonicCompare compare;
} StageContext;

/* Static stage job callback */
static void ECOCALLMETHOD stage_job(void* context, uint32_t lane, uint32_t workers) {
    StageContext* ctx = (StageContext*)context;
    uint32_t i = lane;
    uint32_t partner;
    uint32_t A, B;
    int order;
    int ascending;

    while (i < ctx->N) {
        partner = i ^ ctx->j;
        if (i >= partner) {
            i += workers;
            continue;
        }
        A = ctx->indices[i];
        B = ctx->indices[partner];
        if (A == ctx->count && B == ctx->count) {
            order = 0;
        } else if (A == ctx->count) {
            order = +1;
        } else if (B == ctx->count) {
            order = -1;
        } else {
            order = ctx->compare(ctx->base + A * ctx->width, ctx->base + B * ctx->width);
        }
        ascending = (i & ctx->k) == 0;
        if ((ascending && order > 0) || (!ascending && order < 0)) {
            ctx->indices[i] = B;
            ctx->indices[partner] = A;
        }
        i += workers;
    }
}

/* Public function implementation */
int16_t ECOCALLMETHOD lab_bitonic_sort(IEcoLab64* me, void* base, uint32_t count, uint32_t width, BitonicCompare compare, uint32_t workers, BitonicExecutor executor, void* executor_context, BitonicStats* stats) {
    IEcoMemoryAllocator1* eco_memory_allocator;
    uint32_t N;
    uint32_t i;
    uint32_t k, j;
    uint32_t stages = 0;
    uint64_t network_pairs = 0;
    uint32_t* indices = NULL;
    void* output = NULL;
    int16_t executor_status = 0;

    /* Validate arguments */
    if (me == NULL || compare == NULL || stats == NULL) {
        return -1;
    }
    if (width == 0) {
        return -1;
    }
    if (count > 1048576) {
        return -1;
    }
    if (count > 0 && width > 0xFFFFFFFFU / count) {
        return -1;
    }
    if (count > 0 && base == NULL) {
        return -1;
    }
    if (workers == 0 || workers > 16) {
        return -1;
    }
    if (workers > 1 && executor == NULL) {
        return -1;
    }

    /* Handle zero or singleton */
    if (count <= 1) {
        stats->padded_count = count;
        stats->stages = 0;
        stats->network_pairs = 0;
        return 0;
    }

    /* Compute power of two >= count */
    N = 1;
    while (N < count) {
        N <<= 1;
    }

    /* Allocate indices array */
    eco_memory_allocator = lab_allocator(me);
    if (eco_memory_allocator == NULL) {
        return -2;
    }
    indices = (uint32_t*)eco_memory_allocator->pVTbl->Alloc(eco_memory_allocator, N * sizeof(uint32_t));
    if (indices == NULL) {
        return -2;
    }

    /* Initialize indices */
    for (i = 0; i < count; i++) {
        indices[i] = i;
    }
    for (i = count; i < N; i++) {
        indices[i] = count;
    }

    /* Initialize stats */
    stats->padded_count = N;
    stats->stages = 0;
    stats->network_pairs = 0;

    /* Perform bitonic sort */
    for (k = 2; k <= N; k <<= 1) {
        for (j = k >> 1; j > 0; j >>= 1) {
            StageContext context;
            context.indices = indices;
            context.base = (const unsigned char*)base;
            context.width = width;
            context.count = count;
            context.N = N;
            context.k = k;
            context.j = j;
            context.compare = compare;

            /* Execute stage */
            if (workers == 1) {
                stage_job(&context, 0, 1);
            } else {
                executor_status = executor(executor_context, stage_job, &context, workers);
                if (executor_status != 0) {
                    eco_memory_allocator->pVTbl->Free(eco_memory_allocator, indices);
                    return -3;
                }
            }
            stats->stages++;
            stats->network_pairs += (N / 2);
        }
    }

    /* Allocate output buffer */
    output = eco_memory_allocator->pVTbl->Alloc(eco_memory_allocator, count * width);
    if (output == NULL) {
        eco_memory_allocator->pVTbl->Free(eco_memory_allocator, indices);
        return -2;
    }

/* Copy sorted elements to output */
    for (i = 0; i < count; i++) {
        eco_memory_allocator->pVTbl->Copy(eco_memory_allocator, (char*)output + i * width, (char*)base + indices[i] * width, width);
    }

    /* Copy output back to base */
    eco_memory_allocator->pVTbl->Copy(eco_memory_allocator, base, output, count * width);

    /* Clean up */
    eco_memory_allocator->pVTbl->Free(eco_memory_allocator, indices);
    eco_memory_allocator->pVTbl->Free(eco_memory_allocator, output);

    return 0;
}
