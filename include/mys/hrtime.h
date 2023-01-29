#pragma once
/* Please check https://github.com/nclack/tictoc for platform-timer (Windows/Linux/MacOS) */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "config.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#if defined(ARCH_AARCH64)
static inline const char *hrname() {
    return "High-resolution timer by AArch64 assembly (10ns)";
}
static inline uint64_t hrtick() {
    uint64_t t;
    __asm__ __volatile__("mrs %0, CNTVCT_EL0" : "=r"(t));
    return t;
}
static inline uint64_t hrfreq() {
    uint64_t f;
    __asm__ __volatile__("mrs %0, CNTFRQ_EL0" : "=r"(f));
    return f;
}
static inline double hrtime() {
    return (double)hrtick() / (double)hrfreq();
}

#elif defined(ARCH_X64) && defined(TSC_FREQ) && TSC_FREQ > 1

// Copied from GPTL library
static double get_clockfreq()
{
    double freq = -1.;
    FILE *fd = NULL;
    char buf[4096];

    // First look for max_freq, but that isn't guaranteed to exist
    if ((fd = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "r"))) {
        if(fgets(buf, sizeof(buf), fd)) {
            freq = 1000 * (double)atof(buf); // from KHz
        }
        fclose(fd);
        return freq;
    }

    // Next try /proc/cpuinfo. That has the disadvantage that it may give wrong info
    // for processors that have either idle or turbo mode
    if ((fd = fopen ("/proc/cpuinfo", "r"))) {
        while (fgets(buf, sizeof(buf), fd)) {
            if (strncmp(buf, "cpu MHz", 7) == 0) {
                int index = 7;
                while (buf[index] != '\0' && !isdigit(buf[index]))
                    index += 1;
                if (isdigit(buf[index])) {
                    freq = (double)atof(&buf[index]);
                    break;
                }
            }
        }
        fclose(fd);
    }

    return freq;
}

static inline const char *hrname() {
    return "High-resolution timer by X64 assembly (TSC_FREQ=" STR(TSC_FREQ) ")";
}
static inline uint64_t hrtick() {
    uint32_t lo, hi;
    __asm__ __volatile__("mfence":::"memory");
    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    return (uint64_t)hi << 32 | (uint64_t)lo;
}
static inline uint64_t hrfreq() {
    return (uint64_t)TSC_FREQ;
}
static inline double hrtime() {
    return (double)hrtick() / (double)hrfreq();
}

#elif defined(POSIX_COMPLIANCE)
/*
 * clock_gettime [https://man7.org/linux/man-pages/man2/clock_gettime.2.html]
 * gettimeofday  [https://linux.die.net/man/2/gettimeofday]
 * It often takes 4 ns and doesn't involve any system call [https://stackoverflow.com/a/42190077]
 * Difference between CLOCK_REALTIME and CLOCK_MONOTONIC [https://stackoverflow.com/a/3527632]
 */
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
static inline const char *hrname() {
#if defined(CLOCK_MONOTONIC)
    return "High-resolution timer by clock_gettime() of <time.h> (1us~10us)";
#else
    return "High-resolution timer by gettimeofday() of <sys/time.h> (1us~10us)";
#endif
}
static inline uint64_t hrtick() {
#if defined(CLOCK_MONOTONIC)
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint64_t)t.tv_sec * (uint64_t)1000000000 + (uint64_t)t.tv_nsec;
#else
    struct timeval t;
    gettimeofday(&t, NULL);
    return (uint64_t)t.tv_sec * (uint64_t)1000000 + (uint64_t)t.tv_usec;
#endif
}
static inline double hrfreq() {
#if defined(CLOCK_MONOTONIC)
    return 1000000000;
#else
    return 1000000;
#endif
}
static inline double hrtime() {
    return (double)hrtick() / (double)hrfreq();
}

#elif defined(OS_WINDOWS)
/*
 * https://stackoverflow.com/a/5801863
 * https://stackoverflow.com/a/26945754
 */
#include <windows.h>
static inline const char *hrname() {
    return "High-resolution timer by <windows.h> (1us~10us)";
}
static inline uint64_t hrtick() {
    LARGE_INTEGER t;
    if (!QueryPerformanceCounter(&t))
        return 0;
    return (uint64_t)t.QuadPart;
}
static inline uint64_t hrfreq() {
    LARGE_INTEGER f;
    if (!QueryPerformanceFrequency(&f))
        return 0;
    return (uint64_t)f.QuadPart;
}
static inline double hrtime() {
    return (double)hrtick() / (double)hrfreq();
}

#endif

/******* AUX *******/

#if defined(POSIX_COMPLIANCE)
#include <stdlib.h>
#include <unistd.h>
static inline uint64_t test_freq()
{
    uint64_t raw1 = hrtick();
    sleep(1);
    uint64_t raw2 = hrtick();
    return raw2 - raw1;
}
#endif
