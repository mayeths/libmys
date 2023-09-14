#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#ifdef OS_LINUX
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include "_config.h"
#include "macro.h"
// We can't import any header include "thread.h" here to prevent compiler complaining

/**********************************/
// Memory Barrier
/**********************************/
/* https://support.huaweicloud.com/codeprtr-kunpenggrf/kunpengtaishanporting_12_0048.html */
#if defined(ARCH_X64)
#define mys_memory_barrier() __asm__ __volatile__("": : :"memory")
// #define mys_memory_smp_mb() __asm__ __volatile__("lock; addl $0,-132(%%rsp)" ::: "memory", "cc")
#define mys_memory_smp_mb() mys_memory_barrier()
#define mys_memory_smp_rmb() mys_memory_barrier()
#define mys_memory_smp_wmb() mys_memory_barrier()
#elif defined(ARCH_AARCH64)
/* https://developer.arm.com/documentation/dui0489/c/arm-and-thumb-instructions/miscellaneous-instructions/dmb--dsb--and-isb */
/* https://developer.arm.com/documentation/den0024/a/Memory-Ordering/Memory-attributes/Cacheable-and-shareable-memory-attributes */
// (SY) Full system DMB operation. Wait for all actions before this instruction to be completed.
#define mys_memory_barrier() __asm__ __volatile__("dmb" ::: "memory")
// Inner shareble domain DMB operation that wait all
// memory access before this instruction to be observable
// by all sockets (for example, write all data to main
// memory and observable by cores on another socket).
#define mys_memory_smp_mb()  __asm__ __volatile__("dmb ish" ::: "memory")
#define mys_memory_smp_rmb() __asm__ __volatile__("dmb ishld" ::: "memory")
// Inner shareable domain DMB operation that waits
// only for stores to be observable by all sockets
// (for example, write all data to main memory and
// observable by cores on another socket).
#define mys_memory_smp_wmb() __asm__ __volatile__("dmb ishst" ::: "memory")
#else
#error No supprted CPU model
#endif

/*
    +----------------------------------------------------------------------------------+
    | Hyprevisor (Outer Shareable Domain)                                              |
    |                                                                                  |
    |      +--------------------------------------------------------------------+      |
    |      | Operating System 1 (Inner Shareable Domain)                        |      |
    |      |    +--------------------------+    +--------------------------+    |      |
    |      |    | Socket 0                 |    | Socket 1                 |    |      |
    |      |    |  +--------+  +--------+  |    |  +--------+  +--------+  |    |      |
    |      |    |  | Core 0 |  | Core 1 |  |    |  | Core 2 |  | Core 3 |  |    |      |
    |      |    |  +--------+  +--------+  |    |  +--------+  +--------+  |    |      |
    |      |    |                          |    |                          |    |      |
    |      |    +--------------------------+    +--------------------------+    |      |
    |      |                                                                    |      |
    |      +--------------------------------------------------------------------+      |
    |                                                                                  |
    |                                                                                  |
    |      +--------------------------------------------------------------------+      |
    |      | Operating System 2 (Inner Shareable Domain)                        |      |
    |      |    +--------------------------+    +--------------------------+    |      |
    |      |    | Socket 2                 |    | Socket 3                 |    |      |
    |      |    |  +--------+  +--------+  |    |  +--------+  +--------+  |    |      |
    |      |    |  | Core 4 |  | Core 5 |  |    |  | Core 6 |  | Core 7 |  |    |      |
    |      |    |  +--------+  +--------+  |    |  +--------+  +--------+  |    |      |
    |      |    |                          |    |                          |    |      |
    |      |    +--------------------------+    +--------------------------+    |      |
    |      |                                                                    |      |
    |      +--------------------------------------------------------------------+      |
    |                                                                                  |
    |                                                                                  |
    |           +--------------------------+    +--------------------------+           |
    |           | GPU Device               |    | FPGA Device              |           |
    |           |                          |    |                          |           |
    |           +--------------------------+    +--------------------------+           |
    |                                                                                  |
    +----------------------------------------------------------------------------------+
*/

#if defined(OS_LINUX) && defined(MYS_ENABLE_SHM)
typedef struct mys_shm_t {
    void *mem;
    size_t _size;
    int _fd;
    char _name[NAME_MAX];
} mys_shm_t;

MYS_API mys_shm_t mys_alloc_shared_memory(int owner_rank, size_t size);
MYS_API void mys_free_shared_memory(mys_shm_t *shm);
#endif


typedef struct _mys_bits_t {
    char bits[64 + 1];
} mys_bits_t;

MYS_STATIC mys_bits_t _mys_bits(const void *data, size_t size)
{
    mys_bits_t res;
    memset(&res, 0, sizeof(mys_bits_t));
    const uint8_t *bytes = (const uint8_t *)data;
    int count = 0;
    for (int i = size - 1; i >= 0; --i) { // begin from high bytes
        for (int j = 7; j >= 0; --j) { // begin from high bits
            unsigned int bit = (bytes[i] >> j) & 1;
            res.bits[count++] = bit ? '1' : '0';
        }
    }
    return res;
}
MYS_STATIC mys_bits_t mys_bits_u64(uint64_t n) { return _mys_bits(&n, sizeof(uint64_t)); } /* printf("%s\n", mys_bits_u64(1).bits); */
MYS_STATIC mys_bits_t mys_bits_u32(uint32_t n) { return _mys_bits(&n, sizeof(uint32_t)); } /* printf("%s\n", mys_bits_u32(1).bits); */
MYS_STATIC mys_bits_t mys_bits_i64(int64_t  n) { return _mys_bits(&n, sizeof(int64_t )); } /* printf("%s\n", mys_bits_i64(1).bits); */
MYS_STATIC mys_bits_t mys_bits_i32(int32_t  n) { return _mys_bits(&n, sizeof(int32_t )); } /* printf("%s\n", mys_bits_i32(1).bits); */
MYS_STATIC mys_bits_t mys_bits_f64(double   n) { return _mys_bits(&n, sizeof(double  )); } /* printf("%s\n", mys_bits_f64(1).bits); */
MYS_STATIC mys_bits_t mys_bits_f32(float    n) { return _mys_bits(&n, sizeof(float   )); } /* printf("%s\n", mys_bits_f32(1).bits); */


/* Cache clean */
MYS_STATIC void cachebrush(size_t nbytes) /* = 10 * 1024 * 1024 */
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

MYS_STATIC ssize_t mys_parse_readable_size(const char *text)
{
    static const double Bbase = 1.0;
    static const double Kbase = 1024.0 * Bbase;
    static const double Mbase = 1024.0 * Kbase;
    static const double Gbase = 1024.0 * Mbase;
    static const double Tbase = 1024.0 * Gbase;
    static const double Pbase = 1024.0 * Tbase;
    static const double Ebase = 1024.0 * Pbase;
    static const double Zbase = 1024.0 * Ebase;
    struct unit_t {
        const char *suffix;
        double base;
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

    while (*endptr == ' ')
        endptr++;
    if (*endptr == '\0')
        return (ssize_t)dnum; /* no suffix */

    for (size_t i = 0; i < sizeof(units) / sizeof(struct unit_t); i++) {
        struct unit_t *unit = &units[i];
        int matched = strncmp(endptr, unit->suffix, 32) == 0;
        if (matched)
            return (ssize_t)(dnum * unit->base);
    }

    return -1;
}

MYS_STATIC void mys_readable_size(char **ptr, size_t bytes, size_t precision)
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
