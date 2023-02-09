#pragma once
/* Please check https://github.com/nclack/tictoc for platform-timer (Windows/Linux/MacOS) */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "macro.h"


MYS_API const char *hrname();
MYS_API uint64_t hrtick();
MYS_API uint64_t hrfreq();
MYS_API double hrtime();

MYS_API const char *mys_hrname_mpi();
MYS_API uint64_t mys_hrtick_mpi();
MYS_API uint64_t mys_hrfreq_mpi();
MYS_API double mys_hrtime_mpi();

MYS_API const char *mys_hrname_openmp();
MYS_API uint64_t mys_hrtick_openmp();
MYS_API uint64_t mys_hrfreq_openmp();
MYS_API double mys_hrtime_openmp();

#if defined(ARCH_AARCH64)
#define MYS_ENABLED_HRTIMER_AACH64
MYS_API static inline const char *mys_hrname_aarch64();
MYS_API static inline uint64_t mys_hrtick_aarch64();
MYS_API static inline uint64_t mys_hrfreq_aarch64();
MYS_API static inline double mys_hrtime_aarch64();
#endif

#if defined(ARCH_X64) && defined(TSC_FREQ) && TSC_FREQ > 1
#define MYS_ENABLED_HRTIMER_X64
MYS_API static inline const char *mys_hrname_x64();
MYS_API static inline uint64_t mys_hrtick_x64();
MYS_API static inline uint64_t mys_hrfreq_x64();
MYS_API static inline double mys_hrtime_x64();
#endif

#if defined(POSIX_COMPLIANCE)
#define MYS_ENABLED_HRTIMER_POSIX
MYS_API static inline const char *mys_hrname_posix();
MYS_API static inline uint64_t mys_hrtick_posix();
MYS_API static inline uint64_t mys_hrfreq_posix();
MYS_API static inline double mys_hrtime_posix();
#endif

#if defined(OS_WINDOWS)
#define MYS_ENABLED_HRTIMER_WINDOWS
MYS_API static inline const char *mys_hrname_windows();
MYS_API static inline uint64_t mys_hrtick_windows();
MYS_API static inline uint64_t mys_hrfreq_windows();
MYS_API static inline double mys_hrtime_windows();
#endif


////// Internal

#if defined(MYS_ENABLED_HRTIMER_AACH64)
MYS_API static inline const char *mys_hrname_aarch64() {
    return "High-resolution timer by AArch64 assembly (10ns)";
}
MYS_API static inline uint64_t mys_hrtick_aarch64() {
    uint64_t t;
    __asm__ __volatile__("mrs %0, CNTVCT_EL0" : "=r"(t));
    return t;
}
MYS_API static inline uint64_t mys_hrfreq_aarch64() {
    uint64_t f;
    __asm__ __volatile__("mrs %0, CNTFRQ_EL0" : "=r"(f));
    return f;
}
MYS_API static inline double mys_hrtime_aarch64() {
    return (double)hrtick() / (double)hrfreq();
}
#endif

#if defined(MYS_ENABLED_HRTIMER_X64)
// I don't think there is a good way to detect TSC_FREQ without user input.
// /proc/cpuinfo is not reliable under turbo-boost enabled CPU.
// See the get_clockfreq() and warning about GPTLnanotime in GPTLpr_file() of GPTL.
MYS_API static inline const char *mys_hrname_x64() {
    return "High-resolution timer by X64 assembly (TSC_FREQ=" MYS_MACRO2STR(TSC_FREQ) ")";
}
MYS_API static inline uint64_t mys_hrtick_x64() {
    uint32_t lo, hi;
    __asm__ __volatile__("mfence":::"memory");
    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    return (uint64_t)hi << 32 | (uint64_t)lo;
}
MYS_API static inline uint64_t mys_hrfreq_x64() {
    return (uint64_t)TSC_FREQ;
}
MYS_API static inline double mys_hrtime_x64() {
    return (double)hrtick() / (double)hrfreq();
}
#endif

#if defined(MYS_ENABLED_HRTIMER_POSIX)
/*
 * clock_gettime [https://man7.org/linux/man-pages/man2/clock_gettime.2.html]
 * gettimeofday  [https://linux.die.net/man/2/gettimeofday]
 * It often takes 4 ns and doesn't involve any system call [https://stackoverflow.com/a/42190077]
 * Difference between CLOCK_REALTIME and CLOCK_MONOTONIC [https://stackoverflow.com/a/3527632]
 */
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
MYS_API static inline const char *mys_hrname_posix() {
#if defined(CLOCK_MONOTONIC)
    return "High-resolution timer by clock_gettime() of <time.h> (1us~10us)";
#else
    return "High-resolution timer by gettimeofday() of <sys/time.h> (1us~10us)";
#endif
}
MYS_API static inline uint64_t mys_hrtick_posix() {
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
MYS_API static inline uint64_t mys_hrfreq_posix() {
#if defined(CLOCK_MONOTONIC)
    return 1000000000;
#else
    return 1000000;
#endif
}
MYS_API static inline double mys_hrtime_posix() {
    return (double)hrtick() / (double)hrfreq();
}
#endif

#if defined(MYS_ENABLED_HRTIMER_WINDOWS)
/*
 * https://stackoverflow.com/a/5801863
 * https://stackoverflow.com/a/26945754
 */
#include <windows.h>
MYS_API static inline const char *mys_hrname_windows() {
    return "High-resolution timer by <windows.h> (1us~10us)";
}
MYS_API static inline uint64_t mys_hrtick_windows() {
    LARGE_INTEGER t;
    if (!QueryPerformanceCounter(&t))
        return 0;
    return (uint64_t)t.QuadPart;
}
MYS_API static inline uint64_t mys_hrfreq_windows() {
    LARGE_INTEGER f;
    if (!QueryPerformanceFrequency(&f))
        return 0;
    return (uint64_t)f.QuadPart;
}
MYS_API static inline double mys_hrtime_windows() {
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

/* g++ -std=c++11 -I../include -lm ./a.cpp && ./a.out
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int main(int argc, const char **argv)
{
    uint64_t n = 1000000000;
    double t = 0;
    double t1 = hrtime();
    for (uint64_t i = 0; i < n; i++) {
        t += hrtime();
    }
    double t2 = hrtime();
    double tdiff = t2 - t1;
    DEBUG(0, "tdiff %E/%llu=%E seconds", tdiff, n, tdiff/(double)n);
}

Apple M1:
    * no -O: 7.059100E-09 seconds
    * -O0: 7.061317E-09 seconds
    * -O1: 5.355991E-09 seconds
    * -O2: 2.669528E-09 seconds
    * -O3: 2.716072E-09 seconds
*/
