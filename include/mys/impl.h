/**
 * @file impl.h
 * @author mayeths (wow@mayeths.com)
 * @brief Implement non-static variables and fucntions
 * 
 * The biggest benifit of using standalone "impl.h"
 * instead of #ifdef MYS_IMPL in all other headers are,
 * we can use `#pragma once` in these other headers,
 * and the MYS_IMPL macro is still processed correctly
 */
#ifndef __MYS_IMPL_H__
#define __MYS_IMPL_H__

/*********************************************/
// C definition
/*********************************************/
#include "thread.h"
#include "errno.h"
#include "myspi.h"
#include "log.h"
#include "rand.h"

mys_thread_local int mys_errno = 0;


mys_myspi_G_t mys_myspi_G = {
    .inited = false,
    .lock = MYS_MUTEX_INITIALIZER,
    .myrank = -1,
    .nranks = -1,
};

MYS_API void mys_myspi_init()
{
    if (mys_myspi_G.inited == true)
        return;
    mys_mutex_lock(&mys_myspi_G.lock);
#if defined(MYS_NO_MPI)
    mys_myspi_G.myrank = 0;
    mys_myspi_G.nranks = 1;
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
    MPI_Comm_size(MPI_COMM_WORLD, &mys_myspi_G.nranks);
    MPI_Comm_rank(MPI_COMM_WORLD, &mys_myspi_G.myrank);
#endif
    mys_myspi_G.inited = true;
    mys_mutex_unlock(&mys_myspi_G.lock);
}

MYS_API int mys_myrank()
{
    mys_myspi_init();
    return mys_myspi_G.myrank;
}

MYS_API int mys_nranks()
{
    mys_myspi_init();
    return mys_myspi_G.nranks;
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
    const char *lstr = mys_log_level_string(event->level);
    const char level_shortname = lstr[0];
    FILE *file = event->udata != NULL ? (FILE *)event->udata : stdout;

    char base_label[256];

    char *label = base_label;
    int label_size = sizeof(base_label);

    int myrank = mys_myrank();
    int nranks = mys_nranks();
    int rank_digits = trunc(log10(nranks)) + 1;
    int line_digits = trunc(log10(event->line)) + 1;
    rank_digits = rank_digits > 3 ? rank_digits : 3;
    line_digits = line_digits > 3 ? line_digits : 3;

    snprintf(label, label_size, "[%c::%0*d %s:%0*d] ",
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

    fprintf(file, "%s", label);
    vfprintf(file, event->fmt, event->vargs);
    fprintf(file, "\n");
    fflush(file);
}


MYS_API inline const char *hrname()
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

MYS_API inline uint64_t hrtick()
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

MYS_API inline uint64_t hrfreq()
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

MYS_API inline double hrtime()
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

#if !defined(MYS_NO_MPI)
#include <mpi.h>
extern double _mys_hrstart_mpi;
MYS_API const char *mys_hrname_mpi() {
    return "High-resolution timer by <mpi.h> (1us~10us)";
}
MYS_API uint64_t mys_hrtick_mpi() {
    if (_mys_hrstart_mpi < 0)
        _mys_hrstart_mpi = MPI_Wtime();
    double current = MPI_Wtime() - _mys_hrstart_mpi;
    return (uint64_t)(current * 1e9); // in nano second
}
MYS_API uint64_t mys_hrfreq_mpi() {
    return (uint64_t)1000000000;
}
MYS_API double mys_hrtime_mpi() {
    return (double)hrtick() / (double)hrfreq();
}
#endif

#if defined(_OPENMP)
#include <omp.h>
extern double _mys_hrstart_openmp;
MYS_API const char *mys_hrname_openmp() {
    return "High-resolution timer by <omp.h> (1us~10us)";
}
MYS_API uint64_t mys_hrtick_openmp() {
    if (_mys_hrstart_openmp < 0)
        _mys_hrstart_openmp = omp_get_wtime();
    double current = omp_get_wtime() - _mys_hrstart_openmp;
    return (uint64_t)(current * 1e9); // in nano second
}
MYS_API uint64_t mys_hrfreq_openmp() {
    return (uint64_t)1000000000;
}
MYS_API double mys_hrtime_openmp() {
    return (double)hrtick() / (double)hrfreq();
}
#endif

#endif /*__MYS_IMPL_H__*/
