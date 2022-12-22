#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "macro.h"
#include "random.h"

/* Memory Barrier */
/* https://support.huaweicloud.com/codeprtr-kunpenggrf/kunpengtaishanporting_12_0048.html */
#ifndef barrier
#if defined(ARCH_X64)
#define barrier() __asm__ __volatile__("": : :"memory")
#define smp_mb() __asm__ __volatile__("lock; addl $0,-132(%%rsp)" ::: "memory", "cc")
#define smp_rmb() barrier()
#define smp_wmb() barrier()
#elif defined(ARCH_AARCH64)
#define barrier() __asm__ __volatile__("dmb" ::: "memory")
#define smp_mb()  __asm__ __volatile__("dmb ish" ::: "memory")
#define smp_rmb() __asm__ __volatile__("dmb ishld" ::: "memory")
#define smp_wmb() __asm__ __volatile__("dmb ishst" ::: "memory")
#else
#error No supprted CPU model
#endif
#endif


/* Cache clean */
MYS_API static void cachebrush(size_t nbytes) /* = 10 * 1024 * 1024 */
{
    char * volatile arr = (char * volatile)malloc(nbytes * sizeof(char));
    memset(arr, 0, nbytes);
    for (size_t i = 1; i < nbytes; i++) {
        arr[i] = i | (arr[i - 1]);
    }
    volatile char result;
    result = arr[nbytes - 1];
    free(arr);
}
