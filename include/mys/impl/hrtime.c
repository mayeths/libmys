#include "../hrtime.h"

MYS_API const char *mys_hrname()
{
#if defined(MYS_ENABLED_HRTIMER_AACH64)
    return mys_hrname_aarch64();
#elif defined(MYS_ENABLED_HRTIMER_X64)
    return mys_hrname_x64();
#elif defined(MYS_ENABLED_HRTIMER_POSIX)
    return mys_hrname_posix();
#elif defined(MYS_ENABLED_HRTIMER_WINDOWS)
    return mys_hrname_windows();
#elif defined(MYS_USE_OPENMP_TIMER)
    return mys_hrname_openmp();
#else
    return mys_hrname_mpi();
#endif
}

MYS_API uint64_t mys_hrtick()
{
#if defined(MYS_ENABLED_HRTIMER_AACH64)
    return mys_hrtick_aarch64();
#elif defined(MYS_ENABLED_HRTIMER_X64)
    return mys_hrtick_x64();
#elif defined(MYS_ENABLED_HRTIMER_POSIX)
    return mys_hrtick_posix();
#elif defined(MYS_ENABLED_HRTIMER_WINDOWS)
    return mys_hrtick_windows();
#elif defined(MYS_USE_OPENMP_TIMER)
    return mys_hrtick_openmp();
#else
    return mys_hrtick_mpi();
#endif
}

MYS_API uint64_t mys_hrfreq()
{
#if defined(MYS_ENABLED_HRTIMER_AACH64)
    return mys_hrfreq_aarch64();
#elif defined(MYS_ENABLED_HRTIMER_X64)
    return mys_hrfreq_x64();
#elif defined(MYS_ENABLED_HRTIMER_POSIX)
    return mys_hrfreq_posix();
#elif defined(MYS_ENABLED_HRTIMER_WINDOWS)
    return mys_hrfreq_windows();
#elif defined(MYS_USE_OPENMP_TIMER)
    return mys_hrfreq_openmp();
#else
    return mys_hrfreq_mpi();
#endif
}

MYS_API double mys_hrtime()
{
#if defined(MYS_ENABLED_HRTIMER_AACH64)
    return mys_hrtime_aarch64();
#elif defined(MYS_ENABLED_HRTIMER_X64)
    return mys_hrtime_x64();
#elif defined(MYS_ENABLED_HRTIMER_POSIX)
    return mys_hrtime_posix();
#elif defined(MYS_ENABLED_HRTIMER_WINDOWS)
    return mys_hrtime_windows();
#elif defined(MYS_USE_OPENMP_TIMER)
    return mys_hrtime_openmp();
#else
    return mys_hrtime_mpi();
#endif
}

#if defined(MYS_ENABLED_HRTIMER_AACH64)
typedef struct _mys_hrtime_aarch64_G_t {
    bool inited;
    uint64_t start;
} _mys_hrtime_aarch64_G_t;
mys_thread_local _mys_hrtime_aarch64_G_t _mys_hrtime_aarch64_G = {
    .inited = false,
    .start = 0,
};
MYS_API const char *mys_hrname_aarch64() {
    return "High-resolution timer by AArch64 assembly (10ns)";
}
MYS_API uint64_t mys_hrtick_aarch64() {
    if (_mys_hrtime_aarch64_G.inited == false) {
        __asm__ __volatile__("mrs %0, CNTVCT_EL0" : "=r"(_mys_hrtime_aarch64_G.start));
        _mys_hrtime_aarch64_G.inited = true;
    }
    uint64_t t;
    __asm__ __volatile__("mrs %0, CNTVCT_EL0" : "=r"(t));
    return t - _mys_hrtime_aarch64_G.start;
}
MYS_API uint64_t mys_hrfreq_aarch64() {
    uint64_t f;
    __asm__ __volatile__("mrs %0, CNTFRQ_EL0" : "=r"(f));
    return f;
}
MYS_API double mys_hrtime_aarch64() {
    return (double)mys_hrtick_aarch64() / (double)mys_hrfreq_aarch64();
}
#endif

