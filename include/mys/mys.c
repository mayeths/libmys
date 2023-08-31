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
#include "_config.h"
#include "_hashtable.h"
// _mpi
#if defined(MYS_NO_MPI)
#include "_mpi/seq.c"
#else
#include "_mpi/par.c"
#endif
// _math
#include "_math/fdlibm_copysign.c"
#include "_math/fdlibm_fabs.c"
#include "_math/fdlibm_log.c"
#include "_math/fdlibm_log10.c"
#include "_math/fdlibm_sqrt.c"
#include "_math/fdlibm_scalbn.c"
#include "_math/fdlibm_pow.c"
#include "_math/musl_trunc.c"

#include "assert.h"
#include "base64.h"
#include "checkpoint.h"
#include "env.h"
#include "errno.h"
#include "hashfunction.h"
#include "hrtime.h"
#include "log.h"
#include "macro.h"
#include "memory.h"
#include "os.h"
#include "partition.h"
#include "rand.h"
#include "statistic.h"
#include "thread.h"

mys_thread_local int mys_errno = 0;

typedef struct _mys_mpi_G_t {
    bool inited;
    mys_mutex_t lock;
    int myrank;
    int nranks;
    _mys_MPI_Comm comm;
} _mys_mpi_G_t;

MYS_STATIC _mys_mpi_G_t _mys_mpi_G = {
    .inited = false,
    .lock = MYS_MUTEX_INITIALIZER,
    .myrank = -1,
    .nranks = -1,
    .comm = _mys_MPI_COMM_WORLD,
};

MYS_API void mys_mpi_init()
{
    if (_mys_mpi_G.inited == true)
        return;
    mys_mutex_lock(&_mys_mpi_G.lock);
    int inited;
    _mys_MPI_Initialized(&inited);
    if (!inited) {
        _mys_MPI_Init_thread(NULL, NULL, _mys_MPI_THREAD_SINGLE, &inited);
        fprintf(stdout, ">>>>> ===================================== <<<<<\n");
        fprintf(stdout, ">>>>> Nevel let libmys init MPI you dumbass <<<<<\n");
        fprintf(stdout, ">>>>> ===================================== <<<<<\n");
        fflush(stdout);
    }
    _mys_MPI_Comm_rank(_mys_mpi_G.comm, &_mys_mpi_G.myrank);
    _mys_MPI_Comm_size(_mys_mpi_G.comm, &_mys_mpi_G.nranks);
    _mys_mpi_G.inited = true;
    mys_mutex_unlock(&_mys_mpi_G.lock);
}

MYS_API int mys_mpi_myrank()
{
    mys_mpi_init();
    return _mys_mpi_G.myrank;
}

MYS_API int mys_mpi_nranks()
{
    mys_mpi_init();
    return _mys_mpi_G.nranks;
}

MYS_API int mys_mpi_barrier()
{
    mys_mpi_init();
    return _mys_MPI_Barrier(_mys_mpi_G.comm);
}

MYS_API int mys_mpi_sync()
{
    // At this point we use simple barrier for sync
    return mys_mpi_barrier();
}

