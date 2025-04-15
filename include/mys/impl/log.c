/* 
 * Copyright (c) 2025 Haopeng Huang - All Rights Reserved
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

#define MYS_LOG_MAX_HANDLER 8

typedef struct {
    char *key;
    _mys_UT_hash_handle hh;
} _mys_log_once_t;

typedef struct mys_log_t {
    bool inited;
    const char *name;
    mys_mutex_t lock;
    mys_MPI_Comm comm;
    int level;
    bool silent;
    _mys_log_once_t *once_map;
    struct {
        mys_log_handler_fn fn;
        void *udata;
        int id;
    } handlers[MYS_LOG_MAX_HANDLER];
    int handler_id_counter;
    int num_handlers;
} mys_log_t;

mys_log_t mys_predefined_logger = {
    .inited = true,
    .name = "mys_predefined_logger",
    .lock = MYS_MUTEX_INITIALIZER,
    .comm = mys_MPI_COMM_WORLD,
    .level = MYS_LOG_TRACE,
    .silent = false,
    .once_map = NULL,
    .handlers = {
        { .fn = mys_log_stdio_handler1, .udata = NULL, .id = 100 }
    },
    .handler_id_counter = 101,
    .num_handlers = 1,
};

MYS_PUBLIC mys_log_t *mys_log_create(const char *name)
{
    mys_log_t *logger = (mys_log_t *)mys_calloc2(MYS_ARENA_LOG, sizeof(mys_log_t), 1);
    mys_mutex_init(&logger->lock);
    mys_mutex_lock(&logger->lock);
    logger->inited = true;
    logger->name = strndup(name, 1024);
    logger->comm = mys_MPI_COMM_WORLD;
    logger->level = MYS_LOG_TRACE;
    logger->silent = false;
    logger->once_map = NULL;
    logger->handler_id_counter = 100;
    logger->num_handlers = 0;
    mys_mutex_unlock(&logger->lock);
    return logger;
}

MYS_PUBLIC void mys_log_destroy(mys_log_t **_logger)
{
    if (_logger == NULL || *_logger == NULL || *_logger == &mys_predefined_logger)
        return;
    mys_log_t *logger = *_logger;
    mys_mutex_lock(&logger->lock);
    _mys_log_once_t *entry, *tmp;
    _HASH_ITER(hh, logger->once_map, entry, tmp) {
        _HASH_DEL(logger->once_map, entry);
        free(entry->key);
        free(entry);
    }
    free((char *)(void *)logger->name);
    mys_mutex_unlock(&logger->lock);
    mys_mutex_destroy(&logger->lock);
    mys_free2(MYS_ARENA_LOG, logger, sizeof(mys_log_t));
    *_logger = NULL;
}

MYS_PUBLIC const char *mys_log_get_name(mys_log_t *logger)
{
    const char *name;
    mys_mutex_lock(&logger->lock);
    name = logger->name;
    mys_mutex_unlock(&logger->lock);
    return name;
}


MYS_STATIC void _mys_log_impl(mys_log_t *logger, int who, int level, const char *file, int line, const char *fmt, va_list vargs)
{
    mys_mutex_lock(&logger->lock);
    if (logger->silent == true) {
        mys_mutex_unlock(&logger->lock);
        return;
    }
    int myrank, nranks;
    mys_MPI_Comm_rank(logger->comm, &myrank);
    mys_MPI_Comm_size(logger->comm, &nranks);
    if (who == myrank && (int)level >= (int)logger->level) {
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
        mys_log_invoke_handlers(logger, &event, fmt, vargs);
    }
    mys_mutex_unlock(&logger->lock);
}

MYS_PUBLIC void mys_log_who(mys_log_t *logger, int who, int level, const char *file, int line, const char *fmt, ...)
{
    va_list vargs;
    va_start(vargs, fmt);
    _mys_log_impl(logger, who, level, file, line, fmt, vargs);
    va_end(vargs);
}

MYS_PUBLIC void mys_log_when(mys_log_t *logger, int cond, int level, const char *file, int line, const char *fmt, ...)
{
    if (cond == 0)
        return;
    int myrank;
    mys_MPI_Comm_rank(logger->comm, &myrank);
    va_list vargs;
    va_start(vargs, fmt);
    _mys_log_impl(logger, myrank, level, file, line, fmt, vargs);
    va_end(vargs);
}

MYS_PUBLIC void mys_log_self(mys_log_t *logger, int level, const char *file, int line, const char *fmt, ...)
{
    int myrank;
    mys_MPI_Comm_rank(logger->comm, &myrank);
    va_list vargs;
    va_start(vargs, fmt);
    _mys_log_impl(logger, myrank, level, file, line, fmt, vargs);
    va_end(vargs);
}

MYS_PUBLIC void mys_log_once(mys_log_t *logger, int level, const char *file, int line, const char *fmt, ...)
{
    // log only once at "file:line".
    _mys_log_once_t *entry = NULL, *new_entry = NULL;
    char key[1024];
    snprintf(key, sizeof(key), "%s:%d", file, line);
    mys_mutex_lock(&logger->lock);
    {
        _HASH_FIND_STR(logger->once_map, key, entry);
    }
    mys_mutex_unlock(&logger->lock);

    if (entry != NULL) // already logged
        return;

    new_entry = (_mys_log_once_t *)malloc(sizeof(_mys_log_once_t));
    new_entry->key = strndup(key, sizeof(key));
    mys_mutex_lock(&logger->lock);
    {
        _HASH_ADD_STR(logger->once_map, key, new_entry);
    }
    mys_mutex_unlock(&logger->lock);

    int myrank;
    mys_MPI_Comm_rank(logger->comm, &myrank);
    va_list vargs;
    va_start(vargs, fmt);
    _mys_log_impl(logger, myrank, level, file, line, fmt, vargs);
    va_end(vargs);
}


MYS_PUBLIC void mys_log_ordered(mys_log_t *logger, int level, const char *file, int line, const char *fmt, ...)
{
    mys_mutex_lock(&logger->lock);
    if (logger->silent == true) {
        mys_mutex_unlock(&logger->lock);
        return;
    }
    int myrank, nranks;
    mys_MPI_Comm_rank(logger->comm, &myrank);
    mys_MPI_Comm_size(logger->comm, &nranks);

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
        mys_log_invoke_handlers(logger, &event, fmt, vargs);
        va_end(vargs);
        char buffer[4096];
        for (int rank = 1; rank < nranks; rank++) {
            mys_MPI_Status status;
            int needed = 0;
            mys_MPI_Probe(rank, tag, logger->comm, &status);
            mys_MPI_Get_count(&status, mys_MPI_CHAR, &needed);
            char *ptr = (needed > 4096) ? (char *)mys_malloc2(MYS_ARENA_LOG, needed) : buffer;
            mys_MPI_Recv(ptr, needed, mys_MPI_CHAR, rank, tag, logger->comm, mys_MPI_STATUS_IGNORE);

            if (!broken) {
                event.myrank = rank;
                fmt = ptr;
                event.no_vargs = true;
                mys_log_invoke_handlers(logger, &event, fmt, vargs);
            }
            if (ptr != buffer)
                mys_free2(MYS_ARENA_LOG, ptr, needed);
        }
        mys_MPI_Barrier(logger->comm);
    } else {
        char buffer[4096];
        va_list vargs, vargs_test;
        va_start(vargs, fmt);
        va_copy(vargs_test, vargs);
        int needed = vsnprintf(NULL, 0, fmt, vargs_test) + 1;
        va_end(vargs_test);
        char *ptr = (needed > 4096) ? (char *)mys_malloc2(MYS_ARENA_LOG, needed) : buffer;
        vsnprintf(ptr, needed, fmt, vargs);
        mys_MPI_Send(ptr, needed, mys_MPI_CHAR, 0, tag, logger->comm);
        if (ptr != buffer)
            mys_free2(MYS_ARENA_LOG, ptr, needed);
        va_end(vargs);
        mys_MPI_Barrier(logger->comm); // We don't expect logging increase processes' nondeterministic
    }

    mys_mutex_unlock(&logger->lock);
}

MYS_PUBLIC int mys_log_add_handler(mys_log_t *logger, mys_log_handler_fn handler_fn, void *handler_udata)
{
    mys_mutex_lock(&logger->lock);
    if (logger->num_handlers >= MYS_LOG_MAX_HANDLER) {
        mys_mutex_unlock(&logger->lock);
        return -1;
    }
    if (handler_fn == NULL) {
        mys_mutex_unlock(&logger->lock);
        return -1;
    }
    int idx = logger->num_handlers++;
    int id = logger->handler_id_counter++;
    logger->handlers[idx].fn = handler_fn;
    logger->handlers[idx].udata = handler_udata;
    logger->handlers[idx].id = id;
    mys_mutex_unlock(&logger->lock);
    return id;
}

MYS_PUBLIC void mys_log_remove_handler(mys_log_t *logger, int handler_id)
{
    mys_mutex_lock(&logger->lock);
    int idx = -1;
    for (int i = 0; i < logger->num_handlers; i++) {
        if (logger->handlers[i].id == handler_id) {
            idx = i;
            break;
        }
    }
    if (idx != -1) {
        for (int i = idx + 1; i < logger->num_handlers; i++) {
            logger->handlers[i - 1].fn = logger->handlers[i].fn;
            logger->handlers[i - 1].udata = logger->handlers[i].udata;
            logger->handlers[i - 1].id = logger->handlers[i].id;
        }
        logger->num_handlers--;
    }
    mys_mutex_unlock(&logger->lock);
}

MYS_PUBLIC void mys_log_clear_handler(mys_log_t *logger)
{
    mys_mutex_lock(&logger->lock);
    logger->num_handlers = 0;
    mys_mutex_unlock(&logger->lock);
}

MYS_PUBLIC void mys_log_invoke_handlers(mys_log_t *logger, mys_log_event_t *event, const char *fmt, va_list vargs)
{
    for (int i = 0; i < logger->num_handlers; i++) {
        va_list vargs_copy;
        va_copy(vargs_copy, vargs);
        logger->handlers[i].fn(logger, event, fmt, vargs_copy, logger->handlers[i].udata);
        va_end(vargs_copy);
    }
}

MYS_PUBLIC mys_MPI_Comm mys_log_get_comm(mys_log_t *logger)
{
    mys_MPI_Comm comm;
    mys_mutex_lock(&logger->lock);
    comm = logger->comm;
    mys_mutex_unlock(&logger->lock);
    return comm;
}

MYS_PUBLIC void mys_log_set_comm(mys_log_t *logger, mys_MPI_Comm comm)
{
    mys_mutex_lock(&logger->lock);
    logger->comm = comm;
    mys_mutex_unlock(&logger->lock);
}

MYS_PUBLIC int mys_log_get_level(mys_log_t *logger)
{
    mys_mutex_lock(&logger->lock);
    int level = logger->level;
    mys_mutex_unlock(&logger->lock);
    return level;
}

MYS_PUBLIC void mys_log_set_level(mys_log_t *logger, int level)
{
    mys_mutex_lock(&logger->lock);
    logger->level = level;
    mys_mutex_unlock(&logger->lock);
}

MYS_PUBLIC void mys_log_silent(mys_log_t *logger, bool silent)
{
    mys_mutex_lock(&logger->lock);
    logger->silent = silent;
    mys_mutex_unlock(&logger->lock);
}

MYS_PUBLIC const char* mys_log_level_string(int level)
{
    const char *level_strings[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
    };
    return level_strings[(int)level];
}

MYS_PUBLIC void mys_log_stdio_handler1(mys_log_t *logger, mys_log_event_t *event, const char *fmt, va_list vargs, void *udata)
{
    (void)(logger);
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

MYS_PUBLIC void mys_log_stdio_handler2(mys_log_t *logger, mys_log_event_t *event, const char *fmt, va_list vargs, void *udata)
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
        snprintf(label, label_size, "<%s::%0*d %s:%0*d>",
            logger->name, rank_digits, event->myrank,
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

MYS_PUBLIC void mys_ranklog_old(const char *callfile, int callline, const char *folder, const char *fmt, ...)
{
    _mys_rank_log_init();
    va_list vargs;
    size_t wrote = 0;
    size_t len = strnlen(folder, _MYS_FNAME_MAX);
    int index = _mys_rank_log_find_dest(folder, len);
    mys_mutex_lock(&_mys_rank_log_G.lock);
    callfile = (strrchr(callfile, '/') ? strrchr(callfile, '/') + 1 : callfile);
    if (index == -1) {
        mys_log_self(MYS_LOGGER_G, MYS_LOG_FATAL, callfile, callline, "Invoked mys_ranklog_old() with invalid folder: %s", folder);
        goto _finished;
    }
    if (_mys_rank_log_G.dests[index].file == NULL) {
        mys_log_self(MYS_LOGGER_G, MYS_LOG_FATAL, callfile, callline, "Invoked mys_ranklog_old() with closed folder: %s", folder);
        goto _finished;
    }
    if (fmt == NULL) {
        mys_log_self(MYS_LOGGER_G, MYS_LOG_FATAL, callfile, callline, "Invoked mys_ranklog_old() with NULL format.");
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

MYS_PUBLIC void mys_ranklog_open_old(const char *callfile, int callline, const char *folder)
{
    _mys_rank_log_init();
    int myrank;
    mys_MPI_Comm_rank(mys_MPI_COMM_WORLD, &myrank);
    size_t len = strnlen(folder, _MYS_FNAME_MAX);
    int index = _mys_rank_log_find_dest(folder, len);
    mys_mutex_lock(&_mys_rank_log_G.lock);
    callfile = (strrchr(callfile, '/') ? strrchr(callfile, '/') + 1 : callfile);
    if (index != -1 && _mys_rank_log_G.dests[index].file != NULL) {
        mys_log_self(MYS_LOGGER_G, MYS_LOG_FATAL, callfile, callline, "Reopen an activating rank log folder: %s", folder);
        goto _finished;
    }

    if (_mys_rank_log_G.max_used == _MYS_RANK_LOG_DEST_NUM) {
        mys_log_self(MYS_LOGGER_G, MYS_LOG_FATAL, callfile, callline, "Cannot open %s because too many folder opened (%d)", folder, _mys_rank_log_G.max_used);
        goto _finished;
    }
    char name[4096];
    snprintf(name, sizeof(name), "%s/%06d.log", folder, myrank);
    mys_ensure_dir(folder, 0777);
    if (index == -1) {
        // First time. Open with create mode
        FILE *file = fopen(name, "w");
        if (file == NULL) {
            mys_log_self(MYS_LOGGER_G, MYS_LOG_FATAL, callfile, callline, "Error on opening rank log folder: %s", folder);
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
            mys_log_self(MYS_LOGGER_G, MYS_LOG_FATAL, callfile, callline, "Error on reopening rank log folder: %s", folder);
            goto _finished;
        }
        _mys_rank_log_G.dests[index].file = file;
        _mys_rank_log_G.dests[index].cur_wrote = 0;
    }

    mys_log_who(MYS_LOGGER_G, 0, MYS_LOG_INFO, callfile, callline, "Opened rank log folder: %s", folder);
    mys_MPI_Barrier(mys_MPI_COMM_WORLD);
_finished:
    mys_mutex_unlock(&_mys_rank_log_G.lock);
}

MYS_PUBLIC void mys_ranklog_close_old(const char *callfile, int callline, const char *folder)
{
    _mys_rank_log_init();
    size_t len = strnlen(folder, _MYS_FNAME_MAX);
    int index = _mys_rank_log_find_dest(folder, len);
    mys_mutex_lock(&_mys_rank_log_G.lock);
    size_t tot_wrote = _mys_rank_log_G.dests[index].tot_wrote;
    size_t cur_wrote = _mys_rank_log_G.dests[index].cur_wrote;
    callfile = (strrchr(callfile, '/') ? strrchr(callfile, '/') + 1 : callfile);

    if (index == -1) {
        mys_log_self(MYS_LOGGER_G, MYS_LOG_FATAL, callfile, callline, "Can not close invalid folder: %s", folder);
        goto _finished;
    }
    if (_mys_rank_log_G.dests[index].file == NULL) {
        mys_log_self(MYS_LOGGER_G, MYS_LOG_FATAL, callfile, callline, "Can not close closed folder: %s", folder);
        goto _finished;
    }
    fclose(_mys_rank_log_G.dests[index].file);
    _mys_rank_log_G.dests[index].file = NULL;

    mys_log_who(MYS_LOGGER_G, 0, MYS_LOG_INFO, callfile, callline, "Closed rank log folder: %s (bytes wrote %zu, total wrote %zu)", folder, cur_wrote, tot_wrote);
    mys_MPI_Barrier(mys_MPI_COMM_WORLD);
_finished:
    mys_mutex_unlock(&_mys_rank_log_G.lock);
}
