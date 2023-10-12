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
#define _UTHASH_DEFINE_HASH
#define _UTHASH_DEFINE_LIST
#include "_lib/index.h"
#include "assert.h"
#include "atomic.h"
#include "base64.h"
#include "checkpoint.h"
#include "debug.h"
#include "env.h"
#include "hashfunction.h"
#include "hrtime.h"
#include "log.h"
#include "macro.h"
#include "memory.h"
#ifndef MYS_NO_MPI
#include "mpiz.h"
#endif
#include "os.h"
#include "partition.h"
#include "rand.h"
#include "statistic.h"
#include "string.h"
#include "thread.h"

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


struct _mys_rand_G_t {
    bool inited;
    uint64_t seed[2];
};

mys_thread_local struct _mys_rand_G_t _mys_rand_G = {
    .inited = false,
    .seed = {0, 0},
};

static void _mys_rand_init()
{
    if (_mys_rand_G.inited == true)
        return;
    _mys_rand_G.seed[0] = 0;
    _mys_rand_G.seed[1] = 1;
    _mys_rand_G.inited = true;
}

MYS_API void mys_rand_srand(uint64_t a0, uint64_t a1)
{
    _mys_rand_init();
    _mys_rand_G.seed[0] = a0;
    _mys_rand_G.seed[1] = a1;
}

MYS_API void mys_rand_srand_hardware()
{
    uint64_t t;
#if defined(ARCH_X64)
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    t = ((uint64_t)hi << 32) | (uint64_t)lo;
#elif defined(ARCH_AARCH64)
    __asm__ __volatile__("mrs %0, CNTVCT_EL0" : "=r"(t));
#else
    t = (uint64_t)time(NULL);
#endif
    /* A(1010)5(0101) won't INVALID(1111_1111) again */
    uint64_t a0 = (t << 32) | (t & 0xAAAA5555);
    uint64_t a1 = UINT64_MAX - a0;
    mys_rand_srand(a0, a1);
}

static inline uint64_t _mys_rotl(const uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}
MYS_API uint64_t mys_rand_xoroshiro128ss()
{
    _mys_rand_init();
    const uint64_t s0 = _mys_rand_G.seed[0];
    uint64_t s1 = _mys_rand_G.seed[1];
    const uint64_t result = _mys_rotl(s0 * 5, 7) * 9;
    s1 ^= s0;
    _mys_rand_G.seed[0] = _mys_rotl(s0, 24) ^ s1 ^ (s1 << 16);
    _mys_rand_G.seed[1] = _mys_rotl(s1, 37);
    return result;
}

MYS_API uint64_t mys_rand_u64(uint64_t mi, uint64_t ma)
{
    uint64_t v = mys_rand_xoroshiro128ss();
    if (ma <= mi)
        return mi;
    if (mi == 0 && ma == UINT64_MAX)
        return v;
    return mi + v % (ma - mi);
}

MYS_API uint32_t mys_rand_u32(uint32_t mi, uint32_t ma) { return (uint32_t)mys_rand_u64(mi, ma); }
MYS_API uint16_t mys_rand_u16(uint16_t mi, uint16_t ma) { return (uint16_t)mys_rand_u64(mi, ma); }
MYS_API uint8_t  mys_rand_u8 (uint8_t  mi, uint8_t  ma) { return (uint8_t )mys_rand_u64(mi, ma); }

MYS_API int64_t mys_rand_i64(int64_t mi, int64_t ma)
{
    union _mys_dicast_t { uint64_t u64; int64_t i64; } v;
    v.u64 = mys_rand_xoroshiro128ss();
    if (ma <= mi)
        return mi;
    if (mi == INT64_MIN && ma == INT64_MAX)
        return v.i64;
    return mi + v.i64 % (ma - mi);
}

MYS_API int32_t mys_rand_i32(int32_t mi, int32_t ma) { return (int32_t)mys_rand_i64(mi, ma); }
MYS_API int16_t mys_rand_i16(int16_t mi, int16_t ma) { return (int16_t)mys_rand_i64(mi, ma); }
MYS_API int8_t  mys_rand_i8 (int8_t  mi, int8_t  ma) { return (int8_t )mys_rand_i64(mi, ma); }

MYS_API size_t mys_rand_sizet(size_t mi, size_t ma)
{
#if SIZE_MAX == UINT64_MAX
    return (size_t)mys_rand_u64(mi, ma);
#elif SIZE_MAX == UINT32_MAX
    return (size_t)mys_rand_u32(mi, ma);
#else
#error Invalid SIZE_MAX
#endif
}

MYS_API ssize_t mys_rand_ssizet(ssize_t mi, ssize_t ma)
{
#if SSIZE_MAX == INT64_MAX
    return (ssize_t)mys_rand_i64(mi, ma);
#elif SSIZE_MAX == INT32_MAX
    return (ssize_t)mys_rand_i32(mi, ma);
#else
#error Invalid SSIZE_MAX
#endif
}

MYS_API double mys_rand_f64(double mi, double ma)
{
    uint64_t v = mys_rand_xoroshiro128ss();
    if (ma <= mi)
        return mi;
    // See PostgreSQL pg_prng_double()
    double p252 = 2.2204460492503131e-16; // pow(2, -52)
    double v01 = (double)(v >> (64 - 52)) * p252; // [0, 1)
    return mi + v01 * (ma - mi); // [mi, ma] due to IEEE 754 rounding
}

