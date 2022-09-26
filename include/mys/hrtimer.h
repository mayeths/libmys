#pragma once

#include <stdint.h>
#include "macro.h"

#if defined(ARCH_AARCH64)
static inline const char *hrname() {
    return "High-resolution timer by AArch64 assembly (10ns)";
}
static inline double hrfreq() {
    uint64_t f;
    __asm__ __volatile__("mrs %0, CNTFRQ_EL0" : "=r"(f));
    return (double)f;
}
static inline double hrtime() {
    uint64_t t;
    __asm__ __volatile__("mrs %0, CNTVCT_EL0" : "=r"(t));
    return ((double)t) / hrfreq();
}

#elif defined(POSIX_COMPLIANCE)
/*
 * clock_gettime [https://man7.org/linux/man-pages/man2/clock_gettime.2.html]
 * gettimeofday  [https://linux.die.net/man/2/gettimeofday]
 * It often takes 4 ns and doesn't involve any system call [https://stackoverflow.com/a/42190077]
 * Difference between CLOCK_REALTIME and CLOCK_MONOTONIC [https://stackoverflow.com/a/3527632]
 */
#include <sys/time.h>
static inline const char *hrname() {
    return "High-resolution timer by <time.h> (1us~10us)";
}
static inline double hrfreq() {
    return 1;
}
static inline double hrtime() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + ((double)t.tv_usec) / 1e6;
    /* It seems like these functions have the same resolution */
    // struct timespec t;
    // clock_gettime(CLOCK_REALTIME, &t);
    // return (double)t.tv_sec + (double)t.tv_nsec / (double)1e9;
}

#elif defined(OS_WINDOWS)
/*
 * https://stackoverflow.com/a/5801863
 * https://stackoverflow.com/a/26945754
 */
#include <windows.h>
static inline const char *hrname() {
    return "High-resolution timer by <windows.h> (1us~10us)"
}
static inline double hrfreq() {
    LARGE_INTEGER f;
    if (!QueryPerformanceFrequency(&f))
        return -1;
    return (double)f.QuadPart;
}
static inline double hrtime() {
    LARGE_INTEGER t;
    if (!QueryPerformanceCounter(&t))
        return -1;
    return ((double)t.QuadPart) / hrfreq();
}

#endif
