#include "ArpoMethods.h"

typedef struct StageContext {
    uint32_t* indices;
    const unsigned char* base;
    uint32_t width;
    uint32_t count;
    uint32_t N;
    uint32_t k;
    uint32_t j;
    BitonicCompare compare;
} StageContext;

static void ECOCALLMETHOD stage_job(void* context, uint32_t lane, uint32_t workers) {
    StageContext* ctx = (StageContext*)context;
    uint32_t i;
    uint32_t partner;
    uint32_t left_index;
    uint32_t right_index;
    int order;
    uint32_t temp;

    for (i = lane; i < ctx->N; i += workers) {
        partner = i ^ ctx->j;
        if (i >= partner) continue;

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

int16_t ECOCALLMETHOD lab_bitonic_sort(IEcoLab64* me, void* base, uint32_t count, uint32_t width, BitonicCompare compare, uint32_t workers, BitonicExecutor executor, void* executor_context, BitonicStats* stats) {
    IEcoMemoryAllocator1* allocator;
    uint32_t i;
    uint32_t k;
    uint32_t j;
    uint32_t N;
    uint64_t pairs;
    uint32_t stage_count;
    void* output;
    int16_t executor_status;
    StageContext context;

    if (!me || !compare || !stats) return -1;
    if (width == 0 || (count > 0 && width > 0xFFFFFFFFU / count)) return -1;
    if (workers < 1 || workers > 16) return -1;
    if (executor == NULL && workers > 1) return -1;
    if (count > 1048576) return -1;
    if (count > 0 && !base) return -1;

    if (count <= 1) {
        stats->padded_count = count;
        stats->stages = 0;
        stats->network_pairs = 0;
        return 0;
    }

    N = 1;
    while (N < count) {
        N <<= 1;
    }

    allocator = lab_allocator(me);
    if (!allocator) return -2;

    context.indices = (uint32_t*)allocator->pVTbl->Alloc(allocator, N * sizeof(uint32_t));
    if (!context.indices) {
        allocator->pVTbl->Free(allocator, context.indices);
        return -2;
    }

    for (i = 0; i < count; i++) {
        context.indices[i] = i;
    }
    for (i = count; i < N; i++) {
        context.indices[i] = count;
    }

    context.base = (const unsigned char*)base;
    context.width = width;
    context.count = count;
    context.N = N;
    context.j = j;
    context.k = k;
    context.compare = compare;

    pairs = 0;
    stage_count = 0;

    for (k = 2; k <= N; k <<= 1) {
        for (j = k >> 1; j > 0; j >>= 1) {
            context.k = k;
            context.j = j;
            stage_count++;
            pairs += N / 2;

            if (workers == 1) {
                stage_job(&context, 0, 1);
            } else {
            executor_status = executor(executor_context, stage_job, &context, workers);
                if (executor_status != 0) {
                    allocator->pVTbl->Free(allocator, context.indices);
                    return -3;
                }
            }
        }
    }

    output = allocator->pVTbl->Alloc(allocator, count * width);
    if (!output) {
        allocator->pVTbl->Free(allocator, context.indices);
        return -2;
    }

    for (i = 0; i < count; i++) {
        allocator->pVTbl->Copy(allocator, (unsigned char*)output + i * width, (unsigned char*)base + context.indices[i] * width, width);
    }

    allocator->pVTbl->Copy(allocator, base, output, count * width);
    allocator->pVTbl->Free(allocator, output);
    allocator->pVTbl->Free(allocator, context.indices);

    stats->padded_count = N;
    stats->stages = stage_count;
    stats->network_pairs = pairs;

    return 0;
}
