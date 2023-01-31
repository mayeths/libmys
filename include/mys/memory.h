#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "macro.h"
// We can't import any header include "thread.h" here to prevent compiler complaining

/**********************************/
// Memory Barrier
/**********************************/
/* https://support.huaweicloud.com/codeprtr-kunpenggrf/kunpengtaishanporting_12_0048.html */
#if defined(ARCH_X64)
#define mys_memory_barrier() __asm__ __volatile__("": : :"memory")
#define mys_memory_smp_mb() __asm__ __volatile__("lock; addl $0,-132(%%rsp)" ::: "memory", "cc")
#define mys_memory_smp_rmb() mys_memory_barrier()
#define mys_memory_smp_wmb() mys_memory_barrier()
#elif defined(ARCH_AARCH64)
#define mys_memory_barrier() __asm__ __volatile__("dmb" ::: "memory")
#define mys_memory_smp_mb()  __asm__ __volatile__("dmb ish" ::: "memory")
#define mys_memory_smp_rmb() __asm__ __volatile__("dmb ishld" ::: "memory")
#define mys_memory_smp_wmb() __asm__ __volatile__("dmb ishst" ::: "memory")
#else
#error No supprted CPU model
#endif

/* Cache clean */
MYS_API static void cachebrush(size_t nbytes) /* = 10 * 1024 * 1024 */
{
    char * volatile arr = (char *)malloc(nbytes * sizeof(char));
    memset(arr, 0, nbytes);
    for (size_t i = 1; i < nbytes; i++) {
        arr[i] = i | (arr[i - 1]);
    }
    volatile char result;
    result = arr[nbytes - 1];
    result = (char)(uint64_t)(&arr[(uint64_t)result]);
    free(arr);
}
