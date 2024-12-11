/* 
 * Copyright (c) 2024 Haopeng Huang - All Rights Reserved
 * 
 * Licensed under the MIT License. You may use, distribute,
 * and modify this code under the terms of the MIT license.
 * You should have received a copy of the MIT license along
 * with this file. If not, see:
 * 
 * https://opensource.org/licenses/MIT
 */
#include "../_config.h"
#include "../errno.h"
#include "../mpistubs.h"
#include "../log.h"
#include "../memory.h"
#include "uthash_hash.h"

////// Internal

#define MYS_LOG_MAX_HANDLER 128

typedef struct {
    char *key;
    _mys_UT_hash_handle hh;
} _mys_log_once_t;

typedef struct {
    bool inited;
    mys_mutex_t lock;
    int level;
    bool silent;
    _mys_log_once_t *once_map;
    struct {
        mys_log_handler_fn fn;
        void *udata;
        int id;
    } handlers[MYS_LOG_MAX_HANDLER];
} _mys_log_G_t;

static void _mys_log_stdio_handler(mys_log_event_t *event, const char *fmt, va_list vargs, void *udata);

static _mys_log_G_t _mys_log_G = {
    .inited = false,
    .lock = MYS_MUTEX_INITIALIZER,
    .level = MYS_LOG_TRACE,
    .silent = false,
    .once_map = NULL,
    .handlers = {
#ifndef MYS_LOG_DISABLE_STDOUT_HANDLER
        { .fn = _mys_log_stdio_handler, .udata = NULL, .id = 10000 },
#endif
        { .fn = NULL, .udata = NULL, .id = 0 /* Uninitalized ID is 0 */ },
    },
};

MYS_PUBLIC void mys_log_init()
{
    if (_mys_log_G.inited == true)
        return;
    mys_mutex_lock(&_mys_log_G.lock);
    _mys_log_G.inited = true;
    mys_mutex_unlock(&_mys_log_G.lock);
}

MYS_STATIC void _mys_log_impl(int who, int level, const char *file, int line, const char *fmt, va_list vargs)
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
        event.file = (strrchr(file, '/') ? strrchr(file, '/') + 1 : file);
        event.line = line;
        event.no_vargs = false;
        if (fmt == NULL) {
            event.level = MYS_LOG_FATAL;
            fmt = "Calling mys_log with NULL format string. Do you call LOG_SELF(0, \"...\") or LOG(rank, NULL)?";
            event.no_vargs = true;
        }
        mys_log_invoke_handlers(&event, fmt, vargs);
    }
    mys_mutex_unlock(&_mys_log_G.lock);
}

MYS_PUBLIC void mys_log(int who, int level, const char *file, int line, const char *fmt, ...)
{
    va_list vargs;
    va_start(vargs, fmt);
    _mys_log_impl(who, level, file, line, fmt, vargs);
    va_end(vargs);
}

MYS_PUBLIC void mys_log_when(int cond, int level, const char *file, int line, const char *fmt, ...)
{
    if (cond == 0)
        return;
    va_list vargs;
    va_start(vargs, fmt);
    _mys_log_impl(mys_mpi_myrank(), level, file, line, fmt, vargs);
    va_end(vargs);
}

MYS_PUBLIC void mys_log_self(int level, const char *file, int line, const char *fmt, ...)
{
    va_list vargs;
    va_start(vargs, fmt);
    _mys_log_impl(mys_mpi_myrank(), level, file, line, fmt, vargs);
    va_end(vargs);
}

MYS_PUBLIC void mys_log_once(int level, const char *file, int line, const char *fmt, ...)
{
    // log only once at "file:line".
    _mys_log_once_t *entry = NULL, *new_entry = NULL;
    char key[1024];
    snprintf(key, sizeof(key), "%s:%d", file, line);
    mys_mutex_lock(&_mys_log_G.lock);
    {
        _HASH_FIND_STR(_mys_log_G.once_map, key, entry);
    }
    mys_mutex_unlock(&_mys_log_G.lock);

    if (entry != NULL) // already logged
        return;

    new_entry = (_mys_log_once_t *)malloc(sizeof(_mys_log_once_t));
    new_entry->key = strndup(key, sizeof(key));
    mys_mutex_lock(&_mys_log_G.lock);
    {
        _HASH_ADD_STR(_mys_log_G.once_map, key, new_entry);
    }
    mys_mutex_unlock(&_mys_log_G.lock);

    va_list vargs;
    va_start(vargs, fmt);
    _mys_log_impl(mys_mpi_myrank(), level, file, line, fmt, vargs);
    va_end(vargs);
}


