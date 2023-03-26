#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

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

MYS_API static ssize_t mys_parse_readable_size(const char *text)
{
    static const size_t Bbase = 1ULL;
    static const size_t Kbase = 1024ULL * Bbase;
    static const size_t Mbase = 1024ULL * Kbase;
    static const size_t Gbase = 1024ULL * Mbase;
    static const size_t Tbase = 1024ULL * Gbase;
    static const size_t Pbase = 1024ULL * Tbase;
    static const size_t Ebase = 1024ULL * Pbase;
    static const size_t Zbase = 1024ULL * Ebase;
    struct unit_t {
        const char *suffix;
        size_t base;
    };
    struct unit_t units[] = {
        { .suffix = "Bytes",  .base = Bbase },
        { .suffix = "Byte",   .base = Bbase },
        { .suffix = "B",      .base = Bbase },
        { .suffix = "KBytes", .base = Kbase },
        { .suffix = "KB",     .base = Kbase },
        { .suffix = "K",      .base = Kbase },
        { .suffix = "MBytes", .base = Mbase },
        { .suffix = "MB",     .base = Mbase },
        { .suffix = "M",      .base = Mbase },
        { .suffix = "GBytes", .base = Gbase },
        { .suffix = "GB",     .base = Gbase },
        { .suffix = "G",      .base = Gbase },
        { .suffix = "TBytes", .base = Tbase },
        { .suffix = "TB",     .base = Tbase },
        { .suffix = "T",      .base = Tbase },
        { .suffix = "PBytes", .base = Pbase },
        { .suffix = "PB",     .base = Pbase },
        { .suffix = "P",      .base = Pbase },
        { .suffix = "EBytes", .base = Ebase },
        { .suffix = "EB",     .base = Ebase },
        { .suffix = "E",      .base = Ebase },
        { .suffix = "ZBytes", .base = Zbase },
        { .suffix = "ZB",     .base = Zbase },
        { .suffix = "Z",      .base = Zbase },
    };

    char *endptr = NULL;
    errno = 0;
    double dnum = strtod(text, &endptr);
    int error = errno;
    errno = 0;

    if (endptr == text)
        return -1; /* contains with non-number */
    if (error == ERANGE)
        return -1; /* number out of range for double */
    if (dnum != dnum)
        return -1; /* not a number */

    ssize_t num = (ssize_t)dnum;

    while (*endptr == ' ')
        endptr++;
    if (*endptr == '\0')
        return (ssize_t)dnum; /* no suffix */

    for (size_t i = 0; i < sizeof(units) / sizeof(struct unit_t); i++) {
        struct unit_t *unit = &units[i];
        int matched = strncmp(endptr, unit->suffix, 32) == 0;
        if (matched)
            return num * unit->base;
    }

    return -1;
}

static inline void mys_readable_size(char **ptr, size_t bytes, size_t precision)
{
    int i = 0;
    const char* units[] = {"Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    double size = bytes;
    while (size > 1024) {
        size /= 1024;
        i++;
    }
    int len = snprintf(NULL, 0, "%.*f %s", (int)precision, size, units[i]) + 1; /*%.*f*/
    *ptr = (char *)malloc(sizeof(char) * len);
    snprintf(*ptr, len, "%.*f %s", (int)precision, size, units[i]);
}