MYS_API _mys_MPI_Comm mys_mpi_comm()
{
    return _mys_mpi_G.comm;
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


static void _mys_log_stdio_handler(mys_log_event_t *event, void *udata);

_mys_log_G_t _mys_log_G = {
    .inited = false,
    .lock = MYS_MUTEX_INITIALIZER,
    .level = MYS_LOG_TRACE,
    .silent = false,
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
    if (_mys_log_G.silent == true) {
        mys_mutex_unlock(&_mys_log_G.lock);
        return;
    }
    int myrank = mys_mpi_myrank();
    int nranks = mys_mpi_nranks();
    if (who == myrank && (int)level >= (int)_mys_log_G.level) {
        mys_log_event_t event;
        event.myrank = myrank;
        event.nranks = nranks;
        event.level = level;
        event.file = file;
        event.line = line;
        event.fmt = fmt;
        event.no_vargs = false;
        if (fmt == NULL) {
            event.level = MYS_LOG_FATAL;
            event.fmt = "Calling mys_log with NULL format string. Do you call LOG_SELF(0, \"...\") or LOG(rank, NULL)?";
            event.no_vargs = true;
        }
        va_start(event.vargs, fmt);
        mys_log_invoke_handlers(&event);
        va_end(event.vargs);
    }
    mys_mutex_unlock(&_mys_log_G.lock);
}

MYS_API void mys_log_ordered(int level, const char *file, int line, const char *fmt, ...)
{
    mys_log_init();
    mys_mutex_lock(&_mys_log_G.lock);
    if (_mys_log_G.silent == true) {
        mys_mutex_unlock(&_mys_log_G.lock);
        return;
    }
    int myrank = mys_mpi_myrank();
    int nranks = mys_mpi_nranks();
    _mys_MPI_Comm comm = mys_mpi_comm();

    const int tag = 65521; /*100000007 OpenMPI 4.1.0 on AArch64 throw invalid tag on large number*/

    if (myrank == 0) {
        mys_log_event_t event;
        event.myrank = myrank;
        event.nranks = nranks;
        event.level = level;
        event.file = file;
        event.line = line;
        event.fmt = fmt;
        event.no_vargs = false;
        if (fmt == NULL) {
            event.level = MYS_LOG_FATAL;
            event.fmt = "Calling mys_log with NULL format string. Do you call LOG_SELF(0, \"...\") or LOG(rank, NULL)?";
            event.no_vargs = true;
        }
        va_start(event.vargs, fmt);
        mys_log_invoke_handlers(&event);
        va_end(event.vargs);
        char buffer[4096];
        for (int rank = 1; rank < nranks; rank++) {
            _mys_MPI_Status status;
            int needed;
            _mys_MPI_Probe(rank, tag, comm, &status);
            _mys_MPI_Get_count(&status, _mys_MPI_CHAR, &needed);
            char *ptr = (needed > 4096) ? (char *)malloc(needed) : buffer;
            _mys_MPI_Recv(ptr, needed, _mys_MPI_CHAR, rank, tag, comm, _mys_MPI_STATUS_IGNORE);

            event.myrank = rank;
            event.fmt = ptr;
            event.no_vargs = true;
            mys_log_invoke_handlers(&event);
            if (ptr != buffer)
                free(ptr);
        }
        _mys_MPI_Barrier(comm);
    } else {
        char buffer[4096];
        va_list vargs, vargs_test;
        va_start(vargs, fmt);
        va_copy(vargs_test, vargs);
        int needed = vsnprintf(NULL, 0, fmt, vargs_test) + 1;
        va_end(vargs_test);
        char *ptr = (needed > 4096) ? (char *)malloc(needed) : buffer;
        vsnprintf(ptr, needed, fmt, vargs);
        _mys_MPI_Send(ptr, needed, _mys_MPI_CHAR, 0, tag, comm);
        if (ptr != buffer)
            free(ptr);
        va_end(vargs);
        _mys_MPI_Barrier(comm); // We don't expect logging increase processes' nondeterministic
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

MYS_API void mys_log_invoke_handlers(mys_log_event_t *event)
{
    for (int i = 0; i < 128; i++) {
        if (_mys_log_G.handlers[i].fn == NULL)
            break;
        _mys_log_G.handlers[i].fn(event, _mys_log_G.handlers[i].udata);
    }
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

MYS_API void mys_log_silent(bool silent)
{
    mys_log_init();
    mys_mutex_lock(&_mys_log_G.lock);
    _mys_log_G.silent = silent;
    mys_mutex_unlock(&_mys_log_G.lock);
}

MYS_API const char* mys_log_level_string(int level)
{
    const char *level_strings[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
    };
    return level_strings[(int)level];
}

static void _mys_log_stdio_handler(mys_log_event_t *event, void *udata) {
    FILE *file = udata != NULL ? (FILE *)udata : stdout;

    char base_label[256] = {'\0'};

    char *label = base_label;
    int label_size = sizeof(base_label);

    int rank_digits = _mys_math_trunc(_mys_math_log10(event->nranks)) + 1;
    int line_digits = _mys_math_trunc(_mys_math_log10(event->line)) + 1;
    rank_digits = rank_digits > 3 ? rank_digits : 3;
    line_digits = line_digits > 3 ? line_digits : 3;

    if ((int)event->level < (int)MYS_LOG_RAW) {
        const char *lstr = mys_log_level_string(event->level);
        const char level_shortname = lstr[0];
        snprintf(label, label_size, "[%c::%0*d %s:%0*d]",
            level_shortname, rank_digits, event->myrank,
            event->file, line_digits, event->line
        );
#ifndef MYS_LOG_NO_COLOR
        const char *level_colors[] = {
            MCOLOR_GREEN, MCOLOR_PURPLE, MCOLOR_CYAN,
            MCOLOR_YELLO, MCOLOR_RED, MCOLOR_B_RED,
        };
        char colorized_label[sizeof(base_label) + 128];
        if (isatty(fileno(file))) {
            snprintf(colorized_label, sizeof(colorized_label), "%s%s" MCOLOR_NO,
                level_colors[(int)event->level], label
            );
            label = colorized_label;
            label_size = sizeof(colorized_label);
        }
#endif
        fprintf(file, "%s ", label);
    }

    if (event->no_vargs) {
        fprintf(file, "%s", event->fmt);
    } else {
        vfprintf(file, event->fmt, event->vargs);
    }
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
    char *pathcopy = strdup(path);
    char *dname = dirname(pathcopy);
    int status = mys_ensure_dir(dname, mode);
    free(pathcopy);
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
    int myrank = mys_mpi_myrank();
    int nranks = mys_mpi_nranks();
    int digits = _mys_math_trunc(_mys_math_log10(nranks)) + 1;
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

    mys_mpi_barrier();
    if (myrank == 0) {
        fprintf(stdout, "OK\n");
        fflush(stdout);
    }
}

#if !defined(MYS_NO_LEGACY) && !defined(MYS_NO_LEGACY_OS)
MYS_API popen_t popen_create(const char *argv)
{
    return mys_popen_create(argv);
}

MYS_API prun_t prun_create(const char *argv)
{
    return mys_prun_create(argv);
}

MYS_API int prun_destroy(prun_t *pd)
{
    return mys_prun_destroy(pd);
}

MYS_API int busysleep(double sec)
{
    return mys_busysleep(sec);
}

MYS_API char *bfilename(const char *path)
{
    char *r; mys_bfilename(path, &r); return r;
}

MYS_API const char *procname()
{
    return mys_procname();
}

MYS_API int do_mkdir(const char *path, mode_t mode)
{
    return mys_do_mkdir(path, mode);
}

MYS_API int ensuredir(const char *path, mode_t mode)
{
    return mys_ensure_dir(path, mode);
}

MYS_API int ensureparent(const char *path, mode_t mode)
{
    return mys_ensure_parent(path, mode);
}
#endif

#ifdef MYS_ENABLE_AFFINITY
#include <sched.h>
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
    int myrank = mys_mpi_myrank();
    int nranks = mys_mpi_nranks();
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
                    int rank_digits = _mys_math_trunc(_mys_math_log10(nranks)) + 1;
                    int thread_digits = _mys_math_trunc(_mys_math_log10(nthreads)) + 1;
                    rank_digits = rank_digits > 3 ? rank_digits : 3;
                    thread_digits = thread_digits > 1 ? thread_digits : 1;
                    const char *affinity = mys_get_affinity();
                    fprintf(fd, "rank=%0*d:%0*d affinity=%s\n", rank_digits, myrank, thread_digits, thread_id, affinity);
                    fflush(fd);
                }
            }
        }
        mys_mpi_barrier();
    }
}