MYS_PUBLIC void mys_log_ordered(int level, const char *file, int line, const char *fmt, ...)
{
    mys_log_init();
    mys_mutex_lock(&_mys_log_G.lock);
    if (_mys_log_G.silent == true) {
        mys_mutex_unlock(&_mys_log_G.lock);
        return;
    }
    int myrank = mys_mpi_myrank();
    int nranks = mys_mpi_nranks();
    mys_MPI_Comm comm = mys_mpi_comm();

    const int tag = 65521; /*100000007 OpenMPI 4.1.0 on AArch64 throw invalid tag on large number*/

    if (myrank == 0) {
        mys_log_event_t event;
        event.myrank = myrank;
        event.nranks = nranks;
        event.level = level;
        event.file = (strrchr(file, '/') ? strrchr(file, '/') + 1 : file);
        event.line = line;
        event.no_vargs = false;
        bool broken = false;
        if (fmt == NULL) {
            event.level = MYS_LOG_FATAL;
            fmt = "Calling mys_log with NULL format string. Do you call LOG_SELF(0, \"...\") or LOG(rank, NULL)?";
            event.no_vargs = true;
            broken = true;
        }
        va_list vargs;
        va_start(vargs, fmt);
        mys_log_invoke_handlers(&event, fmt, vargs);
        va_end(vargs);
        char buffer[4096];
        for (int rank = 1; rank < nranks; rank++) {
            mys_MPI_Status status;
            int needed = 0;
            mys_MPI_Probe(rank, tag, comm, &status);
            mys_MPI_Get_count(&status, mys_MPI_CHAR, &needed);
            char *ptr = (needed > 4096) ? (char *)mys_malloc2(mys_arena_log, needed) : buffer;
            mys_MPI_Recv(ptr, needed, mys_MPI_CHAR, rank, tag, comm, mys_MPI_STATUS_IGNORE);

            if (!broken) {
                event.myrank = rank;
                fmt = ptr;
                event.no_vargs = true;
                mys_log_invoke_handlers(&event, fmt, vargs);
            }
            if (ptr != buffer)
                mys_free2(mys_arena_log, ptr, needed);
        }
        mys_MPI_Barrier(comm);
    } else {
        char buffer[4096];
        va_list vargs, vargs_test;
        va_start(vargs, fmt);
        va_copy(vargs_test, vargs);
        int needed = vsnprintf(NULL, 0, fmt, vargs_test) + 1;
        va_end(vargs_test);
        char *ptr = (needed > 4096) ? (char *)mys_malloc2(mys_arena_log, needed) : buffer;
        vsnprintf(ptr, needed, fmt, vargs);
        mys_MPI_Send(ptr, needed, mys_MPI_CHAR, 0, tag, comm);
        if (ptr != buffer)
            mys_free2(mys_arena_log, ptr, needed);
        va_end(vargs);
        mys_MPI_Barrier(comm); // We don't expect logging increase processes' nondeterministic
    }

    mys_mutex_unlock(&_mys_log_G.lock);
}