MYS_API float mys_rand_f32(float mi, float ma) {
    uint32_t v = (uint32_t)mys_rand_xoroshiro128ss();
    if (ma <= mi)
        return mi;
    float p223 = 1.1920928955078125e-07; // pow(2, -23)
    float v01 = (float)(v >> (32 - 23)) * p223; // [0, 1)
    return mi + v01 * (ma - mi); // [mi, ma] due to IEEE 754 rounding
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


#define _MYS_FNAME_MAX 256
#define _MYS_RANK_LOG_DEST_NUM 8

typedef struct {
    bool inited;
    mys_mutex_t lock;
    int max_used;
    struct {
        FILE *file;
        char folder[_MYS_FNAME_MAX];
        size_t tot_wrote;
        size_t cur_wrote;
    } dests[_MYS_RANK_LOG_DEST_NUM];
} _mys_rank_log_G_t;

_mys_rank_log_G_t _mys_rank_log_G = {
    .inited = false,
    .lock = MYS_MUTEX_INITIALIZER,
    .max_used = 0,
    .dests = {
        { .file = NULL, .folder = { '\0' }, .tot_wrote = 0, .cur_wrote = 0 },
    },
};

MYS_STATIC void _mys_rank_log_init()
{
    if (_mys_rank_log_G.inited == true)
        return;
    mys_mutex_lock(&_mys_rank_log_G.lock);
    _mys_rank_log_G.inited = true;
    mys_mutex_unlock(&_mys_rank_log_G.lock);
}

MYS_STATIC int _mys_rank_log_find_dest(const char *folder, size_t len)
{
    int index = -1;
    _mys_rank_log_init();
    mys_mutex_lock(&_mys_rank_log_G.lock);
    for (int i = 0; i < _mys_rank_log_G.max_used; i++) {
        if (strncmp(_mys_rank_log_G.dests[i].folder, folder, len) == 0) {
            index = i;
            break;
        }
    }
    mys_mutex_unlock(&_mys_rank_log_G.lock);
    return index;
}

MYS_API void mys_rank_log(const char *callfile, int callline, const char *folder, const char *fmt, ...)
{
    _mys_rank_log_init();
    va_list vargs;
    size_t wrote = 0;
    size_t len = strnlen(folder, _MYS_FNAME_MAX);
    int index = _mys_rank_log_find_dest(folder, len);
    mys_mutex_lock(&_mys_rank_log_G.lock);
    if (index == -1) {
        mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, callfile, callline, "Invoked mys_rank_log() with invalid folder: %s", folder);
        goto _finished;
    }
    if (_mys_rank_log_G.dests[index].file == NULL) {
        mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, callfile, callline, "Invoked mys_rank_log() with closed folder: %s", folder);
        goto _finished;
    }
    if (fmt == NULL) {
        mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, callfile, callline, "Invoked mys_rank_log() with NULL format.");
        goto _finished;
    }

    va_start(vargs, fmt);
    wrote += vfprintf(_mys_rank_log_G.dests[index].file, fmt, vargs);
    va_end(vargs);
    wrote += fprintf(_mys_rank_log_G.dests[index].file, "\n");
    _mys_rank_log_G.dests[index].tot_wrote += wrote;
    _mys_rank_log_G.dests[index].cur_wrote += wrote;

_finished:
    mys_mutex_unlock(&_mys_rank_log_G.lock);
}

MYS_API void mys_rank_log_open(const char *callfile, int callline, const char *folder)
{
    _mys_rank_log_init();
    size_t len = strnlen(folder, _MYS_FNAME_MAX);
    int index = _mys_rank_log_find_dest(folder, len);
    mys_mutex_lock(&_mys_rank_log_G.lock);
    if (index != -1 && _mys_rank_log_G.dests[index].file != NULL) {
        mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, callfile, callline, "Reopen an activating rank log folder: %s", folder);
        goto _finished;
    }

    if (_mys_rank_log_G.max_used == _MYS_RANK_LOG_DEST_NUM) {
        mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, callfile, callline, "Cannot open %s because too many folder opened (%d)", folder, _mys_rank_log_G.max_used);
        goto _finished;
    }
    char name[4096];
    snprintf(name, sizeof(name), "%s/%06d.log", folder, mys_mpi_myrank());
    mys_ensure_dir(folder, 0777);
    if (index == -1) {
        // First time. Open with create mode
        FILE *file = fopen(name, "w");
        if (file == NULL) {
            mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, callfile, callline, "Error on opening rank log folder: %s", folder);
            goto _finished;
        }
        index = _mys_rank_log_G.max_used++;
        _mys_rank_log_G.dests[index].file = file;
        _mys_rank_log_G.dests[index].tot_wrote = 0;
        _mys_rank_log_G.dests[index].cur_wrote = 0;
        strncpy(_mys_rank_log_G.dests[index].folder, folder, len);
    } else { // _mys_rank_log_G.dests[index].file == NULL
        // Open with append mode
        FILE *file = fopen(name, "a");
        if (file == NULL) {
            mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, callfile, callline, "Error on reopening rank log folder: %s", folder);
            goto _finished;
        }
        _mys_rank_log_G.dests[index].file = file;
        _mys_rank_log_G.dests[index].cur_wrote = 0;
    }

    mys_log(0, MYS_LOG_INFO, callfile, callline, "Opened rank log folder: %s", folder);
    mys_mpi_barrier();
_finished:
    mys_mutex_unlock(&_mys_rank_log_G.lock);
}

MYS_API void mys_rank_log_close(const char *callfile, int callline, const char *folder)
{
    _mys_rank_log_init();
    size_t len = strnlen(folder, _MYS_FNAME_MAX);
    int index = _mys_rank_log_find_dest(folder, len);
    mys_mutex_lock(&_mys_rank_log_G.lock);
    size_t tot_wrote = _mys_rank_log_G.dests[index].tot_wrote;
    size_t cur_wrote = _mys_rank_log_G.dests[index].cur_wrote;

    if (index == -1) {
        mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, callfile, callline, "Can not close invalid folder: %s", folder);
        goto _finished;
    }
    if (_mys_rank_log_G.dests[index].file == NULL) {
        mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, callfile, callline, "Can not close closed folder: %s", folder);
        goto _finished;
    }
    fclose(_mys_rank_log_G.dests[index].file);
    _mys_rank_log_G.dests[index].file = NULL;

    mys_log(0, MYS_LOG_INFO, callfile, callline, "Closed rank log folder: %s (bytes wrote %zu, total wrote %zu)", folder, cur_wrote, tot_wrote);
    mys_mpi_barrier();
_finished:
    mys_mutex_unlock(&_mys_rank_log_G.lock);
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


#define _MYS_CLOSE_FD(fd) do { if (fcntl(fd, F_GETFL) != -1 || errno != EBADF) close(fd); } while (0)

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
        _MYS_CLOSE_FD(in[0]);
        _MYS_CLOSE_FD(out[1]);
        _MYS_CLOSE_FD(err[1]);
        *ipipe = in[1];
        *opipe = out[0];
        *epipe = err[0];
        return pid;
    } else if (pid == 0) { /* child */
        _MYS_CLOSE_FD(in[1]);
        _MYS_CLOSE_FD(out[0]);
        _MYS_CLOSE_FD(err[0]);
        _MYS_CLOSE_FD(0);
        dup(in[0]);
        _MYS_CLOSE_FD(1);
        dup(out[1]);
        _MYS_CLOSE_FD(2);
        dup(err[1]);
        execl( "/bin/sh", "sh", "-c", command, NULL );
        _exit(1);
    } else
        goto error_fork;

    return pid;

error_fork:
    _MYS_CLOSE_FD(err[0]);
    _MYS_CLOSE_FD(err[1]);
error_err:
    _MYS_CLOSE_FD(out[0]);
    _MYS_CLOSE_FD(out[1]);
error_out:
    _MYS_CLOSE_FD(in[0]);
    _MYS_CLOSE_FD(in[1]);
error_in:
    return -1;
}

