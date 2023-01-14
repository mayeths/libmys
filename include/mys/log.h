#pragma once

#define MYS_LOG_VERSION 2

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#include "config.h"
#include "thread.h"
#include "macro.h"
#include "myspi.h"


////// API

#ifdef MYS_LOG_LONG_FILENAME
#define __MYS_LOG_FNAME__ __FILE__
#else
#define __MYS_LOG_FNAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define TLOG(who, fmt, ...) __mys_log(who, MYS_LOG_TRACE, __MYS_LOG_FNAME__, __LINE__, fmt, ##__VA_ARGS__)
#define DLOG(who, fmt, ...) __mys_log(who, MYS_LOG_DEBUG, __MYS_LOG_FNAME__, __LINE__, fmt, ##__VA_ARGS__)
#define ILOG(who, fmt, ...) __mys_log(who, MYS_LOG_INFO,  __MYS_LOG_FNAME__, __LINE__, fmt, ##__VA_ARGS__)
#define WLOG(who, fmt, ...) __mys_log(who, MYS_LOG_WARN,  __MYS_LOG_FNAME__, __LINE__, fmt, ##__VA_ARGS__)
#define ELOG(who, fmt, ...) __mys_log(who, MYS_LOG_ERROR, __MYS_LOG_FNAME__, __LINE__, fmt, ##__VA_ARGS__)
#define FLOG(who, fmt, ...) __mys_log(who, MYS_LOG_FATAL, __MYS_LOG_FNAME__, __LINE__, fmt, ##__VA_ARGS__)

typedef enum {
    MYS_LOG_TRACE, MYS_LOG_DEBUG, MYS_LOG_INFO, MYS_LOG_WARN, MYS_LOG_ERROR, MYS_LOG_FATAL, MYS_LOG_LEVEL_COUNT
} mys_log_level_t;

typedef struct {
    mys_log_level_t level;
    const char *file;
    int line;
    const char *fmt;
    va_list vargs;
    void *udata;
} mys_log_event_t;

typedef void (*mys_log_handler_fn)(const mys_log_event_t *event);

MYS_API static int mys_log_add_handler(mys_log_handler_fn handler_fn, void *handler_udata);
MYS_API static void mys_log_remove_handler(int handler_id);
MYS_API static void mys_log_set_level(mys_log_level_t level);
MYS_API static void mys_log_set_quiet(bool quiet);


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
    mys_log_level_t level;
    mys_log_level_t last_level;
    mys_mutex_t lock;
    int id_generator;
    struct {
        mys_log_handler_fn fn;
        void *udata;
        int id;
    } handlers[128];
} mys_log_G_t;

extern mys_log_G_t mys_log_G;

