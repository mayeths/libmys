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
#pragma once

// #define MYS_LOG_DISABLE_STDOUT_HANDLER

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "_config.h"
#include "mpi.h"
#include "macro.h"
#include "thread.h"
#include "math.h"


////// API

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
#define TLOG(who, fmt, ...)       mys_log((who), MYS_LOG_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define TLOG_SELF(fmt, ...)       mys_log_self(MYS_LOG_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define TLOG_ONCE(fmt, ...)       mys_log_once(MYS_LOG_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__) // once (__FILE__, __LINE__)
#define TLOG_WHEN(cond, fmt, ...) mys_log_when((cond), MYS_LOG_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define TLOG_ORDERED(fmt, ...)    mys_log_ordered(MYS_LOG_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
/**
 * Print log message with 'DEBUG' level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL, RAW)
 */
#define DLOG(who, fmt, ...)       mys_log((who), MYS_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define DLOG_SELF(fmt, ...)       mys_log_self(MYS_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define DLOG_ONCE(fmt, ...)       mys_log_once(MYS_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__) // once (__FILE__, __LINE__)
#define DLOG_WHEN(cond, fmt, ...) mys_log_when((cond), MYS_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define DLOG_ORDERED(fmt, ...)    mys_log_ordered(MYS_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
/**
 * Print log message with 'INFO' level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL, RAW)
 */
#define ILOG(who, fmt, ...)       mys_log((who), MYS_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ILOG_SELF(fmt, ...)       mys_log_self(MYS_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ILOG_ONCE(fmt, ...)       mys_log_once(MYS_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__) // once (__FILE__, __LINE__)
#define ILOG_WHEN(cond, fmt, ...) mys_log_when((cond), MYS_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ILOG_ORDERED(fmt, ...)    mys_log_ordered(MYS_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
/**
 * Print log message with 'WARN' level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL, RAW)
 */
#define WLOG(who, fmt, ...)       mys_log((who), MYS_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define WLOG_SELF(fmt, ...)       mys_log_self(MYS_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define WLOG_ONCE(fmt, ...)       mys_log_once(MYS_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__) // once (__FILE__, __LINE__)
#define WLOG_WHEN(cond, fmt, ...) mys_log_when((cond), MYS_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define WLOG_ORDERED(fmt, ...)    mys_log_ordered(MYS_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
/**
 * Print log message with 'ERROR' level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL, RAW)
 */
#define ELOG(who, fmt, ...)       mys_log((who), MYS_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ELOG_SELF(fmt, ...)       mys_log_self(MYS_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ELOG_ONCE(fmt, ...)       mys_log_once(MYS_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__) // once (__FILE__, __LINE__)
#define ELOG_WHEN(cond, fmt, ...) mys_log_when((cond), MYS_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ELOG_ORDERED(fmt, ...)    mys_log_ordered(MYS_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
/**
 * Print log message with 'FATAL' level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL, RAW)
 */
#define FLOG(who, fmt, ...)       mys_log((who), MYS_LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define FLOG_SELF(fmt, ...)       mys_log_self(MYS_LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define FLOG_ONCE(fmt, ...)       mys_log_once(MYS_LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__) // once (__FILE__, __LINE__)
#define FLOG_WHEN(cond, fmt, ...) mys_log_when((cond), MYS_LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define FLOG_ORDERED(fmt, ...)    mys_log_ordered(MYS_LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
/**
 * Print log message with 'RAW' level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL, RAW)
 */
#define RLOG(who, fmt, ...)       mys_log((who), MYS_LOG_RAW, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define RLOG_SELF(fmt, ...)       mys_log_self(MYS_LOG_RAW, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define RLOG_ONCE(fmt, ...)       mys_log_once(MYS_LOG_RAW, __FILE__, __LINE__, fmt, ##__VA_ARGS__) // once (__FILE__, __LINE__)
#define RLOG_WHEN(cond, fmt, ...) mys_log_when((cond), MYS_LOG_RAW, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define RLOG_ORDERED(fmt, ...)    mys_log_ordered(MYS_LOG_RAW, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
/**
 * Print log message separately to a file named 'rank:6d.log' within the folder, like to folder/000001.log
```c
RANKLOG_OPEN("LOG.test");
RANKLOG("LOG.test", "setup time %f", setup_time);
RANKLOG("LOG.test", "solve time %f", solve_time);
RANKLOG_CLOSE("LOG.test");
```
 */
#define RANKLOG(folder, fmt, ...) mys_rank_log(__FILE__, __LINE__, folder, fmt, ##__VA_ARGS__)
#define RANKLOG_OPEN(folder) mys_rank_log_open(__FILE__, __LINE__, folder) // Collective call
#define RANKLOG_CLOSE(folder) mys_rank_log_close(__FILE__, __LINE__, folder) // Collective call

#define LOG_SILENT() mys_log_silent(true)
#define LOG_UNSILENT() mys_log_silent(false)

typedef struct {
    int myrank;
    int nranks;
    int level;
    const char *file;
    int line;
    bool no_vargs; // true when fmt is the final string
} mys_log_event_t;

typedef void (*mys_log_handler_fn)(mys_log_event_t *event, const char *fmt, va_list vargs, void *udata);

/////// log
__attribute__((format(printf, 5, 6))) MYS_PUBLIC void mys_log(int who, int level, const char *file, int line, const char *fmt, ...);
__attribute__((format(printf, 5, 6))) MYS_PUBLIC void mys_log_when(int cond, int level, const char *file, int line, const char *fmt, ...);
__attribute__((format(printf, 4, 5))) MYS_PUBLIC void mys_log_self(int level, const char *file, int line, const char *fmt, ...);
__attribute__((format(printf, 4, 5))) MYS_PUBLIC void mys_log_once(int level, const char *file, int line, const char *fmt, ...);
__attribute__((format(printf, 4, 5))) MYS_PUBLIC void mys_log_ordered(int level, const char *file, int line, const char *fmt, ...);
MYS_PUBLIC int mys_log_add_handler(mys_log_handler_fn handler_fn, void *handler_udata);
MYS_PUBLIC void mys_log_remove_handler(int handler_id);
MYS_PUBLIC void mys_log_invoke_handlers(mys_log_event_t *event, const char *fmt, va_list vargs);
MYS_PUBLIC int mys_log_get_level();
MYS_PUBLIC void mys_log_set_level(int level);
MYS_PUBLIC void mys_log_silent(bool silent);
MYS_PUBLIC const char* mys_log_level_string(int level);
MYS_PUBLIC void mys_log_init();
/////// rank log
__attribute__((format(printf, 4, 5)))
MYS_PUBLIC void mys_rank_log(const char *callfile, int callline, const char *folder, const char *fmt, ...);
MYS_PUBLIC void mys_rank_log_open(const char *callfile, int callline, const char *folder);
MYS_PUBLIC void mys_rank_log_close(const char *callfile, int callline, const char *folder);


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
