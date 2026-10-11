/* Fixed ABI. Implement only lab_bitonic_sort in SourceFiles/BitonicMethods.c.
 * Contract: me, compare and stats required; width>0; workers in [1,16];
 * executor required for workers>1. base may be NULL only for count==0.
 * count<=1048576; count*width must fit uint32_t. Invalid arguments: -1.
 * Allocation failure: -2; executor failure: -3; success: 0.
 * Do not modify input on allocation/executor failure. Support arbitrary lengths.
 * count<=1: padded_count=count, stages=0, network_pairs=0, no allocation/jobs.
 * Otherwise padded_count is the least power of two >=count; stages is
 * log2(padded_count)*(log2(padded_count)+1)/2; network_pairs is
 * (padded_count/2)*stages, counting all network pairs including padding.
 * workers==1: never call executor. workers>1: one synchronous executor call
 * per network stage. Output ordered according to compare, preserving all bytes.
 * Stability is not required. Caller owns array and must not change it during Sort.
 * lab_allocator returns a borrowed Eco allocator; do not Release it.
 * No libc, pthread, qsort, math, or third-party calls in this source file. */
#ifndef ARPO_BITONIC_METHODS_H
#define ARPO_BITONIC_METHODS_H
#include "IEcoLab64.h"
#include "IEcoMemoryAllocator1.h"
IEcoMemoryAllocator1* ECOCALLMETHOD lab_allocator(IEcoLab64* me);
int16_t ECOCALLMETHOD lab_bitonic_sort(IEcoLab64* me, void* base,
    uint32_t count, uint32_t width, BitonicCompare compare, uint32_t workers,
    BitonicExecutor executor, void* executor_context, BitonicStats* stats);
#endif
