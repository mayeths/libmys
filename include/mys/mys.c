/**
 * @file mys.c
 * @author mayeths (wow@mayeths.com)
 * @brief Implement non-static variables and fucntions
 * 
 * The biggest benifit of using standalone "mys.c"
 * instead of #ifdef MYS_IMPL in all other headers are,
 * we can use `#pragma once` in these other headers,
 * and the MYS_IMPL macro is still processed correctly
 */
#ifndef __MYS_C__
#define __MYS_C__

/*********************************************/
// C definition
/*********************************************/
#include "thread.h"
#include "errno.h"
#include "myspi.h"
#include "log.h"
#include "rand.h"

mys_thread_local int mys_errno = 0;


_mys_myspi_G_t _mys_myspi_G = {
    .inited = false,
    .lock = MYS_MUTEX_INITIALIZER,
    .myrank = -1,
    .nranks = -1,
};

MYS_API void mys_myspi_init()
{
    if (_mys_myspi_G.inited == true)
        return;
    mys_mutex_lock(&_mys_myspi_G.lock);
#if defined(MYS_NO_MPI)
    _mys_myspi_G.myrank = 0;
    _mys_myspi_G.nranks = 1;
#else
    int inited;
    MPI_Initialized(&inited);
    if (!inited) {
        MPI_Init_thread(NULL, NULL, MPI_THREAD_SINGLE, &inited);
        fprintf(stdout, ">>>>> ===================================== <<<<<\n");
        fprintf(stdout, ">>>>> Nevel let libmys init MPI you dumbass <<<<<\n");
        fprintf(stdout, ">>>>> ===================================== <<<<<\n");
        fflush(stdout);
    }
    MPI_Comm_size(MPI_COMM_WORLD, &_mys_myspi_G.nranks);
    MPI_Comm_rank(MPI_COMM_WORLD, &_mys_myspi_G.myrank);
#endif
    _mys_myspi_G.inited = true;
    mys_mutex_unlock(&_mys_myspi_G.lock);
}

MYS_API int mys_myrank()
{
    mys_myspi_init();
    return _mys_myspi_G.myrank;
}

MYS_API int mys_nranks()
{
    mys_myspi_init();
    return _mys_myspi_G.nranks;
}

MYS_API void mys_barrier()
{
    mys_myspi_init();
#if defined(MYS_NO_MPI)
    return;
#else
    MPI_Barrier(MPI_COMM_WORLD);
#endif
}


mys_thread_local _mys_rand_G_t _mys_rand_G = {
    .inited = false,
    .splitmix64_x = 0,
    .xoroshiro128_x = {0, 0},
};

MYS_API void mys_rand_init()
{
    if (_mys_rand_G.inited == true)
        return;
    uint64_t seed[2];
    seed[0] = mys_rand_seed();
    seed[1] = MYS_UINT64_MAX - seed[0];
    mys_rand_srand(seed[0], seed[1]);
    _mys_rand_G.inited = true;
}

MYS_API void mys_rand_srand(uint64_t a0, uint64_t a1)
{
    srand((unsigned int)a0); /* mys_rand_legacy */
    _mys_rand_G.splitmix64_x = a0;
    _mys_rand_G.xoroshiro128_x[0] = a0;
    _mys_rand_G.xoroshiro128_x[1] = a1;
}

MYS_API uint64_t mys_rand_seed()
{
#if defined(MYS_FAKE_RANDOM)
    return mys_rand_seed_fixed();
#elif defined(ARCH_X64)
    return mys_rand_seed_x64();
#elif defined(ARCH_AARCH64)
    return mys_rand_seed_aarch64();
#else
    return mys_rand_seed_time();
#endif
}

MYS_API uint64_t mys_rand_seed_fixed()
{
    return (uint64_t)0x5A5A5555AAAA55AA;
}

MYS_API uint64_t mys_rand_seed_time()
{
    uint64_t t = (uint64_t)time(NULL);
    return (t << 32) | (t & 0xAAAA5555);
}

#ifdef ARCH_X64
MYS_API uint64_t mys_rand_seed_x64()
{
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    uint64_t t = ((uint64_t)hi << 32) | (uint64_t)lo;
    return (t << 32) | (t & 0xAAAA5555); /* A(1010)5(0101) won't INVALID(1111_1111) again */
}
#endif

