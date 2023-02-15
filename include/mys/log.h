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
#define MYS_LOG_FNAME __FILE__
#else
#define MYS_LOG_FNAME (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

enum {
    MYS_LOG_TRACE, MYS_LOG_DEBUG, MYS_LOG_INFO, MYS_LOG_WARN, MYS_LOG_ERROR, MYS_LOG_FATAL, MYS_LOG_LEVEL_COUNT
};

#define TLOG(who, fmt, ...) mys_log(who, MYS_LOG_TRACE, MYS_LOG_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define DLOG(who, fmt, ...) mys_log(who, MYS_LOG_DEBUG, MYS_LOG_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define ILOG(who, fmt, ...) mys_log(who, MYS_LOG_INFO,  MYS_LOG_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define WLOG(who, fmt, ...) mys_log(who, MYS_LOG_WARN,  MYS_LOG_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define ELOG(who, fmt, ...) mys_log(who, MYS_LOG_ERROR, MYS_LOG_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define FLOG(who, fmt, ...) mys_log(who, MYS_LOG_FATAL, MYS_LOG_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define __LOG_ORDERED(LOG, fmt, ...) do { \
    int nranks = mys_nranks();            \
    for (int i = 0; i < nranks; i++) {    \
        LOG(i, fmt, ##__VA_ARGS__);       \
        mys_barrier();                    \
    }                                     \
} while (0)
#define TLOG_ORDERED(fmt, ...) __LOG_ORDERED(TLOG, fmt, ##__VA_ARGS__)
#define DLOG_ORDERED(fmt, ...) __LOG_ORDERED(DLOG, fmt, ##__VA_ARGS__)
#define ILOG_ORDERED(fmt, ...) __LOG_ORDERED(ILOG, fmt, ##__VA_ARGS__)
#define WLOG_ORDERED(fmt, ...) __LOG_ORDERED(WLOG, fmt, ##__VA_ARGS__)
#define ELOG_ORDERED(fmt, ...) __LOG_ORDERED(ELOG, fmt, ##__VA_ARGS__)
#define FLOG_ORDERED(fmt, ...) __LOG_ORDERED(FLOG, fmt, ##__VA_ARGS__)

typedef struct {
    int level;
    const char *file;
    int line;
    const char *fmt;
    va_list vargs;
    void *udata;
} mys_log_event_t;

typedef void (*mys_log_handler_fn)(mys_log_event_t *event);

__attribute__((format(printf, 5, 6)))
MYS_API void mys_log(int who, int level, const char *file, int line, const char *fmt, ...);
MYS_API int mys_log_add_handler(mys_log_handler_fn handler_fn, void *handler_udata);
MYS_API void mys_log_remove_handler(int handler_id);
MYS_API int mys_log_get_level();
MYS_API void mys_log_set_level(int level);
MYS_API const char* mys_log_level_string(int level);


////// MYS_LOG_VERSION 1 Compatibility

#define DEBUG(who, fmt, ...) DLOG(who, fmt, ##__VA_ARGS__)
#define DEBUG_ORDERED(fmt, ...) DLOG_ORDERED(fmt, ##__VA_ARGS__)
#define FAILED(fmt, ...) FLOG(MYRANK(), fmt, ##__VA_ARGS__)
#define THROW_NOT_IMPL() FAILED("Not implemented.")


////// Internal

typedef struct {
    bool inited;
    mys_mutex_t lock;
    int level;
    int last_level;
    struct {
        mys_log_handler_fn fn;
        void *udata;
        int id;
    } handlers[128];
} _mys_log_G_t;

extern _mys_log_G_t _mys_log_G;

MYS_API void mys_log_init();