MYS_API void mys_stick_affinity()
{
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

#if !defined(MYS_NO_LEGACY) && !defined(MYS_NO_LEGACY_PARTITION)
MYS_API int partition1DSimple(
    const int start, const int end,
    const int nworkers, const int workerid,
    int *wstart, int *wend /* return values */
) {
    mys_partition_naive(start, end, nworkers, workerid, wstart, wend);
    return 0;
}
#endif

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
    return _mys_math_pow(product, 1 / (double)n);
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
    return _mys_math_sqrt(denom / nom);
}

MYS_API mys_aggregate_t mys_aggregate_analysis(double value)
{
    mys_aggregate_t result;
    result.mine = value;
    // result.max = value;
    // result.min = value;
    // result.avg = value;
    // result.sum = value;
    // result.std = 0;
    // result.var = 0;
    mys_mpi_init();
    _mys_MPI_Allreduce(&value, &result.max, 1, _mys_MPI_DOUBLE, _mys_MPI_MAX, _mys_mpi_G.comm);
    _mys_MPI_Allreduce(&value, &result.min, 1, _mys_MPI_DOUBLE, _mys_MPI_MIN, _mys_mpi_G.comm);
    _mys_MPI_Allreduce(&value, &result.sum, 1, _mys_MPI_DOUBLE, _mys_MPI_SUM, _mys_mpi_G.comm);
    result.avg = result.sum / (double)_mys_mpi_G.nranks;
    double tmp = (value - result.avg) * (value - result.avg);
    _mys_MPI_Allreduce(&tmp, &result.var, 1, _mys_MPI_DOUBLE, _mys_MPI_SUM, _mys_mpi_G.comm);
    result.var = result.var / (double)_mys_mpi_G.nranks;
    result.std = _mys_math_sqrt(result.var);
    return result;
}