static int _mys_pclose_rwe(int pid, int ipipe, int opipe, int epipe)
{
    _MYS_CLOSE_FD(ipipe);
    _MYS_CLOSE_FD(opipe);
    _MYS_CLOSE_FD(epipe);
    int status = -1;
    if (waitpid(pid, &status, 0) == pid)
        status = WEXITSTATUS(status);
    else
        status = -1;
    return status;
}

#undef _MYS_CLOSE_FD

MYS_API mys_popen_t mys_popen_create(const char *argv)
{
    mys_popen_t result;
    result.ifd = -1;
    result.ofd = -1;
    result.efd = -1;
    result.pid = _mys_popen_rwe(&result.ifd, &result.ofd, &result.efd, argv);
    return result;
}

MYS_API int mys_popen_destroy(mys_popen_t *p)
{
    if (p == NULL)
        return 0;
    int result = _mys_pclose_rwe(p->pid, p->ifd, p->ofd, p->efd);
    p->pid = -1;
    p->ifd = -1;
    p->ofd = -1;
    p->efd = -1;
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


// FIXME: Align to mys_prun_create_s. Like removing suffix \\n
MYS_API mys_prun_t mys_prun_create(const char *argv)
{
    mys_prun_t result;
    result.retval = -1;
    result.out = NULL;
    result.err = NULL;
    result._by_safe = false;

    mys_popen_t pd = mys_popen_create(argv);

    FILE *outfd = fdopen(pd.ofd, "r");
    if (outfd) {
        result.len_out = _mys_readfd(&result.out, outfd);
        printf("alaala %zd %s\n", result.len_out, argv);
        fclose(outfd);
    } else {
        result.out = (char *)malloc(0);
        result.len_out = 0;
    }
    FILE *errfd = fdopen(pd.efd, "r");
    if (errfd) {
        result.len_err = _mys_readfd(&result.err, errfd);
        fclose(errfd);
    } else {
        result.err = (char *)malloc(0);
        result.len_err = 0;
    }
    result.retval = mys_popen_destroy(&pd);
    return result;
}

MYS_API mys_prun_t mys_prun_create_s(const char *argv, char *buf_out, size_t size_out, char *buf_err, size_t size_err)
{
    mys_prun_t result;
    result.out = NULL;
    result.err = NULL;
    result.len_out = 0;
    result.len_err = 0;
    result._by_safe = true;
    mys_popen_t pd = mys_popen_create(argv);

    // Read data from the file descriptor
    if (buf_out != NULL) {
        while (size_out - result.len_out > 0) {
            ssize_t ret = read(pd.ofd, buf_out, size_out - result.len_out);
            if (ret <= 0) {
                break; // cannot read more or no more message
            } else {
                result.len_out += (size_t)ret;
            }
        }
        while (result.len_out > 0 && buf_out[result.len_out - 1] == '\n')
            result.len_out -= 1;
        buf_out[result.len_out] = '\0';
    }

    if (buf_err != NULL) {
        while (size_err - result.len_err > 0) {
            ssize_t ret = read(pd.efd, buf_err, size_err - result.len_err);
            if (ret <= 0) {
                break; // cannot read more or no more message
            } else {
                result.len_err += (size_t)ret;
            }
        }
        while (result.len_err > 0 && buf_err[result.len_err - 1] == '\n')
            result.len_err -= 1;
        buf_err[result.len_err] = '\0';
    }

    result.retval = mys_popen_destroy(&pd);
    return result;
}

MYS_API void mys_prun_destroy(mys_prun_t *p)
{
    if (p == NULL)
        return;
    if (p->_by_safe == false) {
        if (p->out != NULL) free((char *)p->out);
        if (p->err != NULL) free((char *)p->err);
    }
    p->out = NULL;
    p->err = NULL;
    p->len_out = 0;
    p->len_err = 0;
    p->retval = -1;
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
#ifdef __linux__
    static char exe[128] = { '\0' };
    if (exe[0] == '\0') {
        int ret = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
        if (ret > 0) {
            exe[ret] = '\0';
        } else {
            char *reason = strerror(errno);
            int pid = (int)getpid();
            snprintf(exe, sizeof(exe), "<error_exe.pid=%d (%s)>", pid, reason);
        }
    }
    return exe;
#elif __APPLE__
    const char *getprogname(void);
    return getprogname();
#endif
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

MYS_API void mys_aggregate_analysis_array(size_t n, double *values, mys_aggregate_t *results)
{
    mys_mpi_init();
    int myrank = mys_mpi_myrank();
    int nranks = mys_mpi_nranks();
    struct di_t { double d; int i; };
    struct di_t *dibuf = (struct di_t *)malloc(sizeof(struct di_t) * n);
    double *dbuf = (double *)((void *)dibuf);

    {// mine, sum, avg
        _mys_MPI_Allreduce(values, dbuf, n, _mys_MPI_DOUBLE, _mys_MPI_SUM, _mys_mpi_G.comm);
        for (size_t i = 0; i < n; i++) {
            results[i].self = values[i];
            results[i].sum = dbuf[i];
            results[i].avg = dbuf[i] / (double)nranks;
        }
    }
    {// var and std
        for (size_t i = 0; i < n; i++) {
            dbuf[i] = (values[i] - results[i].avg) * (values[i] - results[i].avg);
        }
        _mys_MPI_Allreduce(_mys_MPI_IN_PLACE, dbuf, n, _mys_MPI_DOUBLE, _mys_MPI_SUM, _mys_mpi_G.comm);
        for (size_t i = 0; i < n; i++) {
            results[i].var = dbuf[i] / (double)nranks;
            results[i].std = _mys_math_sqrt(results[i].var);
        }
    }
    {// max and min
        for (size_t i = 0; i < n; i++) {
            dibuf[i].d = values[i];
            dibuf[i].i = myrank;
        }
        _mys_MPI_Allreduce(_mys_MPI_IN_PLACE, dibuf, n, _mys_MPI_DOUBLE_INT, _mys_MPI_MAXLOC, _mys_mpi_G.comm);
        for (size_t i = 0; i < n; i++) {
            results[i].max = dibuf[i].d;
            results[i].loc_max = dibuf[i].i;
        }

        for (size_t i = 0; i < n; i++) {
            dibuf[i].d = values[i];
            dibuf[i].i = myrank;
        }
        _mys_MPI_Allreduce(_mys_MPI_IN_PLACE, dibuf, n, _mys_MPI_DOUBLE_INT, _mys_MPI_MINLOC, _mys_mpi_G.comm);
        for (size_t i = 0; i < n; i++) {
            results[i].min = dibuf[i].d;
            results[i].loc_min = dibuf[i].i;
        }
    }
    free(dibuf);
}

MYS_API mys_aggregate_t mys_aggregate_analysis(double value)
{
    mys_aggregate_t result;
    mys_aggregate_analysis_array(1, &value, &result);
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

#ifdef _OPENMP
#include <omp.h>
#else
mys_thread_local int __mys_thread_id = -1;
static int __mys_thread_count = 0;
#endif

MYS_API int mys_thread_id()
{
#ifdef _OPENMP
    return omp_get_thread_num();
#else
    if (__mys_thread_id == -1) {
        __mys_thread_id = mys_atomic_fetch_add(&__mys_thread_count, 1, MYS_ATOMIC_RELAXED);
    }
    return __mys_thread_id;
#endif
}


#ifdef MYS_USE_POSIX_MUTEX
MYS_API int mys_mutex_init(mys_mutex_t *lock)
{
    return pthread_mutex_init(lock, NULL);
}

MYS_API int mys_mutex_destroy(mys_mutex_t *lock)
{
    return pthread_mutex_destroy(lock);
}

MYS_API int mys_mutex_lock(mys_mutex_t *lock)
{
    return pthread_mutex_lock(lock);
}

MYS_API int mys_mutex_unlock(mys_mutex_t *lock)
{
    return pthread_mutex_unlock(lock);
}
#else
MYS_API int mys_mutex_init(mys_mutex_t *lock)
{
    mys_atomic_store_n(&lock->tid, __MYS_MUTEX_IDLE, MYS_ATOMIC_RELAXED);
    return 0;
}

MYS_API int mys_mutex_destroy(mys_mutex_t *lock)
{
    int oval = __MYS_MUTEX_IDLE;
    int nval = __MYS_MUTEX_INVALID;
    bool ok = mys_atomic_compare_exchange(&lock->tid, &oval, &nval, MYS_ATOMIC_RELAXED, MYS_ATOMIC_RELAXED);
    return (ok == true) ? 0 : (oval == __MYS_MUTEX_INVALID) ? EINVAL : EBUSY;
}

MYS_API int mys_mutex_lock(mys_mutex_t *lock)
{
    int tid = mys_thread_id();
    int oval = __MYS_MUTEX_IDLE;
    int nval = tid;
    while (!mys_atomic_compare_exchange(&lock->tid, &oval, &nval, MYS_ATOMIC_ACQUIRE, MYS_ATOMIC_RELAXED)) {
        if (oval == tid)
            return EDEADLK;
        if (oval == __MYS_MUTEX_INVALID)
            return EINVAL;
        oval = __MYS_MUTEX_IDLE;
    }
    return 0;
}

MYS_API int mys_mutex_unlock(mys_mutex_t *lock)
{
    int tid = mys_thread_id();
    int oval = tid;
    int nval = __MYS_MUTEX_IDLE;
    while (!mys_atomic_compare_exchange(&lock->tid, &oval, &nval, MYS_ATOMIC_RELEASE, MYS_ATOMIC_RELAXED)) {
        if (oval != tid)
            return EPERM;
        if (oval == __MYS_MUTEX_INVALID)
            return EINVAL;
        if (oval == __MYS_MUTEX_IDLE)
            return 0;
        oval = tid;
    }
    return 0;
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

#if defined(OS_LINUX) && defined(MYS_ENABLE_SHM)
struct _mys_shm_G_t {
    bool inited;
    mys_mutex_t lock;
    int program_id;
    size_t counter;
};

static struct _mys_shm_G_t _mys_shm_G = {
    .inited = false,
    .lock = MYS_MUTEX_INITIALIZER,
    .program_id = 0,
    .counter = 0,
};

static void _mys_shm_G_init()
{
    if (_mys_shm_G.inited == true)
        return;
    mys_mutex_lock(&_mys_shm_G.lock);
    {
        if (mys_mpi_myrank() == 0)
            _mys_shm_G.program_id = getpid();
        _mys_MPI_Bcast(&_mys_shm_G.program_id, 1, _mys_MPI_INT, 0, _mys_MPI_COMM_WORLD);
    }
    _mys_shm_G.inited = true;
    mys_mutex_unlock(&_mys_shm_G.lock);
}

MYS_API mys_shm_t mys_alloc_shared_memory(int owner_rank, size_t size)
{
    _mys_shm_G_init();
    mys_mutex_lock(&_mys_shm_G.lock);
    mys_shm_t shm;
    snprintf(shm._name, sizeof(shm._name), "/mys_%d_%zu", _mys_shm_G.program_id, _mys_shm_G.counter);
    _mys_shm_G.counter += 1;
    if (mys_mpi_myrank() == owner_rank) {
        shm._fd = shm_open(shm._name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        shm._size = size;
        ftruncate(shm._fd, shm._size);
        shm.mem = mmap(NULL, shm._size, PROT_READ | PROT_WRITE, MAP_SHARED, shm._fd, 0);
        memset(shm.mem, 0, shm._size);
        mys_memory_smp_mb();
        _mys_MPI_Barrier(_mys_MPI_COMM_WORLD);
    } else {
        _mys_MPI_Barrier(_mys_MPI_COMM_WORLD);
        shm._fd = shm_open(shm._name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        shm.mem = mmap(NULL, shm._size, PROT_READ | PROT_WRITE, MAP_SHARED, shm._fd, 0);
    }
    mys_mutex_unlock(&_mys_shm_G.lock);
    return shm;
}

MYS_API void mys_free_shared_memory(mys_shm_t *shm)
{
    _mys_shm_G_init();
    mys_mutex_lock(&_mys_shm_G.lock);
    {
        munmap(shm->mem, shm->_size);
        close(shm->_fd);
        shm_unlink(shm->_name);
        shm->mem = NULL;
    }
    mys_mutex_unlock(&_mys_shm_G.lock);
}

#endif

MYS_API mys_bits_t mys_bits(const void *data, size_t size)
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

MYS_API void mys_cache_flush(size_t nbytes)
{
    char * volatile arr = (char *)malloc(nbytes * sizeof(char));
    memset(arr, 0, nbytes);
    for (size_t i = 1; i < nbytes; i++) {
        arr[i] = i | (arr[i - 1]);
    }
    volatile char result;
    result = arr[nbytes - 1];
    result = (char)(uint64_t)(&arr[(uint64_t)result]);
    mys_atomic_fence(MYS_ATOMIC_RELEASE);
    free(arr);
}

MYS_API ssize_t mys_parse_readable_size(const char *text)
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

MYS_API void mys_readable_size(char **ptr, size_t bytes, size_t precision)
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

// https://troydhanson.github.io/uthash/utlist.html
typedef struct _mys_guard_record_t {
    struct _mys_guard_record_t *prev; /* needed for a doubly-linked list only */
    struct _mys_guard_record_t *next; /* needed for singly- or doubly-linked lists */
    uint8_t data[0];
} _mys_guard_record_t;

MYS_STATIC _mys_guard_record_t *_mys_guard_record_create(void *data, size_t size)
{
    _mys_guard_record_t *record = (_mys_guard_record_t *)malloc(sizeof(_mys_guard_record_t) + size);
    record->prev = NULL;
    record->next = NULL;
    memcpy(record->data, data, size);
    return record;
}

MYS_STATIC void _mys_guard_record_destroy(_mys_guard_record_t *record)
{
    free(record);
}

MYS_STATIC _mys_guard_record_t *_mys_guard_records_append(_mys_guard_record_t **head, void *data, size_t size)
{
    _mys_guard_record_t *record = (_mys_guard_record_t *)malloc(sizeof(_mys_guard_record_t) + size);
    record->prev = NULL;
    record->next = NULL;
    memcpy(record->data, data, size);
    _DL_APPEND(*head, record);
    return record;
}

MYS_STATIC int _mys_guard_records_remove(_mys_guard_record_t **head, void *data, size_t size)
{
    _mys_guard_record_t *elt, *tmp;
    int rc = 1;
    _DL_FOREACH_SAFE(*head, elt, tmp) {
        if (memcmp(data, elt->data, size) == 0) {
            _DL_DELETE(*head, elt);
            _mys_guard_record_destroy(elt);
            rc = 0;
        }
    }
    return rc;
}

MYS_STATIC void _mys_guard_records_clear(_mys_guard_record_t **head)
{
    _mys_guard_record_t *elt, *tmp;
    /* now delete each element, use the safe iterator */
    _DL_FOREACH_SAFE(*head, elt, tmp) {
        _DL_DELETE(*head, elt);
        _mys_guard_record_destroy(elt);
    }
}

// https://troydhanson.github.io/uthash/userguide.html#_string_keys
typedef struct _mys_guard_map_t {
    char *type_name;
    size_t type_size;
    size_t num_record;
    _mys_guard_record_t *records;
    _mys_UT_hash_handle hh;
} _mys_guard_map_t;

MYS_STATIC _mys_guard_map_t *_mys_guard_map_find(_mys_guard_map_t *node, const char *name)
{
    _mys_guard_map_t *s = NULL;
    _HASH_FIND_STR(node, name, s);
    return s;
}

MYS_STATIC _mys_guard_map_t *_mys_guard_map_insert(_mys_guard_map_t **node, const char *name, size_t size)
{
    _mys_guard_map_t *s = (_mys_guard_map_t *)malloc(sizeof(_mys_guard_map_t));
    size_t len = strnlen(name, 1024);
    s->type_name = strndup(name, len);
    s->type_size = size;
    s->num_record = 0;
    s->records = NULL;
    _HASH_ADD_KEYPTR(hh, *node, s->type_name, len, s);
    return s;
}

MYS_STATIC void _mys_guard_map_remove(_mys_guard_map_t **head, _mys_guard_map_t *node)
{
    _HASH_DEL(*head, node);
    free(node->type_name);
    _mys_guard_records_clear(&node->records);
    node->num_record = 0;
    free(node);
}

struct _mys_guard_G_t {
    _mys_guard_map_t *map;
};

static struct _mys_guard_G_t _mys_guard_G = {
    .map = NULL,
};

MYS_API void mys_guard_begin(const char *type_name, size_t type_size, void *variable_ptr, const char *file, int line)
{
    if (type_name == NULL) {
        mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, file, line, "(INTERNAL ERROR) Invalid type name (nil).");
        exit(1);
    }
    _mys_guard_map_t *type_node = _mys_guard_map_find(_mys_guard_G.map, type_name);
    if (type_node == NULL) {
        type_node = _mys_guard_map_insert(&_mys_guard_G.map, type_name, type_size);
    }
    _mys_guard_record_t *record = _mys_guard_records_append(&type_node->records, variable_ptr, type_size);
    if (record == NULL) {
        mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, file, line, "(INTERNAL ERROR) Cannot acquire type guard (%s).", type_name);
        exit(1);
    }
    type_node->num_record += 1;
}

MYS_API void mys_guard_end(const char *type_name, size_t type_size, void *variable_ptr, const char *file, int line)
{
    if (type_name == NULL) {
        mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, file, line, "(INTERNAL ERROR) Invalid type name (nil).");
        exit(1);
    }
    _mys_guard_map_t *type_node = _mys_guard_map_find(_mys_guard_G.map, type_name);
    if (type_node == NULL) {
        mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, file, line, "(INTERNAL ERROR) Releasing type guard (%s) that doesn't exist.", type_name);
        exit(1);
    }
    if (_mys_guard_records_remove(&type_node->records, variable_ptr, type_size)) {
        mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, file, line, "Releasing type guard (%s) that didn't begin.", type_name);
        exit(1);
    }
    type_node->num_record -= 1;
    if (type_node->num_record == 0) {
        _mys_guard_map_remove(&_mys_guard_G.map, type_node);
    }
}


// https://stackoverflow.com/a/466242/11702338
static size_t _mys_round_ceil_2_power(size_t num)
{
    --num;
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
#if SIZE_MAX == UINT64_MAX
    num |= num >> 32;
#endif
    return ++num;
}

MYS_API mys_string_t mys_string_create()
{
    mys_string_t str = (mys_string_t)malloc(sizeof(struct mys_string_struct));
    str->text = NULL;
    str->size = 0;
    str->capacity = 0;
    return str;
}

MYS_API void mys_string_destroy(mys_string_t str)
{
    if (str != NULL) {
        if (str->text != NULL)
            free(str->text);
        free(str);
    }
}

MYS_API void mys_string_fmt(mys_string_t str, const char *format, ...)
{
    va_list vargs, vargs_copy;
    va_start(vargs, format);

    va_copy(vargs_copy, vargs);
    int needed = vsnprintf(NULL, 0, format, vargs_copy) + 1; // Extra space for '\0'
    va_end(vargs_copy);
    if (needed <= 0)
        goto finish;

    if (str->capacity - str->size < (size_t)needed) {
        size_t new_capacity = _mys_round_ceil_2_power(str->capacity + (size_t)needed);
        char *new_text = (char *)realloc(str->text, new_capacity);
        if (new_text == NULL)
            goto finish;
        str->text = new_text;
        str->capacity = new_capacity;
    }
    str->size += vsnprintf(str->text + str->size, str->capacity - str->size, format, vargs);
finish:
    va_end(vargs);
}


#ifndef MYS_NO_MPI
static int _commgroup_rank_sortfn(const void* a, const void* b)
{
    const int *intA = (const int *)a;
    const int *intB = (const int *)b;
    if (*intA < *intB) return -1;
    else if (*intA > *intB) return 1;
    else return 0;
}

MYS_API mys_commgroup_t mys_commgroup_create(MPI_Comm global_comm, int group_color, int group_key)
{
    struct _mys_commgroup_t *group = (struct _mys_commgroup_t *)malloc(
        sizeof(struct _mys_commgroup_t)
    );
    MPI_Comm_dup(global_comm, &group->global_comm);
    MPI_Comm_size(group->global_comm, &group->global_nranks);
    MPI_Comm_rank(group->global_comm, &group->global_myrank);

    MPI_Comm_split(group->global_comm, group_color, group_key, &group->local_comm);
    MPI_Comm_size(group->local_comm, &group->local_nranks);
    MPI_Comm_rank(group->local_comm, &group->local_myrank);

    group->group_num = 0;
    group->group_id = -1;
    int im_group_root = group->local_myrank == 0;
    MPI_Allreduce(&im_group_root, &group->group_num, 1, MPI_INT, MPI_SUM, global_comm);
    int *roots = (int *)malloc(sizeof(int) * group->group_num);
    int nrequests = (group->global_myrank == 0) ? (group->group_num + 1) : (group->local_myrank == 0) ? 1 : 0;
    MPI_Request *requests = (MPI_Request *)malloc(sizeof(MPI_Request) * nrequests);

    if (group->global_myrank == 0) {
        for(int i = 0; i < group->group_num; i++)
            MPI_Irecv(&roots[i], 1, MPI_INT, MPI_ANY_SOURCE, 17749, global_comm, &requests[1 + i]);
    }
    if (group->local_myrank == 0) {
        MPI_Isend(&group->global_myrank, 1, MPI_INT, 0, 17749, global_comm, &requests[0]);
    }
    if (nrequests > 0) {
        MPI_Waitall(nrequests, requests, MPI_STATUSES_IGNORE);
    }

    if (group->global_myrank == 0) {
        qsort(roots, group->group_num, sizeof(int), _commgroup_rank_sortfn);
        for(int i = 0; i < group->group_num; i++) {
            int rank = roots[i];
            roots[i] = i;
            MPI_Isend(&roots[i], 1, MPI_INT, rank, 18864, global_comm, &requests[1 + i]);
        }
    }
    if (group->local_myrank == 0) {
        MPI_Irecv(&group->group_id, 1, MPI_INT, 0, 18864, global_comm, &requests[0]);
    }
    if (nrequests > 0) {
        MPI_Waitall(nrequests, requests, MPI_STATUSES_IGNORE);
    }

    MPI_Bcast(&group->group_id, 1, MPI_INT, 0, group->local_comm);

    group->_rows = (int *)malloc(sizeof(int) * group->global_nranks);
    group->_rows[group->global_myrank] = group->group_id;
    MPI_Allgather(MPI_IN_PLACE, 1, MPI_INT, group->_rows, 1, MPI_INT, group->global_comm);

    group->_cols = (int *)malloc(sizeof(int) * group->global_nranks);
    group->_cols[group->global_myrank] = group->local_myrank;
    MPI_Allgather(MPI_IN_PLACE, 1, MPI_INT, group->_cols, 1, MPI_INT, group->global_comm);

    group->_brothers = (int *)malloc(sizeof(int) * group->local_nranks);
    group->_brothers[group->local_myrank] = group->global_myrank;
    MPI_Allgather(MPI_IN_PLACE, 1, MPI_INT, group->_brothers, 1, MPI_INT, group->local_comm);

    MPI_Comm_split(group->global_comm, group->local_myrank, group->group_id, &group->inter_comm);
    group->_neighbors = (int *)malloc(sizeof(int) * group->group_num);
    for (int i = 0; i < group->group_num; i++)
        group->_neighbors[i] = -1;
    group->_neighbors[group->group_id] = group->global_myrank;
    MPI_Allgather(MPI_IN_PLACE, 1, MPI_INT, group->_neighbors, 1, MPI_INT, group->inter_comm);

    free(requests);
    free(roots);
    return (mys_commgroup_t)group;
}

MYS_API void mys_commgroup_release(mys_commgroup_t group)
{
    assert(group != NULL);
    if (group == MYS_COMMGROUP_NULL) return;
    MPI_Comm_free(&group->global_comm);
    MPI_Comm_free(&group->local_comm);
    MPI_Comm_free(&group->inter_comm);
    free(group->_rows);
    free(group->_cols);
    free(group->_brothers);
    free(group->_neighbors);
}

MYS_API mys_commgroup_t mys_commgroup_create_node(MPI_Comm global_comm)
{
    int global_nranks, global_myrank;
    MPI_Comm_size(global_comm, &global_nranks);
    MPI_Comm_rank(global_comm, &global_myrank);

    MPI_Comm local_comm = MPI_COMM_NULL;
    int node_root = global_myrank;
    MPI_Comm_split_type(global_comm, MPI_COMM_TYPE_SHARED, global_myrank, MPI_INFO_NULL, &local_comm);
    MPI_Bcast(&node_root, 1, MPI_INT, 0, local_comm);
    MPI_Comm_free(&local_comm);

    return mys_commgroup_create(global_comm, node_root, global_myrank);
}

MYS_API int mys_query_group_id(mys_commgroup_t group, int global_rank)
{
    if (global_rank < 0 || global_rank >= group->global_nranks)
        return -1;
    else
        return group->_rows[global_rank];
}

MYS_API int mys_query_local_rank(mys_commgroup_t group, int global_rank)
{
    if (global_rank < 0 || global_rank >= group->global_nranks)
        return -1;
    else
        return group->_cols[global_rank];
}

MYS_API int mys_query_brother(mys_commgroup_t group, int local_rank)
{
    if (local_rank < 0 || local_rank >= group->local_nranks)
        return -1;
    else
        return group->_brothers[local_rank];
}

MYS_API int mys_query_neighbor(mys_commgroup_t group, int group_id)
{
    if (group_id < 0 || group_id >= group->group_num)
        return -1;
    else
        return group->_neighbors[group_id];
}
#endif

// TODO: Use malloc to alloc a large preserved memory on init, instead of large static array. All memory you use should come from there

#define _STRIP_DEPTH 3 // _mys_debug_print, _mys_debug_handler, +1 stack base
#define _HANDLE_MAX 32 // maximum signal to handle
#define _BACKTRACE_MAX 32ULL // maximum stack backtrace
#define _LOG_BUF_SIZE 65536ULL // for holding the entire backtrace
#define _MSG_BUF_SIZE 256ULL // for holding small message
#define _MSG_BUF_NUM  3 // one for addr2line bufcmd, one for addr2line stdout, one for print signal message
#define _FAULT_TOLERANT_SIZE (128ULL * 1024) // for holding small message
#define _STACK_SIZE ( \
    (SIGSTKSZ) + \
    (_BACKTRACE_MAX * sizeof(void*)) + \
    (_LOG_BUF_SIZE) + \
    (_MSG_BUF_SIZE * _MSG_BUF_NUM) + \
    (_FAULT_TOLERANT_SIZE) \
)