#ifdef ARCH_AARCH64
MYS_API uint64_t mys_rand_seed_aarch64()
{
    uint64_t t;
    __asm__ __volatile__("mrs %0, CNTVCT_EL0" : "=r"(t));
    return (t << 32) | (t & 0xAAAA5555);
}
#endif

MYS_API uint64_t mys_rand_legacy_u64()
{
    mys_rand_init();
    double perc = (double)rand() / (double)RAND_MAX;
    return perc * (double)MYS_UINT64_MAX;
}

MYS_API uint64_t mys_rand_splitmix_u64()
{
    mys_rand_init();
    uint64_t z = (_mys_rand_G.splitmix64_x += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
}

static inline uint64_t __rotl(const uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}
MYS_API uint64_t mys_rand_xoroshiro128ss_u64()
{
    mys_rand_init();
    const uint64_t s0 = _mys_rand_G.xoroshiro128_x[0];
    uint64_t s1 = _mys_rand_G.xoroshiro128_x[1];
    const uint64_t result = __rotl(s0 * 5, 7) * 9;
    s1 ^= s0;
    _mys_rand_G.xoroshiro128_x[0] = __rotl(s0, 24) ^ s1 ^ (s1 << 16);
    _mys_rand_G.xoroshiro128_x[1] = __rotl(s1, 37);
    return result;
}

MYS_API uint64_t mys_randu64(uint64_t minimum, uint64_t maximum)
{
#if defined(MYS_LEGACY_RANDOM)
    uint64_t rand = mys_rand_legacy_u64();
#elif defined(MYS_SPLITMIX_RANDOM)
    uint64_t rand = mys_rand_splitmix_u64();
#else
    uint64_t rand = mys_rand_xoroshiro128ss_u64();
#endif
    if (maximum <= minimum)
        return minimum;
    if (minimum == 0 && maximum == MYS_UINT64_MAX)
        return rand;
    double range = maximum - minimum + 1;
    double perc = (double)rand / (double)MYS_UINT64_MAX;
    return perc * range;
}

MYS_API int64_t mys_randi64(int64_t minimum, int64_t maximum)
{
    uint64_t _min = (uint64_t)minimum;
    uint64_t _max = (uint64_t)maximum;
    uint64_t result = mys_randu64(_min, _max);
    return *(int64_t *)((void *)&result);
}

MYS_API double mys_randf64(double minimum, double maximum)
{
    double range = maximum - minimum;
    double div = (double)MYS_UINT64_MAX / range;
    return minimum + (mys_randu64(0, MYS_UINT64_MAX) / div);
}

MYS_API uint32_t mys_randu32(uint32_t minimum, uint32_t maximum)
{
    return (uint32_t)mys_randu64(minimum, maximum);
}

MYS_API int32_t mys_randi32(int32_t minimum, int32_t maximum)
{
    return (int32_t)mys_randi64(minimum, maximum);
}

MYS_API float mys_randf32(float minimum, float maximum)
{
    return (float)mys_randf64(minimum, maximum);
}

MYS_API const char *mys_randname()
{
#if defined(MYS_LEGACY_RANDOM)
    return "legacy random generator";
#elif defined(MYS_SPLITMIX_RANDOM)
    return "splitmix random generator";
#else
    return "xoroshiro128ss random generator";
#endif
}


static void _mys_log_stdio_handler(mys_log_event_t *event);

_mys_log_G_t _mys_log_G = {
    .inited = false,
    .lock = MYS_MUTEX_INITIALIZER,
    .level = MYS_LOG_TRACE,
    .last_level = MYS_LOG_TRACE,
    .handlers = {
#ifndef MYS_LOG_DISABLE_STDOUT_HANDLER
        { .fn = _mys_log_stdio_handler, .udata = NULL, .id = 10000 },
#endif
        { .fn = NULL, .udata = NULL, .id = 0 /* Uninitalized ID is 0 */ },
    },
};

MYS_API void mys_log_init()
{
    if (_mys_log_G.inited == true)
        return;
    mys_mutex_lock(&_mys_log_G.lock);
    _mys_log_G.inited = true;
    mys_mutex_unlock(&_mys_log_G.lock);
}

MYS_API void mys_log(int who, int level, const char *file, int line, const char *fmt, ...)
{
    mys_log_init();
    mys_mutex_lock(&_mys_log_G.lock);
    int myrank = mys_myrank();
    if (who == myrank && (int)level >= (int)_mys_log_G.level) {
        for (int i = 0; i < 128; i++) {
            if (_mys_log_G.handlers[i].fn == NULL)
                break;
            mys_log_event_t event;
            event.level = level;
            event.file = file;
            event.line = line;
            event.fmt = fmt;
            event.udata = _mys_log_G.handlers[i].udata;
            va_start(event.vargs, fmt);
            _mys_log_G.handlers[i].fn(&event);
            va_end(event.vargs);
        }
    }
    mys_mutex_unlock(&_mys_log_G.lock);
}

MYS_API int mys_log_add_handler(mys_log_handler_fn handler_fn, void *handler_udata)
{
    mys_log_init();
    mys_mutex_lock(&_mys_log_G.lock);
    int used_max_id = INT32_MIN;
    for (int i = 0; i < 128; i++) {
        if (_mys_log_G.handlers[i].fn == NULL)
            break;
        if (used_max_id < _mys_log_G.handlers[i].id)
            used_max_id = _mys_log_G.handlers[i].id;
    }
    int id = used_max_id + 1;
    for (int i = 0; i < 128; i++) {
        if (_mys_log_G.handlers[i].fn != NULL)
            continue;
        _mys_log_G.handlers[i].fn = handler_fn;
        _mys_log_G.handlers[i].udata = handler_udata;
        _mys_log_G.handlers[i].id = id;
        break;
    }
    mys_mutex_unlock(&_mys_log_G.lock);
    return id;
}

MYS_API void mys_log_remove_handler(int handler_id)
{
    mys_log_init();
    mys_mutex_lock(&_mys_log_G.lock);
    for (int i = 0; i < 128; i++) {
        if (_mys_log_G.handlers[i].id != handler_id)
            continue;
        _mys_log_G.handlers[i].fn = NULL;
        _mys_log_G.handlers[i].udata = NULL;
        _mys_log_G.handlers[i].id = 0;
        for (int j = i + 1; j < 128; j++) {
            if (_mys_log_G.handlers[j].fn == NULL)
                break;
            _mys_log_G.handlers[j - 1].fn = _mys_log_G.handlers[j].fn;
            _mys_log_G.handlers[j - 1].udata = _mys_log_G.handlers[j].udata;
            _mys_log_G.handlers[j - 1].id = _mys_log_G.handlers[j].id;
        }
        break;
    }
    mys_mutex_unlock(&_mys_log_G.lock);
}

MYS_API int mys_log_get_level()
{
    mys_log_init();
    mys_mutex_lock(&_mys_log_G.lock);
    int level = _mys_log_G.level;
    mys_mutex_unlock(&_mys_log_G.lock);
    return level;
}

MYS_API void mys_log_set_level(int level)
{
    mys_log_init();
    mys_mutex_lock(&_mys_log_G.lock);
    _mys_log_G.level = level;
    mys_mutex_unlock(&_mys_log_G.lock);
}

MYS_API const char* mys_log_level_string(int level)
{
    const char *level_strings[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
    };
    return level_strings[(int)level];
}

static void _mys_log_stdio_handler(mys_log_event_t *event) {
    FILE *file = event->udata != NULL ? (FILE *)event->udata : stdout;

    char base_label[256] = {'\0'};

    char *label = base_label;
    int label_size = sizeof(base_label);

    int myrank = mys_myrank();
    int nranks = mys_nranks();
    int rank_digits = trunc(log10(nranks)) + 1;
    int line_digits = trunc(log10(event->line)) + 1;
    rank_digits = rank_digits > 3 ? rank_digits : 3;
    line_digits = line_digits > 3 ? line_digits : 3;

    if ((int)event->level < (int)MYS_LOG_RAW) {
        const char *lstr = mys_log_level_string(event->level);
        const char level_shortname = lstr[0];
        snprintf(label, label_size, "[%c::%0*d %s:%0*d]",
            level_shortname, rank_digits, myrank,
            event->file, line_digits, event->line
        );
#ifdef MYS_LOG_COLOR
        const char *level_colors[] = {
            "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
        };
        char colorized_label[sizeof(base_label) + 128];
        if (isatty(fileno(file))) {
            snprintf(colorized_label, sizeof(colorized_label), "%s%s\x1b[0m",
                level_colors[(int)event->level], label
            );
            label = colorized_label;
            label_size = sizeof(colorized_label);
        }
#endif
        fprintf(file, "%s ", label);
    }

    vfprintf(file, event->fmt, event->vargs);
    fprintf(file, "\n");
    fflush(file);
}


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
_mys_hrtime_aarch64_G_t _mys_hrtime_aarch64_G = {
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
_mys_hrtime_x64_G_t _mys_hrtime_x64_G = {
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
_mys_hrtime_posix_G_t _mys_hrtime_posix_G = {
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
        _mys_hrtime_posix_G.start = (uint64_t)t.tv_sec * (uint64_t)1000000 + (uint64_t)t.tv_usec;
        _mys_hrtime_posix_G.inited = true;
    }
    struct timeval ts;
    gettimeofday(&ts, NULL);
    t = (uint64_t)t.tv_sec * (uint64_t)1000000 + (uint64_t)t.tv_usec;
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

#if !defined(MYS_NO_MPI)
#include <mpi.h>
typedef struct _mys_hrtime_mpi_G_t {
    bool inited;
    double start;
} _mys_hrtime_mpi_G_t;
_mys_hrtime_mpi_G_t _mys_hrtime_mpi_G = {
    .inited = false,
    .start = 0,
};
MYS_API const char *mys_hrname_mpi() {
    return "High-resolution timer by <mpi.h> (1us~10us)";
}
MYS_API uint64_t mys_hrtick_mpi() {
    if (_mys_hrtime_mpi_G.inited == false) {
        _mys_hrtime_mpi_G.start = MPI_Wtime();
        _mys_hrtime_mpi_G.inited = true;
    }
    double current = MPI_Wtime() - _mys_hrtime_mpi_G.start;
    return (uint64_t)(current * 1e9); // in nano second
}
MYS_API uint64_t mys_hrfreq_mpi() {
    return (uint64_t)1000000000;
}
MYS_API double mys_hrtime_mpi() {
    return (double)mys_hrtick_mpi() / (double)mys_hrfreq_mpi();
}
#endif

#if defined(_OPENMP)
#include <omp.h>
typedef struct _mys_hrtime_openmp_G_t {
    bool inited;
    double start;
} _mys_hrtime_openmp_G_t;
_mys_hrtime_openmp_G_t _mys_hrtime_openmp_G = {
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


static void _mys_close_fd(int fd)
{
    if (fcntl(fd, F_GETFL) != -1 || errno != EBADF)
        close(fd);
}

/**
 * @brief Create stdin/stdout/stderr pipe to subprocess opened with command
 * @note
 * Thanks to http://www.jukie.net/bart/blog/popenRWE
 * https://github.com/sni/mod_gearman/blob/master/common/popenRWE.c
 * https://github.com/marssaxman/ozette/blob/833b659757/src/console/popenRWE.cpp
 */
static int _mys_popen_rwe(int *ipipe, int *opipe, int *epipe, const char *command)
{
    int in[2] = {-1, -1};
    int out[2] = {-1, -1};
    int err[2] = {-1, -1};
    int rc = 0;
    int pid = 0;

    if ((rc = pipe(in)) < 0)
        goto error_in;
    if ((rc = pipe(out)) < 0)
        goto error_out;
    if ((rc = pipe(err)) < 0)
        goto error_err;

    pid = fork();
    if (pid > 0) { /* parent */
        _mys_close_fd(in[0]);
        _mys_close_fd(out[1]);
        _mys_close_fd(err[1]);
        *ipipe = in[1];
        *opipe = out[0];
        *epipe = err[0];
        return pid;
    } else if (pid == 0) { /* child */
        _mys_close_fd(in[1]);
        _mys_close_fd(out[0]);
        _mys_close_fd(err[0]);
        _mys_close_fd(0);
        dup(in[0]);
        _mys_close_fd(1);
        dup(out[1]);
        _mys_close_fd(2);
        dup(err[1]);
        execl( "/bin/sh", "sh", "-c", command, NULL );
        _exit(1);
    } else
        goto error_fork;

    return pid;

error_fork:
    _mys_close_fd(err[0]);
    _mys_close_fd(err[1]);
error_err:
    _mys_close_fd(out[0]);
    _mys_close_fd(out[1]);
error_out:
    _mys_close_fd(in[0]);
    _mys_close_fd(in[1]);
error_in:
    return -1;
}

static int _mys_pclose_rwe(int pid, int ipipe, int opipe, int epipe)
{
    _mys_close_fd(ipipe);
    _mys_close_fd(opipe);
    _mys_close_fd(epipe);
    int status = -1;
    if (waitpid(pid, &status, 0) == pid)
        status = WEXITSTATUS(status);
    else
        status = -1;
    return status;
}

MYS_API mys_popen_t mys_popen_create(const char *argv)
{
    mys_popen_t result;
    result.ifd = -1;
    result.ofd = -1;
    result.efd = -1;
    result.pid = _mys_popen_rwe(&result.ifd, &result.ofd, &result.efd, argv);
    return result;
}

MYS_API int mys_popen_destroy(mys_popen_t *pd)
{
    if (pd == NULL)
        return 0;
    int result = _mys_pclose_rwe(pd->pid, pd->ifd, pd->ofd, pd->efd);
    pd->pid = -1;
    pd->ifd = -1;
    pd->ofd = -1;
    pd->efd = -1;
    return result;
}

static size_t _mys_readfd(char **buffer, FILE *fd)
{
    *buffer = NULL;
    size_t capacity = 0;
    size_t total_size = 0;
    char trunk[1024] = {0};
    while (fgets(trunk, sizeof(trunk) - 1, fd)) {
        size_t read_size = strnlen(trunk, sizeof(trunk));
        if (capacity < total_size + read_size) {
            // increase the buffer's capacity to put the new trunk
            capacity += read_size < 512 ? 512 : read_size;
            *buffer = (char *)realloc(*buffer, capacity);
        }
        // concat new trunk to buffer
        strncat(*buffer, trunk, read_size);
        total_size += read_size;
    }

    // size_t capacity = sizeof(trunk);
    // size_t total_size = 0;
    // while (fgets(trunk, sizeof(trunk)-1, fd)) {
    //     size_t read_size = strlen(trunk);
    //     if (capacity < total_size + read_size) {
    //         // increase the capacity to put the new trunk
    //         capacity += read_size < 512 ? 512 : read_size;
    //         *buffer = (char *)realloc(*buffer, capacity);
    //     }
    //     strncat(*buffer, trunk, capacity - total_size);
    //     total_size += read_size;
    // }
    return total_size;
}

MYS_API mys_prun_t mys_prun_create(const char *argv)
{
    mys_prun_t result;
    result.retval = -1;
    result.out = NULL;
    result.err = NULL;

    mys_popen_t pd = mys_popen_create(argv);

    FILE *outfd = fdopen(pd.ofd, "r");
    if (outfd) {
        _mys_readfd(&result.out, outfd);
        fclose(outfd);
    } else {
        result.out = (char *)malloc(0);
    }
    FILE *errfd = fdopen(pd.efd, "r");
    if (errfd) {
        _mys_readfd(&result.err, errfd);
        fclose(errfd);
    } else {
        result.err = (char *)malloc(0);
    }
    result.retval = mys_popen_destroy(&pd);
    return result;
}

MYS_API int mys_prun_destroy(mys_prun_t *pd)
{
    if (pd == NULL)
        return 0;
    if (pd->out != NULL)
        free((char *)pd->out);
    if (pd->err != NULL)
        free((char *)pd->err);
    pd->out = NULL;
    pd->err = NULL;
    pd->retval = -1;
    return 0;
}

MYS_API void mys_bfilename(const char *path, char **basename)
{
    const char *s = strrchr(path, '/');
    *basename = s ? strdup(s + 1) : strdup(path);
}

MYS_API int mys_do_mkdir(const char *path, mode_t mode)
{
    struct stat st;
    int  status = 0;
    if (stat(path, &st) != 0) {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0 && errno != EEXIST)
            status = -1;
    } else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        status = -1;
    }
    return status;
}

MYS_API int mys_ensure_dir(const char *path, mode_t mode)
{
    char *p = strdup(path);
    char *pp = p;
    char *sp = NULL;
    int status = 0;
    while (status == 0 && (sp = strchr(pp, '/')) != 0) {
        if (sp != pp) {
            *sp = '\0';
            status = mys_do_mkdir(p, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (status == 0)
        status = mys_do_mkdir(path, mode);
    free(p);
    return status;
}

MYS_API int mys_ensure_parent(const char *path, mode_t mode)
{
    char *dirc = strdup(path);
    // char *dname = dirname(dirc);
    // FIXME: why dname is not used?
    int status = mys_ensure_dir(dirc, mode);
    free(dirc);
    return status;
}

MYS_API int mys_busysleep(double seconds)
{
#if defined(POSIX_COMPLIANCE)
    /* https://stackoverflow.com/a/8158862 */
    struct timespec req;
    req.tv_sec = (uint64_t)seconds;
    req.tv_nsec = (uint64_t)((seconds - (double)req.tv_sec) * 1000000000);
    do {
        if(nanosleep(&req, &req) == 0)
            break;
        else if(errno != EINTR)
            return -1;
    } while (req.tv_sec > 0 || req.tv_nsec > 0);
    return 0;
#elif defined(OS_WINDOWS)
    /* https://stackoverflow.com/a/5801863 */
    /* https://stackoverflow.com/a/26945754 */
    LARGE_INTEGER fr,t1,t2;
    if (!QueryPerformanceFrequency(&fr))
        return -1;
    if (!QueryPerformanceCounter(&t1))
        return -1;
    do {
        if (!QueryPerformanceCounter(&t2))
            return -1;
    } while(((double)t2.QuadPart-(double)t1.QuadPart)/(double)fr.QuadPart < sec);
    return 0;
#else
    return -2;
#endif
}

MYS_API const char *mys_procname()
{
    static char *name = NULL;
    if (name != NULL)
        return name;

    char exe[1024];
    int pid = (int)getpid();
    snprintf(exe, sizeof(exe), "/proc/%d/exe", pid);
    size_t capacity = 128;
    char *buffer = (char *)malloc(sizeof(char) * capacity);
    ssize_t len = readlink(exe, buffer, capacity);
    while ((size_t)len >= (capacity - 1)) {
        capacity += 128;
        buffer = (char *)realloc(buffer, capacity);
        len = readlink(exe, buffer, capacity);
    }
    if (len <= 0) {
        char *reason = strerror(errno);
        int size = snprintf(NULL, 0, "<error_exe.pid=%d (%s)>", pid, reason);
        name = (char *)malloc(sizeof(char) * size);
        snprintf(name, size, "<error_exe.pid=%d (%s)>", pid, reason);
    } else {
        buffer[len] = '\0';
        mys_bfilename(buffer, &name);
    }
    return name;
}

MYS_API void mys_wait_flag(const char *file, int line, const char *flagfile)
{
    int myrank = mys_myrank();
    int nranks = mys_nranks();
    int digits = trunc(log10(nranks)) + 1;
    digits = digits > 3 ? digits : 3;
    if (myrank == 0) {
        fprintf(stdout, "[WAIT::%0*d %s:%03d] Use \"touch %s\" to continue... ",
            digits, 0, file, line, flagfile);
        fflush(stdout);
    }
    struct stat fstat;
    time_t last_modified = stat(flagfile, &fstat) == 0 ? fstat.st_mtime : 0;
    while (stat(flagfile, &fstat) == 0 && fstat.st_mtime <= last_modified)
        sleep(1);

    mys_barrier();
    if (myrank == 0) {
        fprintf(stdout, "OK\n");
        fflush(stdout);
    }
}

#if defined(OS_LINUX)
mys_thread_local char _mys_affinity_buffer[256];
MYS_API const char *mys_get_affinity() {
    int ncores = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if ((int)ncores > (int)sizeof(_mys_affinity_buffer))
        return NULL;

    cpu_set_t cpu_set;
    if (sched_getaffinity(0, sizeof(cpu_set_t), &cpu_set) == -1)
        return NULL;

    for (int i = 0; i < ncores; i++) {
        _mys_affinity_buffer[i] = CPU_ISSET(i, &cpu_set) ? '1' : '0';
}
    _mys_affinity_buffer[ncores] = '\0';
    return _mys_affinity_buffer;
}

MYS_API void mys_print_affinity(FILE *fd)
{
    int myrank = mys_myrank();
    int nranks = mys_nranks();
    for (int rank = 0; rank < nranks; rank++) {
#ifdef _OPENMP
        #pragma omp parallel
#endif
        if (myrank == rank) {
#ifdef _OPENMP
            int nthreads = omp_get_num_threads();
            int thread_id = omp_get_thread_num();
            #pragma omp for ordered schedule(static,1)
#else
            int nthreads = 1;
            int thread_id = 0;
#endif
            for (int t = 0; t < nthreads; t++) {
#ifdef _OPENMP
                #pragma omp ordered
#endif
                {
                    int rank_digits = trunc(log10(nranks)) + 1;
                    int thread_digits = trunc(log10(nthreads)) + 1;
                    rank_digits = rank_digits > 3 ? rank_digits : 3;
                    thread_digits = thread_digits > 1 ? thread_digits : 1;
                    const char *affinity = mys_get_affinity();
                    fprintf(fd, "rank=%0*d:%0*d affinity=%s\n", rank_digits, myrank, thread_digits, thread_id, affinity);
                    fflush(fd);
                }
            }
        }
        mys_barrier();
    }
}

MYS_API void mys_stick_affinity()
{
    int myrank = mys_myrank();
    int nranks = mys_nranks();
#ifdef _OPENMP
    #pragma omp parallel
#endif
    {
#ifdef _OPENMP
        int thread_id = omp_get_thread_num();
#else
        int thread_id = 0;
#endif
        const char *affinity = mys_get_affinity();
        int len = strnlen(affinity, INT_MAX);
        int count = 0;
        for (int i = 0; i < len; i++) {
            if (affinity[i] == '0')
                continue;
            if (count == thread_id) {
                cpu_set_t cpu_set;
                CPU_ZERO(&cpu_set);
                CPU_SET(i, &cpu_set);
                sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set);
                break;
            }
            count += 1;
        }
    }
}
#endif

MYS_API void mys_partition_naive(const int gs, const int ge, const int n, const int i, int *ls, int *le) {
    const int total = ge - gs;
    int size = total / n;
    int rest = total % n;
    (*ls) = gs;
    if (i < rest) {
        size += 1;
        (*ls) += (i * size);
    } else {
        (*ls) += rest + (i * size);
    }
    (*le) = (*ls) + size;
}

MYS_API double mys_arthimetic_mean(double *arr, int n)
{
    double sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return (1 / (double)n) * sum;
}

MYS_API double mys_harmonic_mean(double *arr, int n)
{
    double sum = 0;
    for (int i = 0; i < n; i++) {
        sum += 1 / arr[i];
    }
    return ((double)n) / sum;
}

MYS_API double mys_geometric_mean(double *arr, int n)
{
    double product = 1;
    for (int i = 0; i < n; i++) {
        product *= arr[i];
    }
    return pow(product, 1 / (double)n);
}

MYS_API double mys_standard_deviation(double *arr, int n)
{
    double xbar = arthimetic_mean(arr, n);
    double denom = 0;
    double nom = n - 1;
    for (int i = 0; i < n; i++) {
        double diff = arr[i] - xbar;
        denom += diff * diff;
    }
    return sqrt(denom / nom);
}

#ifdef MYS_USE_POSIX_MUTEX
MYS_API void mys_mutex_init(mys_mutex_t *lock)
{
    pthread_mutex_init(lock, NULL);
}

MYS_API void mys_mutex_destroy(mys_mutex_t *lock)
{
    pthread_mutex_destroy(lock);
}

MYS_API void mys_mutex_lock(mys_mutex_t *lock)
{
    pthread_mutex_lock(lock);
}

MYS_API void mys_mutex_unlock(mys_mutex_t *lock)
{
    pthread_mutex_unlock(lock);
}
#else
MYS_API void mys_mutex_init(mys_mutex_t *lock)
{
    __MYS_COMPARE_AND_SWAP(&lock->guard, __MYS_MUTEX_UNINITIALIZE, __MYS_MUTEX_IDLE);
    mys_memory_smp_mb();
}

MYS_API void mys_mutex_destroy(mys_mutex_t *lock)
{
    mys_mutex_lock(lock);
    lock->guard = __MYS_MUTEX_UNINITIALIZE;
    mys_mutex_unlock(lock);
}

MYS_API void mys_mutex_lock(mys_mutex_t *lock)
{
    while(__MYS_COMPARE_AND_SWAP(&lock->guard, __MYS_MUTEX_IDLE, __MYS_MUTEX_BUSY) != __MYS_MUTEX_IDLE)
        continue;
    mys_memory_smp_mb();
}

MYS_API void mys_mutex_unlock(mys_mutex_t *lock)
{
    while(__MYS_COMPARE_AND_SWAP(&lock->guard, __MYS_MUTEX_BUSY, __MYS_MUTEX_IDLE) != __MYS_MUTEX_BUSY)
        continue;
    mys_memory_smp_mb();
}
#endif /*MYS_USE_POSIX_MUTEX*/

static const char _mys_base64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static const unsigned char _mys_base64_map[256] =
{
    /* ASCII table */
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

MYS_API size_t mys_base64_encode_len(size_t plain_size)
{
    return ((plain_size + 2) / 3 * 4) + 1;
}

MYS_API size_t mys_base64_decode_len(const char *encoded_src)
{
    size_t nbytesdecoded;
    const unsigned char *s;
    size_t nprbytes;

    s = (const unsigned char *)encoded_src;
    while (_mys_base64_map[*(s++)] <= 63)
        ;

    nprbytes      = (s - (const unsigned char *)encoded_src) - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    return nbytesdecoded + 1;
}

#define _PCHK() do {                       \
    /* Save 1 char for NULL end */         \
    if ((char *)d >= dst + dst_size - 1) { \
        goto finish;                       \
    }                                      \
} while (0)

MYS_API size_t mys_base64_encode(char *dst, size_t dst_size, const char *src, size_t src_size)
{
    const char *tab = _mys_base64_table;
    char *d;
    size_t i;

    if (dst_size == 0 || src_size == 0)
        return 0;

    i = 0;
    d = dst;
    if (src_size > 2) {
        for (i = 0; i < src_size - 2; i += 3) {
            _PCHK(); *d++ = tab[(src[i] >> 2) & 0x3F];
            _PCHK(); *d++ = tab[((src[i] & 0x3) << 4) | ((int)(src[i + 1] & 0xF0) >> 4)];
            _PCHK(); *d++ = tab[((src[i + 1] & 0xF) << 2) | ((int)(src[i + 2] & 0xC0) >> 6)];
            _PCHK(); *d++ = tab[src[i + 2] & 0x3F];
        }
    }
    _PCHK();

    if (i < src_size) {
        *d++ = tab[(src[i] >> 2) & 0x3F];
        if (i == (src_size - 1)) {
            _PCHK(); *d++ = tab[((src[i] & 0x3) << 4)];
            _PCHK(); *d++ = '=';
        } else {
            _PCHK(); *d++ = tab[((src[i] & 0x3) << 4) | ((int)(src[i + 1] & 0xF0) >> 4)];
            _PCHK(); *d++ = tab[((src[i + 1] & 0xF) << 2)];
        }
        _PCHK(); *d++ = '=';
    }

finish:
    if (d >= dst + dst_size)
        d = dst + dst_size - 1;
    *d = '\0';
    return d - dst;
}

MYS_API size_t mys_base64_decode(char *dst, size_t dst_size, const char *src, size_t src_size)
{
    const unsigned char *map = _mys_base64_map;
    typedef unsigned char u8_t;
    const u8_t *s;
    u8_t *d;
    size_t nprbytes;

    if (dst_size == 0 || src_size == 0)
        return 0;

    d = (u8_t *)dst;
    s = (const u8_t *)src;
    while (map[*(s++)] <= 63)
        ;

    nprbytes = (s - (const u8_t *)src) - 1;
    s  = (const u8_t *)src;

    while (nprbytes > 4) {
        _PCHK(); *(d++) = (u8_t)(map[*s] << 2 | map[s[1]] >> 4);
        _PCHK(); *(d++) = (u8_t)(map[s[1]] << 4 | map[s[2]] >> 2);
        _PCHK(); *(d++) = (u8_t)(map[s[2]] << 6 | map[s[3]]);
        s += 4;
        nprbytes -= 4;
    }
    _PCHK();

    /* Note: (nprbytes == 1) would be an error, so just ingore that case */
    if (nprbytes > 1) {
        _PCHK(); *(d++) = (u8_t)(map[*s] << 2 | map[s[1]] >> 4);
    }
    if (nprbytes > 2) {
        _PCHK(); *(d++) = (u8_t)(map[s[1]] << 4 | map[s[2]] >> 2);
    }
    if (nprbytes > 3) {
        _PCHK(); *(d++) = (u8_t)(map[s[2]] << 6 | map[s[3]]);
    }
    _PCHK();

finish:
    if (d >= (u8_t *)dst + dst_size)
        d = (u8_t *)dst + dst_size - 1;
    *d = '\0';
    return d - (u8_t *)dst;
}

#undef _PCHK


#endif /*__MYS_C__*/