#if !defined(MYS_NO_LEGACY) && !defined(MYS_NO_LEGACY_STATISTIC)
MYS_API double arthimetic_mean(double *arr, int n)
{
    return mys_arthimetic_mean(arr, n);
}

MYS_API double harmonic_mean(double *arr, int n)
{
    return mys_harmonic_mean(arr, n);
}

MYS_API double geometric_mean(double *arr, int n)
{
    return mys_geometric_mean(arr, n);
}

MYS_API double standard_deviation(double *arr, int n)
{
    return mys_standard_deviation(arr, n);
}
#endif

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

#define _PCHK() do {                      \
    /* Save 1 char for NULL end */        \
    size_t pos = dst_size - 1;            \
    if ((char *)d >= (char *)dst + pos) { \
        goto finish;                      \
    }                                     \
} while (0)

MYS_API size_t mys_base64_encode(char *dst, size_t dst_size, const void *src, size_t src_size)
{
    const char *tab = _mys_base64_table;
    char *d;
    char *s;
    size_t i;

    if (dst_size == 0 || src_size == 0)
        return 0;

    i = 0;
    s = (char *)src;
    d = dst;
    if (src_size > 2) {
        for (i = 0; i < src_size - 2; i += 3) {
            _PCHK(); *d++ = tab[(s[i] >> 2) & 0x3F];
            _PCHK(); *d++ = tab[((s[i] & 0x3) << 4) | ((int)(s[i + 1] & 0xF0) >> 4)];
            _PCHK(); *d++ = tab[((s[i + 1] & 0xF) << 2) | ((int)(s[i + 2] & 0xC0) >> 6)];
            _PCHK(); *d++ = tab[s[i + 2] & 0x3F];
        }
    }
    _PCHK();

    if (i < src_size) {
        *d++ = tab[(s[i] >> 2) & 0x3F];
        if (i == (src_size - 1)) {
            _PCHK(); *d++ = tab[((s[i] & 0x3) << 4)];
            _PCHK(); *d++ = '=';
        } else {
            _PCHK(); *d++ = tab[((s[i] & 0x3) << 4) | ((int)(s[i + 1] & 0xF0) >> 4)];
            _PCHK(); *d++ = tab[((s[i + 1] & 0xF) << 2)];
        }
        _PCHK(); *d++ = '=';
    }

finish:
    if (d >= dst + dst_size)
        d = dst + dst_size - 1;
    *d = '\0';
    return d - dst;
}

MYS_API size_t mys_base64_decode(void *dst, size_t dst_size, const char *src, size_t src_size)
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

#define _ROTL(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define _ROTR(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define _CH(x,y,z)  (((x) & (y)) ^ (~(x) & (z)))
#define _MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define _EP0(x)     (_ROTR(x,2) ^ _ROTR(x,13) ^ _ROTR(x,22))
#define _EP1(x)     (_ROTR(x,6) ^ _ROTR(x,11) ^ _ROTR(x,25))
#define _SIG0(x)    (_ROTR(x,7) ^ _ROTR(x,18) ^ ((x) >> 3))
#define _SIG1(x)    (_ROTR(x,17) ^ _ROTR(x,19) ^ ((x) >> 10))

