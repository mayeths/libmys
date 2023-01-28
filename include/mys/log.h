#pragma once

#define MYS_LOG_VERSION 2
// #define MYS_LOG_DISABLE_STDOUT_HANDLER

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#include "config.h"
#include "macro.h"
#include "thread.h"
#include "myspi.h"


////// API

#ifdef MYS_LOG_LONG_FILENAME
#define __MYS_LOG_FNAME__ __FILE__
#else
#define __MYS_LOG_FNAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define TLOG(who, fmt, ...) mys_log(who, MYS_LOG_TRACE, __MYS_LOG_FNAME__, __LINE__, fmt, ##__VA_ARGS__)
#define DLOG(who, fmt, ...) mys_log(who, MYS_LOG_DEBUG, __MYS_LOG_FNAME__, __LINE__, fmt, ##__VA_ARGS__)
#define ILOG(who, fmt, ...) mys_log(who, MYS_LOG_INFO,  __MYS_LOG_FNAME__, __LINE__, fmt, ##__VA_ARGS__)
#define WLOG(who, fmt, ...) mys_log(who, MYS_LOG_WARN,  __MYS_LOG_FNAME__, __LINE__, fmt, ##__VA_ARGS__)
#define ELOG(who, fmt, ...) mys_log(who, MYS_LOG_ERROR, __MYS_LOG_FNAME__, __LINE__, fmt, ##__VA_ARGS__)
#define FLOG(who, fmt, ...) mys_log(who, MYS_LOG_FATAL, __MYS_LOG_FNAME__, __LINE__, fmt, ##__VA_ARGS__)

enum {
    MYS_LOG_TRACE, MYS_LOG_DEBUG, MYS_LOG_INFO, MYS_LOG_WARN, MYS_LOG_ERROR, MYS_LOG_FATAL, MYS_LOG_LEVEL_COUNT
};

typedef struct {
    int level;
    const char *file;
    int line;
    const char *fmt;
    va_list vargs;
    void *udata;
} mys_log_event_t;

typedef void (*mys_log_handler_fn)(mys_log_event_t *event);

MYS_API static int mys_log_add_handler(mys_log_handler_fn handler_fn, void *handler_udata);
MYS_API static void mys_log_remove_handler(int handler_id);
MYS_API static void mys_log_set_level(int level);
MYS_API static int mys_log_get_level();


////// MYS_LOG_VERSION 1 Compatibility

#define DEBUG(who, fmt, ...) DLOG(who, fmt, ##__VA_ARGS__)
#define DEBUG_ORDERED(fmt, ...) do {   \
    int nranks = NRANKS();             \
    for (int i = 0; i < nranks; i++) { \
        DEBUG(i, fmt, ##__VA_ARGS__);  \
        BARRIER();                     \
    }                                  \
} while (0)
#define FAILED(fmt, ...) FLOG(MYRANK(), fmt, ##__VA_ARGS__)
#define THROW_NOT_IMPL() FAILED("Not implemented.")
#define WAIT_FLAG(flagfile) __mys_wait_flag(__FILE__, __LINE__, flagfile)


////// Internal

typedef struct {
    int level;
    int last_level;
    mys_mutex_t lock;
    struct {
        mys_log_handler_fn fn;
        void *udata;
        int id;
    } handlers[128];
} mys_log_G_t;

extern mys_log_G_t mys_log_G;

__attribute__((format(printf, 5, 6)))
MYS_API static void mys_log(int who, int level, const char *file, int line, const char *fmt, ...)
{
    mys_mutex_lock(&mys_log_G.lock);
    int myrank = mys_myrank();
    if (who == myrank && (int)level >= (int)mys_log_G.level) {
        for (int i = 0; i < 128; i++) {
            if (mys_log_G.handlers[i].fn == NULL)
                break;
            mys_log_event_t event;
            event.level = level;
            event.file = file;
            event.line = line;
            event.fmt = fmt;
            event.udata = mys_log_G.handlers[i].udata;
            va_start(event.vargs, fmt);
            mys_log_G.handlers[i].fn(&event);
            va_end(event.vargs);
        }
    }
    mys_mutex_unlock(&mys_log_G.lock);
}

MYS_API static int mys_log_add_handler(mys_log_handler_fn handler_fn, void *handler_udata)
{
    mys_mutex_lock(&mys_log_G.lock);
    int used_max_id = INT32_MIN;
    for (int i = 0; i < 128; i++) {
        if (mys_log_G.handlers[i].fn == NULL)
            break;
        if (used_max_id < mys_log_G.handlers[i].id)
            used_max_id = mys_log_G.handlers[i].id;
    }
    int id = used_max_id + 1;
    for (int i = 0; i < 128; i++) {
        if (mys_log_G.handlers[i].fn != NULL)
            continue;
        mys_log_G.handlers[i].fn = handler_fn;
        mys_log_G.handlers[i].udata = handler_udata;
        mys_log_G.handlers[i].id = id;
        break;
    }
    mys_mutex_unlock(&mys_log_G.lock);
    return id;
}

MYS_API static void mys_log_remove_handler(int handler_id)
{
    mys_mutex_lock(&mys_log_G.lock);
    for (int i = 0; i < 128; i++) {
        if (mys_log_G.handlers[i].id != handler_id)
            continue;
        mys_log_G.handlers[i].fn = NULL;
        mys_log_G.handlers[i].udata = NULL;
        mys_log_G.handlers[i].id = 0;
        for (int j = i + 1; j < 128; j++) {
            if (mys_log_G.handlers[j].fn == NULL)
                break;
            mys_log_G.handlers[j - 1].fn = mys_log_G.handlers[j].fn;
            mys_log_G.handlers[j - 1].udata = mys_log_G.handlers[j].udata;
            mys_log_G.handlers[j - 1].id = mys_log_G.handlers[j].id;
        }
        break;
    }
    mys_mutex_unlock(&mys_log_G.lock);
}

MYS_API static int mys_log_get_level()
{
    return mys_log_G.level;
}

MYS_API static void mys_log_set_level(int level)
{
    mys_mutex_lock(&mys_log_G.lock);
    mys_log_G.level = level;
    mys_mutex_unlock(&mys_log_G.lock);
}

MYS_API static const char* mys_log_level_string(int level)
{
    const char *level_strings[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
    };
    return level_strings[(int)level];
}

MYS_API static void mys_log_stdio_handler(mys_log_event_t *event) {
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