__attribute__((format(printf, 5, 6)))
MYS_API static void __mys_log(int who, mys_log_level_t level, const char *file, int line, const char *fmt, ...)
{
    mys_mutex_lock(&mys_log_G.lock);
    int myrank = __mys_myrank();
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
    int id = mys_log_G.id_generator;
    mys_log_G.id_generator += 1;
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

MYS_API static void mys_log_set_level(mys_log_level_t level)
{
    mys_mutex_lock(&mys_log_G.lock);
    mys_log_G.last_level = mys_log_G.level;
    mys_log_G.level = level;
    mys_mutex_unlock(&mys_log_G.lock);
}

MYS_API static void mys_log_set_quiet(bool quiet)
{
    mys_mutex_lock(&mys_log_G.lock);
    if (mys_log_G.level == MYS_LOG_LEVEL_COUNT)
        return;
    const mys_log_level_t last = mys_log_G.last_level;
    mys_log_G.last_level = mys_log_G.level;
    mys_log_G.level = quiet ? MYS_LOG_LEVEL_COUNT : last;
    mys_mutex_unlock(&mys_log_G.lock);
}

MYS_API static const char* mys_log_level_string(mys_log_level_t level)
{
    const char *level_strings[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
    };
    return level_strings[(int)level];
}

MYS_API static void __mys_stdout_handler(const mys_log_event_t *event) {
    const char *lstr = mys_log_level_string(event->level);
    const char level_shortname = lstr[0];

    char base_label[256];

    char *label = base_label;
    int label_size = sizeof(base_label);

    int myrank = __mys_myrank();
    int nranks = __mys_nranks();
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
    if (isatty(fileno(stdout))) {
        snprintf(colorized_label, sizeof(colorized_label), "%s%s\x1b[0m",
            level_colors[(int)event->level], label
        );
        label = colorized_label;
        label_size = sizeof(colorized_label);
    }
#endif

    fprintf(stdout, "%s", label);
    vfprintf(stdout, event->fmt, event->vargs);
    fprintf(stdout, "\n");
    fflush(stdout);
}

#if 0

/** https://github.com/rxi/log.c
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#define LOG_VERSION "0.1.0"

/* https://stackoverflow.com/a/38237385 */
#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
// #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)



typedef struct {
  va_list ap;
  const char *fmt;
  const char *file;
  struct tm *time;
  void *udata;
  int line;
  int level;
} log_Event;

typedef void (*log_LogFn)(log_Event *ev);
typedef void (*log_LockFn)(bool lock, void *udata);

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define log_trace(...) log_log(LOG_TRACE, __FILENAME__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILENAME__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  __FILENAME__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  __FILENAME__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILENAME__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILENAME__, __LINE__, __VA_ARGS__)

static const char* log_level_string(int level);
static void log_set_lock(log_LockFn fn, void *udata);
static void log_set_level(int level);
static void log_set_quiet(bool enable);
static int log_add_callback(log_LogFn fn, void *udata, int level);
static int log_add_fp(FILE *fp, int level);

static void log_log(int level, const char *file, int line, const char *fmt, ...);

/*
 * Copyright (c) 2020 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */


#define MAX_CALLBACKS 128

typedef struct {
  log_LogFn fn;
  void *udata;
  int level;
} Callback;

static struct {
  void *udata;
  log_LockFn lock;
  int level;
  bool quiet;
  Callback callbacks[MAX_CALLBACKS];
} L;

static const char *level_strings[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};


static void stdout_callback(log_Event *ev) {
  char buf[16];
  buf[strftime(buf, sizeof(buf), "%H:%M:%S", ev->time)] = '\0';
    if (isatty(fileno(stdout))) {
        fprintf(
            (FILE *)ev->udata, "%s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
            level_colors[ev->level], level_strings[ev->level],
            ev->file, ev->line);
    }
  vfprintf((FILE *)ev->udata, ev->fmt, ev->ap);
  fprintf((FILE *)ev->udata, "\n");
  fflush((FILE *)ev->udata);
}


static void file_callback(log_Event *ev) {
  char buf[64];
  buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
  fprintf(
    (FILE *)ev->udata, "%s %-5s %s:%d: ",
    buf, level_strings[ev->level], ev->file, ev->line);
  vfprintf((FILE *)ev->udata, ev->fmt, ev->ap);
  fprintf((FILE *)ev->udata, "\n");
  fflush((FILE *)ev->udata);
}


static void lock(void)   {
  if (L.lock) { L.lock(true, L.udata); }
}


static void unlock(void) {
  if (L.lock) { L.lock(false, L.udata); }
}


static const char* log_level_string(int level) {
  return level_strings[level];
}


static void log_set_lock(log_LockFn fn, void *udata) {
  L.lock = fn;
  L.udata = udata;
}


static void log_set_level(int level) {
  L.level = level;
}


static void log_set_quiet(bool enable) {
  L.quiet = enable;
}


static int log_add_callback(log_LogFn fn, void *udata, int level) {
  for (int i = 0; i < MAX_CALLBACKS; i++) {
    if (!L.callbacks[i].fn) {
      L.callbacks[i] = (Callback) { fn, udata, level };
      return 0;
    }
  }
  return -1;
}


static int log_add_fp(FILE *fp, int level) {
  return log_add_callback(file_callback, fp, level);
}


static void init_event(log_Event *ev, void *udata) {
  if (!ev->time) {
    time_t t = time(NULL);
    ev->time = localtime(&t);
  }
  ev->udata = udata;
}


static void log_log(int level, const char *file, int line, const char *fmt, ...) {
  log_Event ev = {
    .fmt   = fmt,
    .file  = file,
    .line  = line,
    .level = level,
  };

  lock();

  if (!L.quiet && level >= L.level) {
    init_event(&ev, stderr);
    va_start(ev.ap, fmt);
    stdout_callback(&ev);
    va_end(ev.ap);
  }

  for (int i = 0; i < MAX_CALLBACKS && L.callbacks[i].fn; i++) {
    Callback *cb = &L.callbacks[i];
    if (level >= cb->level) {
      init_event(&ev, cb->udata);
      va_start(ev.ap, fmt);
      cb->fn(&ev);
      va_end(ev.ap);
    }
  }

  unlock();
}

#endif
#endif