#define POST_ACTION_NONE     (0U) // no post action
#define POST_ACTION_EXIT     (1U) // directly exit program
#define POST_ACTION_RAISE    (2U) // re-rase signo to old handler
#define POST_ACTION_FREEZE   (3U) // freeze program by while(1) loop

mys_thread_local char _mys_debug_last_message[MYS_SIGNAL_LAST_MESSAGE_MAX] = { '\0' };
struct _mys_debug_G_t {
    mys_mutex_t lock;
    bool inited;
    int outfd;
    int outfd_isatty;
    int signals[_HANDLE_MAX];
    struct sigaction old_actions[_HANDLE_MAX];
    uint32_t post_action;
    stack_t stack;
    uint8_t stack_memory[_STACK_SIZE];
};

static struct _mys_debug_G_t _mys_debug_G = {
    .lock = MYS_MUTEX_INITIALIZER,
    .inited = false,
    .outfd = STDERR_FILENO,
    .outfd_isatty = 0,
    .signals = { SIGINT, SIGILL, SIGTRAP, SIGBUS, SIGFPE, SIGSEGV, SIGTERM, SIGCHLD, SIGABRT, 0 }, // 0 to stop
    .old_actions = {},
    .post_action = POST_ACTION_EXIT,
    .stack = { NULL, 0, 0 },
    .stack_memory = { '\0' },
};