#if defined(MYS_ENABLED_HRTIMER_X64)
// I don't think there is a good way to detect TSC_FREQ without user input.
// /proc/cpuinfo is not reliable under turbo-boost enabled CPU.
// See the get_clockfreq() and warning about GPTLnanotime in GPTLpr_file() of GPTL.
typedef struct _mys_hrtime_x64_G_t {
    bool inited;
    uint64_t start;
} _mys_hrtime_x64_G_t;
mys_thread_local _mys_hrtime_x64_G_t _mys_hrtime_x64_G = {
    .inited = false,
    .start = 0,
};
MYS_API const char *mys_hrname_x64() {
    return "High-resolution timer by X64 assembly (TSC_FREQ=" MYS_MACRO2STR(TSC_FREQ) ")";
}
MYS_API uint64_t mys_hrtick_x64() {
    if (_mys_hrtime_x64_G.inited == false) {
        uint32_t lo, hi;
        __asm__ __volatile__("mfence":::"memory");
        __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
        _mys_hrtime_x64_G.start = (uint64_t)hi << 32 | (uint64_t)lo;
        _mys_hrtime_x64_G.inited = true;
    }
    uint32_t lo, hi;
    __asm__ __volatile__("mfence":::"memory");
    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    uint64_t t = (uint64_t)hi << 32 | (uint64_t)lo;
    return t - _mys_hrtime_x64_G.start;
}
MYS_API uint64_t mys_hrfreq_x64() {
    return (uint64_t)TSC_FREQ;
}
MYS_API double mys_hrtime_x64() {
    return (double)mys_hrtick_x64() / (double)mys_hrfreq_x64();
}
#endif

#if defined(MYS_ENABLED_HRTIMER_POSIX)
/*
 * clock_gettime [https://man7.org/linux/man-pages/man2/clock_gettime.2.html]
 * gettimeofday  [https://linux.die.net/man/2/gettimeofday]
 * It often takes 4 ns and doesn't involve any system call [https://stackoverflow.com/a/42190077]
 * Difference between CLOCK_REALTIME and CLOCK_MONOTONIC [https://stackoverflow.com/a/3527632]
 */
typedef struct _mys_hrtime_posix_G_t {
    bool inited;
    uint64_t start;
} _mys_hrtime_posix_G_t;
mys_thread_local _mys_hrtime_posix_G_t _mys_hrtime_posix_G = {
    .inited = false,
    .start = 0,
};
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
MYS_API const char *mys_hrname_posix() {
#if defined(CLOCK_MONOTONIC)
    return "High-resolution timer by clock_gettime() of <time.h> (1us~10us)";
#else
    return "High-resolution timer by gettimeofday() of <sys/time.h> (1us~10us)";
#endif
}
MYS_API uint64_t mys_hrtick_posix() {
    uint64_t t = 0;
#if defined(CLOCK_MONOTONIC)
    if (_mys_hrtime_posix_G.inited == false) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        _mys_hrtime_posix_G.start = (uint64_t)ts.tv_sec * (uint64_t)1000000000 + (uint64_t)ts.tv_nsec;
        _mys_hrtime_posix_G.inited = true;
    }
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    t = (uint64_t)ts.tv_sec * (uint64_t)1000000000 + (uint64_t)ts.tv_nsec;
#else
    if (_mys_hrtime_posix_G.inited == false) {
        struct timeval ts;
        gettimeofday(&ts, NULL);
        _mys_hrtime_posix_G.start = (uint64_t)ts.tv_sec * (uint64_t)1000000 + (uint64_t)ts.tv_usec;
        _mys_hrtime_posix_G.inited = true;
    }
    struct timeval ts;
    gettimeofday(&ts, NULL);
    t = (uint64_t)ts.tv_sec * (uint64_t)1000000 + (uint64_t)ts.tv_usec;