static const unsigned int _mys_sha256_k[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void _mys_sha256_trans(mys_sha256_ctx_t *ctx, const unsigned char data[])
{
    unsigned int a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    for ( ; i < 64; ++i)
        m[i] = _SIG1(m[i - 2]) + m[i - 7] + _SIG0(m[i - 15]) + m[i - 16];

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; ++i) {
        t1 = h + _EP1(e) + _CH(e,f,g) + _mys_sha256_k[i] + m[i];
        t2 = _EP0(a) + _MAJ(a,b,c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

MYS_API void mys_sha256_init(mys_sha256_ctx_t *ctx)
{
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
}

MYS_API void mys_sha256_update(mys_sha256_ctx_t *ctx, const void *data, size_t len)
{
    unsigned int i;
    const unsigned char *raw = (const unsigned char *)data;

    for (i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = raw[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            _mys_sha256_trans(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

MYS_API void mys_sha256_update_i8(mys_sha256_ctx_t *ctx, const int8_t data)
{
    mys_sha256_update(ctx, &data, sizeof(int8_t));
}
MYS_API void mys_sha256_update_i16(mys_sha256_ctx_t *ctx, const int16_t data)
{
    mys_sha256_update(ctx, &data, sizeof(int16_t));
}
MYS_API void mys_sha256_update_i32(mys_sha256_ctx_t *ctx, const int32_t data)
{
    mys_sha256_update(ctx, &data, sizeof(int32_t));
}
MYS_API void mys_sha256_update_i64(mys_sha256_ctx_t *ctx, const int64_t data)
{
    mys_sha256_update(ctx, &data, sizeof(int64_t));
}
MYS_API void mys_sha256_update_u8(mys_sha256_ctx_t *ctx, const uint8_t data)
{
    mys_sha256_update(ctx, &data, sizeof(uint8_t));
}
MYS_API void mys_sha256_update_u16(mys_sha256_ctx_t *ctx, const uint16_t data)
{
    mys_sha256_update(ctx, &data, sizeof(uint16_t));
}
MYS_API void mys_sha256_update_u32(mys_sha256_ctx_t *ctx, const uint32_t data)
{
    mys_sha256_update(ctx, &data, sizeof(uint32_t));
}
MYS_API void mys_sha256_update_u64(mys_sha256_ctx_t *ctx, const uint64_t data)
{
    mys_sha256_update(ctx, &data, sizeof(uint64_t));
}
MYS_API void mys_sha256_update_f32(mys_sha256_ctx_t *ctx, const float data)
{
    mys_sha256_update(ctx, &data, sizeof(float));
}
MYS_API void mys_sha256_update_f64(mys_sha256_ctx_t *ctx, const double data)
{
    mys_sha256_update(ctx, &data, sizeof(double));
}
MYS_API void mys_sha256_update_int(mys_sha256_ctx_t *ctx, const int data)
{
    mys_sha256_update(ctx, &data, sizeof(int));
}
MYS_API void mys_sha256_update_float(mys_sha256_ctx_t *ctx, const float data)
{
    mys_sha256_update(ctx, &data, sizeof(float));
}
MYS_API void mys_sha256_update_double(mys_sha256_ctx_t *ctx, const double data)
{
    mys_sha256_update(ctx, &data, sizeof(double));
}
MYS_API void mys_sha256_update_char(mys_sha256_ctx_t *ctx, const char data)
{
    mys_sha256_update(ctx, &data, sizeof(char));
}
MYS_API void mys_sha256_update_ptr(mys_sha256_ctx_t *ctx, const void *data)
{
    mys_sha256_update(ctx, &data, sizeof(void *));
}
MYS_API void mys_sha256_update_arr(mys_sha256_ctx_t *ctx, const void *data, size_t size)
{
    mys_sha256_update(ctx, &data, size);
}

MYS_API void mys_sha256_dump_bin(mys_sha256_ctx_t *ctx, void *outbin)
{
    mys_sha256_ctx_t ictx;
    memcpy(&ictx, ctx, sizeof(mys_sha256_ctx_t));

    unsigned int i;
    unsigned char *hash = (unsigned char *)outbin;

    i = ictx.datalen;

    // Pad whatever data is left in the buffer.
    if (ictx.datalen < 56) {
        ictx.data[i++] = 0x80;
        while (i < 56)
            ictx.data[i++] = 0x00;
    }
    else {
        ictx.data[i++] = 0x80;
        while (i < 64)
            ictx.data[i++] = 0x00;
        _mys_sha256_trans(&ictx, ictx.data);
        memset(ictx.data, 0, 56);
    }

    // Append to the padding the total message's length in bits and transform.
    ictx.bitlen += ictx.datalen * 8;
    ictx.data[63] = ictx.bitlen;
    ictx.data[62] = ictx.bitlen >> 8;
    ictx.data[61] = ictx.bitlen >> 16;
    ictx.data[60] = ictx.bitlen >> 24;
    ictx.data[59] = ictx.bitlen >> 32;
    ictx.data[58] = ictx.bitlen >> 40;
    ictx.data[57] = ictx.bitlen >> 48;
    ictx.data[56] = ictx.bitlen >> 56;
    _mys_sha256_trans(&ictx, ictx.data);

    // Since this implementation uses little endian byte ordering and SHA uses big endian,
    // reverse all the bytes when copying the final state to the output hash.
    for (i = 0; i < 4; ++i) {
        hash[i]      = (ictx.state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4]  = (ictx.state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8]  = (ictx.state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (ictx.state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (ictx.state[4] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 20] = (ictx.state[5] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 24] = (ictx.state[6] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 28] = (ictx.state[7] >> (24 - i * 8)) & 0x000000ff;
    }
}

static const char _mys_hex_table[] = "0123456789abcdef";

MYS_API void mys_sha256_dump_hex(mys_sha256_ctx_t *ctx, void *outhex)
{
    uint8_t outbin[MYS_SHA256_BIN_SIZE];
    mys_sha256_dump_bin(ctx, outbin);
    char *out = (char *)outhex;

    for (int i = 0; i < MYS_SHA256_BIN_SIZE; i++) {
        const char byte = outbin[i];
        uint32_t hi = (byte & 0xF0) >> 4;
        uint32_t lo = (byte & 0x0F) >> 0;
        *(out++) = _mys_hex_table[hi];
        *(out++) = _mys_hex_table[lo];
    }
    *out = '\0';
}

MYS_API void mys_sha256_dump_base64(mys_sha256_ctx_t *ctx, void *outbase64)
{
    uint8_t outbin[MYS_SHA256_BIN_SIZE];
    mys_sha256_dump_bin(ctx, outbin);
    mys_base64_encode((char *)outbase64, MYS_SHA256_BASE64_SIZE, outbin, MYS_SHA256_BIN_SIZE);
}

MYS_API void mys_sha256_bin(const void *text, size_t size, uint8_t output[MYS_SHA256_BIN_SIZE])
{
    mys_sha256_ctx_t ctx;
    mys_sha256_init(&ctx);
    mys_sha256_update(&ctx, text, size);
    mys_sha256_dump_bin(&ctx, (void *)output);
}

MYS_API void mys_sha256_base64(const void *text, size_t size, char output[MYS_SHA256_BASE64_SIZE])
{
    mys_sha256_ctx_t ctx;
    mys_sha256_init(&ctx);
    mys_sha256_update(&ctx, text, size);
    mys_sha256_dump_base64(&ctx, (void *)output);
}

MYS_API void mys_sha256_hex(const void *text, size_t size, char output[MYS_SHA256_HEX_SIZE])
{
    mys_sha256_ctx_t ctx;
    mys_sha256_init(&ctx);
    mys_sha256_update(&ctx, text, size);
    mys_sha256_dump_hex(&ctx, (void *)output);
}

// https://troydhanson.github.io/uthash/userguide.html#_string_keys
typedef struct _mys_chk_name_t {
    char *name;
    _mys_UT_hash_handle hh;
} _mys_chk_name_t;

static _mys_chk_name_t *checkpoint_name_insert(_mys_chk_name_t **head, const char *name)
{
    _mys_chk_name_t *s = (_mys_chk_name_t *)malloc(sizeof(_mys_chk_name_t));
    s->name = strndup(name, 4096);
    _HASH_ADD_KEYPTR(hh, *head, s->name, strlen(s->name), s);
    return s;
}

static void checkpoint_name_clear(_mys_chk_name_t **head) {
    _mys_chk_name_t *s = NULL;
    _mys_chk_name_t *tmp = NULL;
    _HASH_ITER(hh, *head, s, tmp) {
        _HASH_DEL(*head, s);
        free(s->name);
        free(s);
    }
}

static _mys_chk_name_t *checkpoint_name_find(_mys_chk_name_t *head, const char *name)
{
    _mys_chk_name_t *s = NULL;
    _HASH_FIND_STR(head, name, s);
    return s;
}

typedef struct _mys_chk_t {
    char *name;
    double time;
} _mys_chk_t;

typedef struct _mys_chk_G_t {
    bool inited;
    mys_mutex_t lock;
    double offset;
    size_t size;
    size_t capacity;
    _mys_chk_t *arr;
    _mys_chk_name_t *nameset;
} _mys_chk_G_t;

static _mys_chk_G_t _mys_chk_G = {
    .inited = false,
    .lock = MYS_MUTEX_INITIALIZER,
    .offset = 0,
    .size = 0,
    .capacity = 0,
    .arr = NULL,
    .nameset = NULL,
};

MYS_API void mys_checkpoint_init()
{
    if (_mys_chk_G.inited == true)
        return;
    mys_mutex_lock(&_mys_chk_G.lock);
    if (_mys_chk_G.inited == true)
        return;
    _mys_chk_G.offset = mys_hrtime();
    _mys_chk_G.size = 0;
    _mys_chk_G.capacity = 0;
    _mys_chk_G.arr = NULL;
    _mys_chk_G.nameset = NULL;
    _mys_chk_G.inited = true;
    mys_mutex_unlock(&_mys_chk_G.lock);
}


MYS_API void mys_checkpoint_reset()
{
    mys_checkpoint_init();
    mys_mutex_lock(&_mys_chk_G.lock);
    _mys_chk_G.offset = mys_hrtime();
    _mys_chk_G.size = 0;
    _mys_chk_G.capacity = 0;
    if (_mys_chk_G.arr != NULL) free(_mys_chk_G.arr);
    if (_mys_chk_G.nameset != NULL) checkpoint_name_clear(&_mys_chk_G.nameset);
    _mys_chk_G.arr = NULL;
    _mys_chk_G.nameset = NULL;
    mys_mutex_unlock(&_mys_chk_G.lock);
}

MYS_API void mys_checkpoint(const char *name_format, ...)
{
    mys_checkpoint_init();
    char name[4096];
    va_list args;
    va_start(args, name_format);
    vsnprintf(name, sizeof(name), name_format, args);
    va_end(args);

    mys_mutex_lock(&_mys_chk_G.lock);

    _mys_chk_name_t *child = checkpoint_name_find(_mys_chk_G.nameset, name);
    if (child == NULL)
        child = checkpoint_name_insert(&_mys_chk_G.nameset, name);

    if (_mys_chk_G.size == _mys_chk_G.capacity) {
        _mys_chk_G.capacity = (_mys_chk_G.capacity == 0) ? 128 : _mys_chk_G.capacity * 2;
        size_t bytes = sizeof(_mys_chk_t) * _mys_chk_G.capacity;
        _mys_chk_G.arr = (_mys_chk_t *)realloc(_mys_chk_G.arr, bytes);
    }

    double current = mys_hrtime() - _mys_chk_G.offset;
    _mys_chk_G.arr[_mys_chk_G.size].time = current;
    _mys_chk_G.arr[_mys_chk_G.size].name = child->name;
    _mys_chk_G.size += 1;
    mys_mutex_unlock(&_mys_chk_G.lock);
}

MYS_API int mys_checkpoint_dump(const char *file_format, ...)
{
    mys_checkpoint_init();
    char file[4096];
    va_list args;
    va_start(args, file_format);
    vsnprintf(file, sizeof(file), file_format, args);
    va_end(args);

    mys_mutex_lock(&_mys_chk_G.lock);
    mys_ensure_parent(file, 0777);
    FILE *fd = fopen(file, "w");
    if (fd == NULL)
        return 1;
    fprintf(fd, "name,time\n");
    for (size_t i = 0; i < _mys_chk_G.size; i++) {
        const char *checkpoint_name = _mys_chk_G.arr[i].name;
        double time = _mys_chk_G.arr[i].time;
        fprintf(fd, "%s,%.17e\n", checkpoint_name, time);
    }
    fclose(fd);
    ILOG(0, "Checkpoints Wrote to %s", file);
    mys_mutex_unlock(&_mys_chk_G.lock);

    return 0;
}

#define _UTHASH_UNDEF
#include "_hashtable.h"

#endif /*__MYS_C__*/