static const char *_mys_sigcause(int signo, int sigcode)
{
    if (signo == SIGILL) {
        switch (sigcode) {
            case ILL_ILLOPC : return "illegal opcode";
            case ILL_ILLOPN : return "illegal operand";
            case ILL_ILLADR : return "illegal addressing mode";
            case ILL_ILLTRP : return "illegal trap";
            case ILL_PRVOPC : return "privileged opcode";
            case ILL_PRVREG : return "privileged register";
            case ILL_COPROC : return "coprocessor error";
            case ILL_BADSTK : return "internal stack error";
        }
    } else if (signo == SIGTRAP) {
        switch (sigcode) {
            case TRAP_BRKPT : return "process breakpoint";
            case TRAP_TRACE : return "process trace trap";
        }
    } else if (signo == SIGBUS) {
        switch (sigcode) {
            case BUS_ADRALN : return "invalid address alignment";
            case BUS_ADRERR : return "nonexistent physical address";
            case BUS_OBJERR : return "object-specific hardware error";
        }
    } else if (signo == SIGFPE) {
        switch (sigcode) {
            case FPE_INTDIV : return "integer divide by zero";
            case FPE_INTOVF : return "integer overflow";
            case FPE_FLTDIV : return "floating-point divide by zero";
            case FPE_FLTOVF : return "floating-point overflow";
            case FPE_FLTUND : return "floating-point underflow";
            case FPE_FLTRES : return "floating-point inexact result";
            case FPE_FLTINV : return "floating-point invalid operation";
            case FPE_FLTSUB : return "subscript bufout of range";
        }
    } else if (signo == SIGSEGV) {
        switch (sigcode) {
            case SEGV_MAPERR : return "address not mapped to object";
            case SEGV_ACCERR : return "invalid permissions for mapped object";
        }
    } else if (signo == SIGCHLD) {
        switch (sigcode) {
            case CLD_EXITED    : return "child has exited";
            case CLD_KILLED    : return "child was killed";
            case CLD_DUMPED    : return "child terminated abnormally";
            case CLD_TRAPPED   : return "traced child has trapped";
            case CLD_STOPPED   : return "child has stopped";
            case CLD_CONTINUED : return "stopped child has continued";
        }
    } else { // common
        switch (sigcode) {
            case SI_USER    : return "sent by kill(2) or raise(3)";
#ifdef __linux__
            case SI_KERNEL  : return "sent by kernel";
#endif
            case SI_QUEUE   : return "sent by sigqueue(2)";
            case SI_TIMER   : return "sent by POSIX timer expiration";
            case SI_MESGQ   : return "sent by POSIX message queue state change";
            case SI_ASYNCIO : return "sent by AIO completion";
#ifdef SI_SIGIO
            case SI_SIGIO   : return "sent by queued SIGIO";
#endif
#ifdef SI_TKILL
            case SI_TKILL   : return "sent by tkill(2) or tgkill(2)";
#endif
        }
    }
    return "<unknown reason>";
}

