/* Common host-only scheduler. No sorting or comparison logic is supplied here. */
#ifndef ARPO_PARALLEL_POOL_H
#define ARPO_PARALLEL_POOL_H
#include <pthread.h>
#include <string.h>
#include "ArpoMethods.h"
typedef struct ParallelPool ParallelPool;
typedef struct PoolLane { ParallelPool* pool; uint32_t id; } PoolLane;
struct ParallelPool {
    pthread_mutex_t mutex;
    pthread_cond_t start;
    pthread_cond_t done;
    pthread_t threads[16];
    PoolLane lanes[16];
    uint64_t jobs[16];
    uint32_t size;
    uint32_t generation;
    uint32_t completed;
    uint32_t active;
    uint32_t calls;
    uint32_t fail_at;
    int stop;
    BitonicJob job;
    void* argument;
};
static void* pool_worker(void* argument) {
    PoolLane* lane = (PoolLane*)argument;
    ParallelPool* pool = lane->pool;
    uint32_t generation = 0;
    uint32_t active;
    BitonicJob job;
    void* context;
    pthread_mutex_lock(&pool->mutex);
    for (;;) {
        while (!pool->stop && generation == pool->generation)
            pthread_cond_wait(&pool->start, &pool->mutex);
        if (pool->stop) break;
        generation = pool->generation;
        active = pool->active;
        if (lane->id >= active) continue;
        job = pool->job;
        context = pool->argument;
        pthread_mutex_unlock(&pool->mutex);
        job(context, lane->id, active);
        pthread_mutex_lock(&pool->mutex);
        pool->jobs[lane->id]++;
        pool->completed++;
        if (pool->completed == active) pthread_cond_signal(&pool->done);
    }
    pthread_mutex_unlock(&pool->mutex);
    return 0;
}
static int pool_init(ParallelPool* pool, uint32_t size) {
    uint32_t i;
    memset(pool, 0, sizeof(*pool));
    if (size == 0 || size > 16) return -1;
    if (pthread_mutex_init(&pool->mutex, 0) != 0) return -1;
    if (pthread_cond_init(&pool->start, 0) != 0) return -1;
    if (pthread_cond_init(&pool->done, 0) != 0) return -1;
    pool->size = size;
    for (i = 0; i < size; i++) {
        pool->lanes[i].pool = pool;
        pool->lanes[i].id = i;
        if (pthread_create(&pool->threads[i], 0, pool_worker, &pool->lanes[i]) != 0)
            return -1;
    }
    return 0;
}
static int16_t ECOCALLMETHOD pool_execute(void* context, BitonicJob job,
        void* argument, uint32_t workers) {
    ParallelPool* pool = (ParallelPool*)context;
    if (!pool || !job || !workers || workers > pool->size) return -1;
    pthread_mutex_lock(&pool->mutex);
    pool->calls++;
    if (pool->fail_at && pool->calls == pool->fail_at) {
        pthread_mutex_unlock(&pool->mutex);
        return -1;
    }
    pool->job = job;
    pool->argument = argument;
    pool->active = workers;
    pool->completed = 0;
    pool->generation++;
    pthread_cond_broadcast(&pool->start);
    while (pool->completed != workers) pthread_cond_wait(&pool->done, &pool->mutex);
    pthread_mutex_unlock(&pool->mutex);
    return 0;
}
static void pool_destroy(ParallelPool* pool) {
    uint32_t i;
    pthread_mutex_lock(&pool->mutex);
    pool->stop = 1;
    pthread_cond_broadcast(&pool->start);
    pthread_mutex_unlock(&pool->mutex);
    for (i = 0; i < pool->size; i++) pthread_join(pool->threads[i], 0);
    pthread_cond_destroy(&pool->start);
    pthread_cond_destroy(&pool->done);
    pthread_mutex_destroy(&pool->mutex);
}
#endif
