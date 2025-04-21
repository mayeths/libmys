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
#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "_config.h"
#include "mpistubs.h"
#include "macro.h"
#include "color.h"
#include "thread.h"
#include "math.h"

enum {
    MYS_LOG_TRACE,
    MYS_LOG_DEBUG,
    MYS_LOG_INFO,
    MYS_LOG_WARN,
    MYS_LOG_ERROR,
    MYS_LOG_FATAL,
    MYS_LOG_RAW,
    MYS_LOG_LEVEL_COUNT
};

/**
 * Print log message with 'TRACE' level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL, RAW)
 */
#define TLOG(rank, fmt, ...)      mys_log_rank(MYS_LOGGER_G, (rank), MYS_LOG_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define TLOG_WHEN(when, fmt, ...) mys_log_when(MYS_LOGGER_G, (when), MYS_LOG_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define TLOG_SELF(fmt, ...)       mys_log_self(MYS_LOGGER_G, MYS_LOG_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define TLOG_ONCE(fmt, ...)       mys_log_once(MYS_LOGGER_G, MYS_LOG_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__) // once (__FILE__, __LINE__)
#define TLOG_ORDERED(fmt, ...)    mys_log_ordered(MYS_LOGGER_G, MYS_LOG_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
/**
 * Print log message with 'DEBUG' level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL, RAW)
 */
#define DLOG(rank, fmt, ...)      mys_log_rank(MYS_LOGGER_G, (rank), MYS_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define DLOG_WHEN(when, fmt, ...) mys_log_when(MYS_LOGGER_G, (when), MYS_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define DLOG_SELF(fmt, ...)       mys_log_self(MYS_LOGGER_G, MYS_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define DLOG_ONCE(fmt, ...)       mys_log_once(MYS_LOGGER_G, MYS_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__) // once (__FILE__, __LINE__)
#define DLOG_ORDERED(fmt, ...)    mys_log_ordered(MYS_LOGGER_G, MYS_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
/**
 * Print log message with 'INFO' level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL, RAW)
 */
#define ILOG(rank, fmt, ...)      mys_log_rank(MYS_LOGGER_G, (rank), MYS_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ILOG_WHEN(when, fmt, ...) mys_log_when(MYS_LOGGER_G, (when), MYS_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ILOG_SELF(fmt, ...)       mys_log_self(MYS_LOGGER_G, MYS_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ILOG_ONCE(fmt, ...)       mys_log_once(MYS_LOGGER_G, MYS_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__) // once (__FILE__, __LINE__)
#define ILOG_ORDERED(fmt, ...)    mys_log_ordered(MYS_LOGGER_G, MYS_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
/**
 * Print log message with 'WARN' level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL, RAW)
 */
#define WLOG(rank, fmt, ...)      mys_log_rank(MYS_LOGGER_G, (rank), MYS_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define WLOG_WHEN(when, fmt, ...) mys_log_when(MYS_LOGGER_G, (when), MYS_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define WLOG_SELF(fmt, ...)       mys_log_self(MYS_LOGGER_G, MYS_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define WLOG_ONCE(fmt, ...)       mys_log_once(MYS_LOGGER_G, MYS_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__) // once (__FILE__, __LINE__)
#define WLOG_ORDERED(fmt, ...)    mys_log_ordered(MYS_LOGGER_G, MYS_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
/**
 * Print log message with 'ERROR' level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL, RAW)
 */
#define ELOG(rank, fmt, ...)      mys_log_rank(MYS_LOGGER_G, (rank), MYS_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ELOG_WHEN(when, fmt, ...) mys_log_when(MYS_LOGGER_G, (when), MYS_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ELOG_SELF(fmt, ...)       mys_log_self(MYS_LOGGER_G, MYS_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ELOG_ONCE(fmt, ...)       mys_log_once(MYS_LOGGER_G, MYS_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__) // once (__FILE__, __LINE__)
#define ELOG_ORDERED(fmt, ...)    mys_log_ordered(MYS_LOGGER_G, MYS_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
/**
 * Print log message with 'FATAL' level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL, RAW)
 */