static void _mys_debug_revert()
{
    mys_mutex_lock(&_mys_debug_G.lock);
    {
        char msg[256];
        for (int i = 0; i < _HANDLE_MAX; ++i) {
            int sig = _mys_debug_G.signals[i];
            if (sig == 0)
                break;
            struct sigaction action;
            int ret = sigaction(sig, &_mys_debug_G.old_actions[i], &action);
            if (ret != 0) {
                snprintf(msg, sizeof(msg), "failed to revert signal handler for sig %d : %s\n",
                    sig, strerror(errno));
                write(_mys_debug_G.outfd, msg, strnlen(msg, sizeof(msg)));
            }
        }
    }
    mys_mutex_unlock(&_mys_debug_G.lock);
}

__attribute__((noinline))
static void _mys_debug_print(int signo, const char *fmt, ...)
{
    char cause[_MSG_BUF_SIZE];
    char buflog[_LOG_BUF_SIZE];
    char bufcmd[_MSG_BUF_SIZE];
    char bufout[_MSG_BUF_SIZE];
    void *baddrs[_BACKTRACE_MAX];
    int myrank = mys_mpi_myrank();
    int nranks = mys_mpi_nranks();
    int digits = _mys_math_trunc(_mys_math_log10(nranks)) + 1;
    digits = digits > 3 ? digits : 3;
    size_t lenlog = 0;

    {
        va_list args;
        va_start(args, fmt);
        vsnprintf(cause, sizeof(cause), fmt, args);
        va_end(args);
    }
#define _DOFMT(f, ...) do {                                                             \
    if (lenlog < sizeof(buflog))                                                        \
        lenlog += snprintf(buflog + lenlog, sizeof(buflog) - lenlog, f, ##__VA_ARGS__); \
} while (0)
#define _NFMT1 "[F::%0*d CRASH] -------------------------------\n"
#define _NFMT2 "[F::%0*d CRASH] | Caught signal %d. %s: %s\n"
#define _NFMT3 "[F::%0*d CRASH] | %s\n"
#define _NFMT4 "[F::%0*d CRASH] | No backtrace stack available\n"
#define _NFMT5 "[F::%0*d CRASH] -------------------------------\n"
#define _YFMT1 MCOLOR_RED "[F::%0*d CRASH] -------------------------------\n"
#define _YFMT2 "[F::%0*d CRASH] |" MCOLOR_BOLD " Caught signal %d. %s: %s " MCOLOR_NO MCOLOR_RED "\n"
#define _YFMT3 "[F::%0*d CRASH] |" MCOLOR_BOLD " %s " MCOLOR_NO MCOLOR_RED "\n"
#define _YFMT4 "[F::%0*d CRASH] |" MCOLOR_BOLD " No backtrace stack available " MCOLOR_NO MCOLOR_RED "\n"
#define _YFMT5 "[F::%0*d CRASH] -------------------------------" MCOLOR_NO "\n"
    {
        const char *self_exe = mys_procname();
        int bsize = backtrace(baddrs, _BACKTRACE_MAX);
        char **bsyms = backtrace_symbols(baddrs, bsize);

        _DOFMT(_mys_debug_G.outfd_isatty ? _YFMT1 : _NFMT1, digits, myrank);
        _DOFMT(_mys_debug_G.outfd_isatty ? _YFMT2 : _NFMT2, digits, myrank, signo, strsignal(signo), cause);
        if (_mys_debug_last_message[0] != '\0')
            _DOFMT(_mys_debug_G.outfd_isatty ? _YFMT3 : _NFMT3, digits, myrank, _mys_debug_last_message);
        if (bsize == 0)
            _DOFMT(_mys_debug_G.outfd_isatty ? _YFMT4 : _NFMT4, digits, myrank);

        for (int i = _STRIP_DEPTH; i < bsize; ++i) {
            snprintf(bufcmd, sizeof(bufcmd), "addr2line -e %s %p", self_exe, baddrs[i]);
            mys_prun_t run = mys_prun_create_s(bufcmd, bufout, sizeof(bufout), NULL, 0);
            _DOFMT("[F::%0*d CRASH] | %d   %s at %s\n",
                digits, myrank, i - _STRIP_DEPTH, bsyms[i], bufout);
            mys_prun_destroy(&run);
        }
        free(bsyms);
        _DOFMT(_mys_debug_G.outfd_isatty ? _YFMT5 : _NFMT5, digits, myrank);
    }
#undef _DOFMT
#undef _NFMT1
#undef _NFMT2
#undef _NFMT3
#undef _NFMT4
#undef _NFMT5
#undef _YFMT1
#undef _YFMT2
#undef _YFMT3
#undef _YFMT4
#undef _YFMT5
    write(_mys_debug_G.outfd, buflog, lenlog);
}