MYS_PUBLIC int mys_log_add_handler(mys_log_handler_fn handler_fn, void *handler_udata)
{
    mys_log_init();
    mys_mutex_lock(&_mys_log_G.lock);
    int used_max_id = INT32_MIN;
    for (int i = 0; i < MYS_LOG_MAX_HANDLER; i++) {
        if (_mys_log_G.handlers[i].fn == NULL)
            break;
        if (used_max_id < _mys_log_G.handlers[i].id)
            used_max_id = _mys_log_G.handlers[i].id;
    }
    int id = used_max_id + 1;
    for (int i = 0; i < MYS_LOG_MAX_HANDLER; i++) {
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

MYS_PUBLIC void mys_log_remove_handler(int handler_id)
{
    mys_log_init();
    mys_mutex_lock(&_mys_log_G.lock);
    for (int i = 0; i < MYS_LOG_MAX_HANDLER; i++) {
        if (_mys_log_G.handlers[i].id != handler_id)
            continue;
        _mys_log_G.handlers[i].fn = NULL;
        _mys_log_G.handlers[i].udata = NULL;
        _mys_log_G.handlers[i].id = 0;
        for (int j = i + 1; j < MYS_LOG_MAX_HANDLER; j++) {
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

MYS_PUBLIC void mys_log_invoke_handlers(mys_log_event_t *event, const char *fmt, va_list vargs)
{
    for (int i = 0; i < MYS_LOG_MAX_HANDLER; i++) {
        if (_mys_log_G.handlers[i].fn == NULL)
            break;
        va_list vargs_copy;
        va_copy(vargs_copy, vargs);
        _mys_log_G.handlers[i].fn(event, fmt, vargs_copy, _mys_log_G.handlers[i].udata);
        va_end(vargs_copy);
    }
}

MYS_PUBLIC int mys_log_get_level()
{
    mys_log_init();
    mys_mutex_lock(&_mys_log_G.lock);
    int level = _mys_log_G.level;
    mys_mutex_unlock(&_mys_log_G.lock);
    return level;
}

MYS_PUBLIC void mys_log_set_level(int level)
{
    mys_log_init();
    mys_mutex_lock(&_mys_log_G.lock);
    _mys_log_G.level = level;
    mys_mutex_unlock(&_mys_log_G.lock);
}

MYS_PUBLIC void mys_log_silent(bool silent)
{
    mys_log_init();
    mys_mutex_lock(&_mys_log_G.lock);
    _mys_log_G.silent = silent;
    mys_mutex_unlock(&_mys_log_G.lock);
}

MYS_PUBLIC const char* mys_log_level_string(int level)
{
    const char *level_strings[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
    };
    return level_strings[(int)level];
}

static void _mys_log_stdio_handler(mys_log_event_t *event, const char *fmt, va_list vargs, void *udata)
{
    FILE *file = udata != NULL ? (FILE *)udata : stdout;

    char base_label[256] = {'\0'};

    char *label = base_label;
    int label_size = sizeof(base_label);

    int rank_digits = mys_math_trunc(mys_math_log10(event->nranks)) + 1;
    int line_digits = mys_math_trunc(mys_math_log10(event->line)) + 1;
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
        fprintf(file, "%s", fmt);
    } else {
        vfprintf(file, fmt, vargs);
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

static _mys_rank_log_G_t _mys_rank_log_G = {
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

MYS_PUBLIC void mys_rank_log(const char *callfile, int callline, const char *folder, const char *fmt, ...)
{
    _mys_rank_log_init();
    va_list vargs;
    size_t wrote = 0;
    size_t len = strnlen(folder, _MYS_FNAME_MAX);
    int index = _mys_rank_log_find_dest(folder, len);
    mys_mutex_lock(&_mys_rank_log_G.lock);
    callfile = (strrchr(callfile, '/') ? strrchr(callfile, '/') + 1 : callfile);
    if (index == -1) {
        mys_log_self(MYS_LOG_FATAL, callfile, callline, "Invoked mys_rank_log() with invalid folder: %s", folder);
        goto _finished;
    }
    if (_mys_rank_log_G.dests[index].file == NULL) {
        mys_log_self(MYS_LOG_FATAL, callfile, callline, "Invoked mys_rank_log() with closed folder: %s", folder);
        goto _finished;
    }
    if (fmt == NULL) {
        mys_log_self(MYS_LOG_FATAL, callfile, callline, "Invoked mys_rank_log() with NULL format.");
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

MYS_PUBLIC void mys_rank_log_open(const char *callfile, int callline, const char *folder)
{
    _mys_rank_log_init();
    size_t len = strnlen(folder, _MYS_FNAME_MAX);
    int index = _mys_rank_log_find_dest(folder, len);
    mys_mutex_lock(&_mys_rank_log_G.lock);
    callfile = (strrchr(callfile, '/') ? strrchr(callfile, '/') + 1 : callfile);
    if (index != -1 && _mys_rank_log_G.dests[index].file != NULL) {
        mys_log_self(MYS_LOG_FATAL, callfile, callline, "Reopen an activating rank log folder: %s", folder);
        goto _finished;
    }

    if (_mys_rank_log_G.max_used == _MYS_RANK_LOG_DEST_NUM) {
        mys_log_self(MYS_LOG_FATAL, callfile, callline, "Cannot open %s because too many folder opened (%d)", folder, _mys_rank_log_G.max_used);
        goto _finished;
    }
    char name[4096];
    snprintf(name, sizeof(name), "%s/%06d.log", folder, mys_mpi_myrank());
    mys_ensure_dir(folder, 0777);
    if (index == -1) {
        // First time. Open with create mode
        FILE *file = fopen(name, "w");
        if (file == NULL) {
            mys_log_self(MYS_LOG_FATAL, callfile, callline, "Error on opening rank log folder: %s", folder);
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
            mys_log_self(MYS_LOG_FATAL, callfile, callline, "Error on reopening rank log folder: %s", folder);
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

MYS_PUBLIC void mys_rank_log_close(const char *callfile, int callline, const char *folder)
{
    _mys_rank_log_init();
    size_t len = strnlen(folder, _MYS_FNAME_MAX);
    int index = _mys_rank_log_find_dest(folder, len);
    mys_mutex_lock(&_mys_rank_log_G.lock);
    size_t tot_wrote = _mys_rank_log_G.dests[index].tot_wrote;
    size_t cur_wrote = _mys_rank_log_G.dests[index].cur_wrote;
    callfile = (strrchr(callfile, '/') ? strrchr(callfile, '/') + 1 : callfile);

    if (index == -1) {
        mys_log_self(MYS_LOG_FATAL, callfile, callline, "Can not close invalid folder: %s", folder);
        goto _finished;
    }
    if (_mys_rank_log_G.dests[index].file == NULL) {
        mys_log_self(MYS_LOG_FATAL, callfile, callline, "Can not close closed folder: %s", folder);
        goto _finished;
    }
    fclose(_mys_rank_log_G.dests[index].file);
    _mys_rank_log_G.dests[index].file = NULL;

    mys_log(0, MYS_LOG_INFO, callfile, callline, "Closed rank log folder: %s (bytes wrote %zu, total wrote %zu)", folder, cur_wrote, tot_wrote);
    mys_mpi_barrier();
_finished:
    mys_mutex_unlock(&_mys_rank_log_G.lock);
}
