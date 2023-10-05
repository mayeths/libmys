#pragma once

// #define MYS_LOG_DISABLE_STDOUT_HANDLER

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "_config.h"
#include "_lib/mpi.h"
#include "macro.h"
#include "thread.h"


////// API

enum {
    MYS_LOG_TRACE,
    MYS_LOG_DEBUG,
    MYS_LOG_INFO,
    MYS_LOG_WARN,
    MYS_LOG_ERROR,
    MYS_LOG_FATAL,
    MYS_LOG_RAW,
    MYS_LOG_XFILE,
    MYS_LOG_LEVEL_COUNT
};

/**
 * Print log message with 'TRACE' level ( [less important->] TDIWEFR [->most important] )
 */
#define TLOG(who, fmt, ...) mys_log(who, MYS_LOG_TRACE, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define TLOG_SELF(fmt, ...) mys_log(mys_mpi_myrank(), MYS_LOG_TRACE, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define TLOG_ORDERED(fmt, ...) mys_log_ordered(MYS_LOG_TRACE, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
/**
 * Print log message with 'DEBUG' level ( [less important->] TDIWEFR [->most important] )
 */
#define DLOG(who, fmt, ...) mys_log(who, MYS_LOG_DEBUG, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define DLOG_SELF(fmt, ...) mys_log(mys_mpi_myrank(), MYS_LOG_DEBUG, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define DLOG_ORDERED(fmt, ...) mys_log_ordered(MYS_LOG_DEBUG, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
/**
 * Print log message with 'INFO' level ( [less important->] TDIWEFR [->most important] )
 */
#define ILOG(who, fmt, ...) mys_log(who, MYS_LOG_INFO, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define ILOG_SELF(fmt, ...) mys_log(mys_mpi_myrank(), MYS_LOG_INFO, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define ILOG_ORDERED(fmt, ...) mys_log_ordered(MYS_LOG_INFO, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
/**
 * Print log message with 'WARN' level ( [less important->] TDIWEFR [->most important] )
 */
#define WLOG(who, fmt, ...) mys_log(who, MYS_LOG_WARN, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define WLOG_SELF(fmt, ...) mys_log(mys_mpi_myrank(), MYS_LOG_WARN,  MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define WLOG_ORDERED(fmt, ...) mys_log_ordered(MYS_LOG_WARN, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
/**
 * Print log message with 'ERROR' level ( [less important->] TDIWEFR [->most important] )
 */
#define ELOG(who, fmt, ...) mys_log(who, MYS_LOG_ERROR, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define ELOG_SELF(fmt, ...) mys_log(mys_mpi_myrank(), MYS_LOG_ERROR, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define ELOG_ORDERED(fmt, ...) mys_log_ordered(MYS_LOG_ERROR, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
/**
 * Print log message with 'FATAL' level ( [less important->] TDIWEFR [->most important] )
 */
#define FLOG(who, fmt, ...) mys_log(who, MYS_LOG_FATAL, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define FLOG_SELF(fmt, ...) mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define FLOG_ORDERED(fmt, ...) mys_log_ordered(MYS_LOG_FATAL, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
/**
 * Print log message with 'RAW' level ( [less important->] TDIWEFR [->most important] )
 */
#define RLOG(who, fmt, ...) mys_log(who, MYS_LOG_RAW, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define RLOG_SELF(fmt, ...) mys_log(mys_mpi_myrank(), MYS_LOG_RAW, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
#define RLOG_ORDERED(fmt, ...) mys_log_ordered(MYS_LOG_RAW, MYS_FNAME, __LINE__, fmt, ##__VA_ARGS__)
/**
 * TODO
 * Behavior: Default to stdout to force explicit open and close a folder
 * Upon the first time opening, print a info log to stdout to indicate using this facility.
 * XLOG will flush message to last opening target, which we store the folder name internally.
 * In our design, such a function may be high-overhead, so it should be striking enough to prevent accident outputing.
 */
#define RANKLOG(folder, fmt, ...) mys_rank_log(MYS_FNAME, __LINE__, folder, fmt, ##__VA_ARGS__)
#define RANKLOG_OPEN(folder) mys_rank_log_open(MYS_FNAME, __LINE__, folder) // Collective call
#define RANKLOG_CLOSE(folder) mys_rank_log_close(MYS_FNAME, __LINE__, folder) // Collective call

#define LOG_SILENT() mys_log_silent(true)
#define LOG_UNSILENT() mys_log_silent(false)

#define THROW_NOT_IMPL() FLOG(MYRANK(), "Not implemented.")

typedef struct {
    int myrank;
    int nranks;
    int level;
    const char *file;
    int line;
    const char *fmt;
    bool no_vargs; // true when fmt is the final string
    va_list vargs;
} mys_log_event_t;

typedef void (*mys_log_handler_fn)(mys_log_event_t *event, void *udata);

/////// log
__attribute__((format(printf, 5, 6)))
MYS_API void mys_log(int who, int level, const char *file, int line, const char *fmt, ...);
__attribute__((format(printf, 4, 5)))
MYS_API void mys_log_ordered(int level, const char *file, int line, const char *fmt, ...);
MYS_API int mys_log_add_handler(mys_log_handler_fn handler_fn, void *handler_udata);
MYS_API void mys_log_remove_handler(int handler_id);
MYS_API void mys_log_invoke_handlers(mys_log_event_t *event);
MYS_API int mys_log_get_level();
MYS_API void mys_log_set_level(int level);
MYS_API void mys_log_silent(bool silent);
MYS_API const char* mys_log_level_string(int level);
/////// rank log
__attribute__((format(printf, 4, 5)))
MYS_API void mys_rank_log(const char *callfile, int callline, const char *folder, const char *fmt, ...);
MYS_API void mys_rank_log_open(const char *callfile, int callline, const char *folder);
MYS_API void mys_rank_log_close(const char *callfile, int callline, const char *folder);


#define MCOLOR_NO        "\x1b[0m"
#define MCOLOR_BOLD      "\x1b[1m"
#define MCOLOR_BLACK     "\x1b[30m"
#define MCOLOR_RED       "\x1b[31m"
#define MCOLOR_GREEN     "\x1b[32m"
#define MCOLOR_YELLO     "\x1b[33m"
#define MCOLOR_BLUE      "\x1b[34m"
#define MCOLOR_PURPLE    "\x1b[35m"
#define MCOLOR_CYAN      "\x1b[36m"
#define MCOLOR_GRAY      "\x1b[37m"
#define MCOLOR_B_BLACK   "\x1b[40m"
#define MCOLOR_B_RED     "\x1b[41m"
#define MCOLOR_B_GREEN   "\x1b[42m"
#define MCOLOR_B_YELLOW  "\x1b[43m"
#define MCOLOR_B_BLUE    "\x1b[44m"
#define MCOLOR_B_MAGENTA "\x1b[45m"
#define MCOLOR_B_CYAN    "\x1b[46m"
#define MCOLOR_B_WHITE   "\x1b[47m"


////// Internal

typedef struct {
    bool inited;
    mys_mutex_t lock;
    int level;
    bool silent;
    struct {
        mys_log_handler_fn fn;
        void *udata;
        int id;
    } handlers[128];
} _mys_log_G_t;

extern _mys_log_G_t _mys_log_G;

MYS_API void mys_log_init();
