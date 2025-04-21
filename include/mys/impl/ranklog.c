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

    mys_log_rank(MYS_LOGGER_G, 0, MYS_LOG_INFO, callfile, callline, "Opened rank log folder: %s", folder);
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

    mys_log_rank(MYS_LOGGER_G, 0, MYS_LOG_INFO, callfile, callline, "Closed rank log folder: %s (bytes wrote %zu, total wrote %zu)", folder, cur_wrote, tot_wrote);
    mys_MPI_Barrier(mys_MPI_COMM_WORLD);
_finished:
    mys_mutex_unlock(&_mys_rank_log_G.lock);
}