#endif
    return t - _mys_hrtime_posix_G.start;
}
MYS_API uint64_t mys_hrfreq_posix() {
#if defined(CLOCK_MONOTONIC)
    return 1000000000;
#else
    return 1000000;
#endif
}
MYS_API double mys_hrtime_posix() {
    return (double)mys_hrtick_posix() / (double)mys_hrfreq_posix();
}
#endif

#if defined(MYS_ENABLED_HRTIMER_WINDOWS)
/*
 * https://stackoverflow.com/a/5801863
 * https://stackoverflow.com/a/26945754
 */
#include <windows.h>
MYS_API const char *mys_hrname_windows() {
    return "High-resolution timer by <windows.h> (1us~10us)";
}
MYS_API uint64_t mys_hrtick_windows() {
    LARGE_INTEGER t;
    if (!QueryPerformanceCounter(&t))
        return 0;
    return (uint64_t)t.QuadPart;
}
MYS_API uint64_t mys_hrfreq_windows() {
    LARGE_INTEGER f;
    if (!QueryPerformanceFrequency(&f))
        return 0;
    return (uint64_t)f.QuadPart;
}
MYS_API double mys_hrtime_windows() {
    return (double)mys_hrtick_windows() / (double)mys_hrfreq_windows();
}
#endif

typedef struct _mys_hrtime_mpi_G_t {
    bool inited;
    double start;
} _mys_hrtime_mpi_G_t;
mys_thread_local _mys_hrtime_mpi_G_t _mys_hrtime_mpi_G = {
    .inited = false,
    .start = 0,
};
MYS_API const char *mys_hrname_mpi() {
    return "High-resolution timer by <mpi.h> (1us~10us)";
}
MYS_API uint64_t mys_hrtick_mpi() {
    if (_mys_hrtime_mpi_G.inited == false) {
        _mys_hrtime_mpi_G.start = _mys_MPI_Wtime();
        _mys_hrtime_mpi_G.inited = true;
    }
    double current = _mys_MPI_Wtime() - _mys_hrtime_mpi_G.start;
    return (uint64_t)(current * 1e9); // in nano second
}
MYS_API uint64_t mys_hrfreq_mpi() {
    return (uint64_t)1000000000;
}
MYS_API double mys_hrtime_mpi() {
    return (double)mys_hrtick_mpi() / (double)mys_hrfreq_mpi();
}

#if defined(_OPENMP)
#include <omp.h>
typedef struct _mys_hrtime_openmp_G_t {
    bool inited;
    double start;
} _mys_hrtime_openmp_G_t;
mys_thread_local _mys_hrtime_openmp_G_t _mys_hrtime_openmp_G = {
    .inited = false,
    .start = 0,
};
MYS_API const char *mys_hrname_openmp() {
    return "High-resolution timer by <omp.h> (1us~10us)";
}
MYS_API uint64_t mys_hrtick_openmp() {
    if (_mys_hrtime_openmp_G.inited == false) {
        _mys_hrtime_openmp_G.start = omp_get_wtime();
        _mys_hrtime_openmp_G.inited = true;
    }
    double current = omp_get_wtime() - _mys_hrtime_openmp_G.start;
    return (uint64_t)(current * 1e9); // in nano second
}
MYS_API uint64_t mys_hrfreq_openmp() {
    return (uint64_t)1000000000;
}
MYS_API double mys_hrtime_openmp() {
    return (double)mys_hrtick_openmp() / (double)mys_hrfreq_openmp();
}
#endif

#if !defined(MYS_NO_LEGACY) && !defined(MYS_NO_LEGACY_HRTIME)
MYS_API const char *hrname()
{
    return mys_hrname();
}

MYS_API uint64_t hrtick()
{
    return mys_hrtick();
}

MYS_API uint64_t hrfreq()
{
    return mys_hrfreq();
}

MYS_API double hrtime()
{
    return mys_hrtime();
}
#endif
