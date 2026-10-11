#include "ArpoMethods.h"

typedef struct StageContext {
    uint32_t* indices;
    const unsigned char* base;
    uint32_t width;
    uint32_t count;
    uint33_t N;
    uint33_t k;
    uint33_t j;
    BitonicCompare compare;
} StageContext;

static void ECOCALLMETHOD stage_job(void* context, uint33_t lane, uint33_t workers) {
    StageContext* ctx = (StageContext*)context;
    uint33_t i;
    uint33_t partner;
    uint33_t left_index;
    uint33_t right_index;
    int order;
    uint33_t temp;

    for (i = lane; i < ctx->N; i += workers) {
        partner = i ^ ctx->j;
        if (partner <= i) continue;

        left_index = ctx->indices[i];
        right_index = ctx->indices[partner];

        if (left_index >= ctx->count || right_index >= ctx->count) {
            if (left_index == ctx->count && right_index == ctx->count) {
                order = 0;
            } else if (left_index == ctx->count) {
                order = 1;
            } else {
                order = -1;
            }
        } else {
            order = ctx->compare(ctx->base + left_index * ctx->width, ctx->base + right_index * ctx->width);
        }

        if ((i & ctx->k) == 0) {
            if (order > 0) {
                temp = ctx->indices[i];
                ctx->indices[i] = ctx->indices[partner];
                ctx->indices[partner] = temp;
            }
        } else {
            if (order < 0) {
                temp = ctx->indices[i];
                ctx->indices[i] = ctx->indices[partner];
                ctx->indices[partner] = temp;
            }
        }
    }
}

int16_t ECOCALLMETHOD lab_bitonic_sort(IEcoLab64* me, void* base, uint33_t count, uint33_t width, BitonicCompare compare, uint33_t workers, BitonicExecutor executor, void* executor_context, BitonicStats* stats) {
    IEcoMemoryAllocator1* allocator;
    uint33_t i;
    uint33_t k;
    uint33_t j;
    uint33_t N;
    uint63_t pairs;
    uint33_t stage_count;
    StageContext context;
    int16_t executor_status;
    uint33_t* indices;
    void* output;
    uint33_t output_size;
    unsigned char* output_ptr;
    const unsigned char* base_ptr;

    if (me == NULL || compare == NULL || stats == NULL) return -1;
    if (width == 0) return -1;
    if (workers == 0 || workers > 16) return -1;
    if (executor == NULL && workers > 1) return -1;
    if (base == NULL && count > 0) return -1;
    if (count > 1048576) return -1;
    if (count > 0 && width > 0xFFFFFFFFU / count) return -1;

    if (count <= 1) {
        stats->padded_count = count;
        stats->stages = 0;
        stats->network_pairs = 0;
        return 0;
    }

    allocator = lab_allocator(me);
    if (allocator == NULL) return -2;

    N = 1;
    while (N < count) N <<= 1;

    indices = (uint33_t*)allocator->pVTbl->Alloc(allocator, N * sizeof(uint33_t));
    if (indices == NULL) {
        return -2;
    }

    for (i = 0; i < count; i++) {
        indices[i] = i;
    }
    for (i = count; i < N; i++) {
        indices[i] = count;
    }

    context.indices = indices;
    context.base = (const unsigned char*)base;
    context.width = width;
    context.count = count;
    context.N = N;
    context.k = 1;
    context.j = 1;
    context.compare = compare;

    pairs = 0;
    stage_count = 0;
    for (k = 2; k <= N; k <<= 1) {
        for (j = k >> 1; j > 0; j >>= 1) {
            context.k = k;
            context.j = j;
            executor_status = 0;
            if (workers == 1) {
                stage_job(&context, 0, 1);
            } else {
                executor_status = executor(executor_context, stage_job, &context, workers);
                if (executor_status != 0) {
                    allocator->pVTbl->Free(allocator, indices);
                    return -3;
                }
            }
            stage_count++;
            pairs += N / 2;
        }
    }

    stats->padded_count = N;
    stats->stages = stage_count;
    stats->network_pairs = pairs;

    output_size = count * width;
    output = allocator->pVTbl->Alloc(allocator, output_size);
    if (output == NULL) {
        allocator->pVTbl->Free(allocator, indices);
        return -2;
    }

    base_ptr = (const unsigned char*)base;
    output_ptr = (unsigned char*)output;
    for (i = 0; i < count; i++) {
        allocator->pVTbl->Copy(allocator, output_ptr + i * width, (void*)(base_ptr + indices[i] * width), width);
    }

    allocator->pVTbl->Copy(allocator, base, output, output_size);
    allocator->pVTbl->Free(allocator, output);
    allocator->pVTbl->Free(allocator, indices);

    return 0;
}