__attribute__((noinline))
static void _mys_debug_handler(int signo, siginfo_t *info, void *context)
{
    (void)context;
    uint32_t post_action;
    mys_mutex_lock(&_mys_debug_G.lock);
    {
        post_action = _mys_debug_G.post_action;
    }
    mys_mutex_unlock(&_mys_debug_G.lock);
    _mys_debug_revert(); // let old handler to clean up if our handler crash.

    // Code below shouldn't access _mys_debug_G anymore.
    int sigcode = info->si_code;
    void *addr = info->si_addr;
    switch (signo) {
    case SIGILL:
        _mys_debug_print(signo, "%s", _mys_sigcause(signo, sigcode));
        break;
    case SIGTRAP:
        _mys_debug_print(signo, "%s", _mys_sigcause(signo, sigcode));
        break;
    case SIGBUS:
        _mys_debug_print(signo, "%s", _mys_sigcause(signo, sigcode));
        break;
    case SIGFPE:
        _mys_debug_print(signo, "%s", _mys_sigcause(signo, sigcode));
        break;
    case SIGSEGV:
        _mys_debug_print(signo, "%s at %p", _mys_sigcause(signo, sigcode), addr);
        break;
    case SIGCHLD:
        _mys_debug_print(signo, "%s at %p", _mys_sigcause(signo, sigcode), addr);
        break;
    case SIGINT:
    case SIGTERM:
        break;
    default:
        _mys_debug_print(signo, "%s", _mys_sigcause(signo, sigcode));
        break;
    }
    if (post_action == POST_ACTION_EXIT) {
        _exit(signo);
    } else if (post_action == POST_ACTION_RAISE) {
        raise(signo);
    } else if (post_action == POST_ACTION_FREEZE) {
        do {} while (1);
    }
}