#define FLOG(rank, fmt, ...)      mys_log_rank(MYS_LOGGER_G, (rank), MYS_LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define FLOG_WHEN(when, fmt, ...) mys_log_when(MYS_LOGGER_G, (when), MYS_LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define FLOG_SELF(fmt, ...)       mys_log_self(MYS_LOGGER_G, MYS_LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define FLOG_ONCE(fmt, ...)       mys_log_once(MYS_LOGGER_G, MYS_LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__) // once (__FILE__, __LINE__)
#define FLOG_ORDERED(fmt, ...)    mys_log_ordered(MYS_LOGGER_G, MYS_LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
/**
 * Print log message with 'RAW' level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL, RAW)
 */
#define RLOG(rank, fmt, ...)      mys_log_rank(MYS_LOGGER_G, (rank), MYS_LOG_RAW, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define RLOG_WHEN(when, fmt, ...) mys_log_when(MYS_LOGGER_G, (when), MYS_LOG_RAW, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define RLOG_SELF(fmt, ...)       mys_log_self(MYS_LOGGER_G, MYS_LOG_RAW, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define RLOG_ONCE(fmt, ...)       mys_log_once(MYS_LOGGER_G, MYS_LOG_RAW, __FILE__, __LINE__, fmt, ##__VA_ARGS__) // once (__FILE__, __LINE__)
#define RLOG_ORDERED(fmt, ...)    mys_log_ordered(MYS_LOGGER_G, MYS_LOG_RAW, __FILE__, __LINE__, fmt, ##__VA_ARGS__)


/////////////

typedef struct mys_log_t mys_log_t;
MYS_PUBVAR mys_log_t mys_predefined_logger;
#define MYS_LOGGER_G (&mys_predefined_logger)

typedef struct {
    int myrank;
    int nranks;
    int level;
    const char *file;
    int line;
    bool no_vargs; // true when fmt is the final string
} mys_log_event_t;

typedef void (*mys_log_handler_fn)(mys_log_t *logger, mys_log_event_t *event, const char *fmt, va_list vargs, void *udata);

/////////////

MYS_ATTR_PRINTF(6, 7) MYS_PUBLIC void mys_log_rank(mys_log_t *logger, int rank, int level, const char *file, int line, const char *fmt, ...);
MYS_ATTR_PRINTF(6, 7) MYS_PUBLIC void mys_log_when(mys_log_t *logger, int when, int level, const char *file, int line, const char *fmt, ...);
MYS_ATTR_PRINTF(5, 6) MYS_PUBLIC void mys_log_self(mys_log_t *logger, int level, const char *file, int line, const char *fmt, ...);
MYS_ATTR_PRINTF(5, 6) MYS_PUBLIC void mys_log_once(mys_log_t *logger, int level, const char *file, int line, const char *fmt, ...);
MYS_ATTR_PRINTF(5, 6) MYS_PUBLIC void mys_log_ordered(mys_log_t *logger, int level, const char *file, int line, const char *fmt, ...);
MYS_PUBLIC mys_log_t *mys_log_create(const char *name);
MYS_PUBLIC void mys_log_destroy(mys_log_t **logger);
MYS_PUBLIC const char *mys_log_get_name(mys_log_t *logger);

MYS_PUBLIC int mys_log_add_handler(mys_log_t *logger, mys_log_handler_fn handler_fn, void *handler_udata);
MYS_PUBLIC void mys_log_remove_handler(mys_log_t *logger, int handler_id);
MYS_PUBLIC void mys_log_clear_handler(mys_log_t *logger);
MYS_PUBLIC void mys_log_invoke_handlers(mys_log_t *logger, mys_log_event_t *event, const char *fmt, va_list vargs);
MYS_PUBLIC mys_MPI_Comm mys_log_get_comm(mys_log_t *logger);
MYS_PUBLIC void mys_log_set_comm(mys_log_t *logger, mys_MPI_Comm comm);
MYS_PUBLIC void mys_log_set_level(mys_log_t *logger, int level);
MYS_PUBLIC int mys_log_get_level(mys_log_t *logger);
MYS_PUBLIC const char* mys_log_level_string(int level);
MYS_PUBLIC void mys_log_silent(mys_log_t *logger, bool silent);

// [I::000 ex01.hello-gcc.c:012] Test ILOG function
MYS_PUBLIC void mys_log_stdio_handler1(mys_log_t *logger, mys_log_event_t *event, const char *fmt, va_list vargs, void *udata);
// <mys_predefined_logger::000 ex01.hello-gcc.c:012> Test ILOG function
MYS_PUBLIC void mys_log_stdio_handler2(mys_log_t *logger, mys_log_event_t *event, const char *fmt, va_list vargs, void *udata);
