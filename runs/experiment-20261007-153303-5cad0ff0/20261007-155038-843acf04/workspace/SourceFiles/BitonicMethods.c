#include "ArpoMethods.h"

/* Define the StageContext structure */
typedef struct {
    const unsigned char* base;
    uint32_t width;
    uint32_t count;
    uint32_t N;
    uint32_t k;
    uint32_t j;
    BitonicCompare compare;
    uint32_t* indices;
} StageContext;

/* Static callback function for stage execution */
static void ECOCALLMETHOD stage_job(void* context, uint32_t lane, uint32_t workers) {
    StageContext* ctx = (StageContext*)context;
    uint32_t i, partner, A, B;
    int order;
    int ascending;

    /* Loop through indices in steps of 'workers' */
    for (i = lane; i < ctx->N; i += workers) {
        /* Skip if i >= partner */
        partner = i ^ ctx->j;
        if (i >= partner) continue;

        /* Compute ascending */
        ascending = (i & ctx->k) == 0;

        /* Compare indices */
        A = ctx->indices[i];
        B = ctx->indices[partner];
    
        /* Handle padding */
        if (A == ctx->count && B == ctx->count) {
            order = 0;
        } else if (A == ctx->count) {
            order = +1;
        } else if (B == ctx->count) {
            order = -1;
        } else {
            order = ctx->compare(ctx->base + A * ctx->width, ctx->base + B * ctx->width);
        }

        /* Swap indices if necessary */
        if ((ascending && order > 0) || (!ascending && order < 0)) {
            uint32_t temp = ctx->indices[i];
            ctx->indices[i] = ctx->indices[partner];
            ctx->indices[partner] = temp;
        }
    }
}

int16_t ECOCALLMETHOD lab_bitonic_sort(IEcoLab64* me, void* base, uint32_t count, uint32_t width, BitonicCompare compare, uint32_t workers, BitonicExecutor executor, void* executor_context, BitonicStats* stats) {
    IEcoMemoryAllocator1* allocator;
    uint32_t N, i, j, k, stages, pairs;
    uint32_t* indices = NULL;
    void* output = NULL;
    int16_t executor_status;
    uint32_t padding;

    /* Validate arguments */
    if (me == NULL || compare == NULL || stats == NULL) return -1;
    if (width == 0) return -1;
    if (count > 1048576) return -1;
    if (count > 0 && base == NULL) return -1;
    if (width > 0xFFFFFFFFU / count) return -1;
    if (workers == 0 || workers > 16) return -1;
    if (executor == NULL && workers > 1) return -1;

    /* Zero/singleton count */
    if (count <= 1) {
        stats->padded_count = count;
        stats->stages = 0;
        stats->network_pairs = 0;
        return 0;
    }

    /* Compute power of two */
    N = 1;
    while (N < count) {
        N <<= 1;
    }

    /* Allocate indices */
    allocator = lab_allocator(me);
    if (allocator == NULL) return -2;
    indices = (uint32_t*)allocator->pVTbl->Alloc(allocator, N * sizeof(uint32_t));
    if (indices == NULL) {
        return -2;
    }

/* Initialize statistics */
    stages = 0;
    pairs = 0;

    /* Sort */
    for (k = 2; k <= N; k <<= 1) {
        for (j = k >> 1; j > 0; j >>= 1) {
            StageContext context;
            context.base = (const unsigned char*)base;
            context.width = width;
            context.count = count;
            context.N = N;
            context.k = k;
            context.j = j;
            context.indices = indices;
            context.compare = compare;

            /* Execute stage */
            if (workers == 1) {
                stage_job(&context, 0, 1);
            } else {
                executor_status = executor(executor_context, stage_job, &context, workers);
                if (executor_status != 0) {
                    allocator->pVTbl->Free(allocator, indices);
                    return -3;
                }
            }
            pairs += N / 2;
            stages++;
        }
    }

    /* Allocate output */
    output = allocator->pVTbl->Alloc(allocator, count * width);
    if (output == NULL) {
        allocator->pVTbl->Free(allocator, indices);
        return -2;
    }

    /* Copy data */
    for (i = 0; i < count; i++) {
        allocator->pVTbl->Copy(allocator, (char*)output + i * width, (char*)base + indices[i] * width, width);
    }

    /* Copy back */
    allocator->pVTbl->Copy(allocator, (char*)base, (char*)output, count * width);

    /* Free */
    allocator->pVTbl->Free(allocator, output);
    allocator->pVTbl->Free(allocator, indices);

    /* Update stats */
    stats->padded_count = N;
    stats->stages = stages;
    stats->network_pairs = (uint64_t)pairs;

    return 0;
}