MYS_API void mys_debug_init()
{
    // https://stackoverflow.com/a/61860187/11702338
    // Make sure that backtrace(libgcc) is loaded before any signals are generated
    void* dummy = NULL;
    backtrace(&dummy, 1);
    mys_mpi_init();
    memset(_mys_debug_last_message, 0, MYS_SIGNAL_LAST_MESSAGE_MAX);

    mys_mutex_lock(&_mys_debug_G.lock);
    {
        struct sigaction new_action, old_action;
        unsigned int i;
        int ret;

        memset(_mys_debug_G.stack_memory, 0, _STACK_SIZE);
        _mys_debug_G.stack.ss_sp = _mys_debug_G.stack_memory;
        _mys_debug_G.stack.ss_size = _STACK_SIZE;
        _mys_debug_G.stack.ss_flags = 0;
        ret = sigaltstack(&_mys_debug_G.stack, NULL);
        if (ret) {
            printf("sigaltstack failed: %s\n", strerror(errno));
            return;
        }

        new_action.sa_sigaction = _mys_debug_handler;
        new_action.sa_flags = SA_SIGINFO | SA_ONSTACK;
        sigemptyset(&new_action.sa_mask);
        for (i = 0; i < _HANDLE_MAX; ++i) {
            int sig = _mys_debug_G.signals[i];
            if (sig == 0)
                break;
            ret = sigaction(sig, &new_action, &old_action);
            if (ret == 0) {
                _mys_debug_G.old_actions[i] = old_action;
            } else {
                printf("failed to set signal handler for sig %d : %s\n", sig, strerror(errno));
                _mys_debug_G.old_actions[i].sa_sigaction = NULL;
            }
        }

        _mys_debug_G.outfd_isatty = isatty(_mys_debug_G.outfd);
        _mys_debug_G.inited = true;
    }
    mys_mutex_unlock(&_mys_debug_G.lock);
}

MYS_API void mys_debug_fini()
{
    _mys_debug_revert();
    mys_mutex_lock(&_mys_debug_G.lock);
    {
        _mys_debug_G.inited = false;
    }
    mys_mutex_unlock(&_mys_debug_G.lock);
}

MYS_API const char *mys_debug_last_message(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(_mys_debug_last_message, MYS_SIGNAL_LAST_MESSAGE_MAX, fmt, args);
    va_end(args);
    return _mys_debug_last_message;
}


////////////
//////////// End of mys.c
////////////


#define _UTHASH_UNDEF_HASH
#define _UTHASH_UNDEF_LIST
#include "_lib/index.h"

#endif /*__MYS_C__*/
