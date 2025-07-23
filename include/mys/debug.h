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

// https://github.com/openucx/ucx/blob/da0fdd7d0bd6d4592cd26f61ce712a4a77073fa0/src/ucs/debug/debug.c#L1080

#include "_config.h"
#include "os.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <signal.h>
#include <execinfo.h>


#define MYS_DEBUG_MESSAGE_MAX 1024

#define MYS_DEBUG_ACTION_NONE   0 // no post action
#define MYS_DEBUG_ACTION_EXIT   1 // directly exit program
#define MYS_DEBUG_ACTION_RAISE  2 // re-rase signo to old handler
#define MYS_DEBUG_ACTION_FREEZE 3 // freeze program by while(1) loop

/**
 * @brief Initialize libmys' signal handlers for debugging
 */
MYS_PUBLIC void mys_debug_init();
/**
 * @brief Finalize libmys' signal handlers and revert old handlers
 */
MYS_PUBLIC void mys_debug_fini();
/**
 * @brief Get the (thread local) message for signal handlers to print.
 * @return Constructed last message
 */
MYS_PUBLIC void mys_debug_get_message(char *buffer);
/**
 * @brief Enable catch of signal.
 * 
 * @param signo Signal number
 */
MYS_PUBLIC int mys_debug_set_signal(int signo);
MYS_PUBLIC int mys_debug_clear_signal(int signo);
/**
 * @brief Set the (thread local) message for signal handlers to print.
 * 
 * @param fmt Formatter
 * @param ... Format arguments
 * 
 * @note This message length should not exceed `MYS_SIGNAL_LAST_MESSAGE_MAX`
 */
MYS_PUBLIC void mys_debug_set_message(const char *fmt, ...);
MYS_PUBLIC void mys_debug_clear_message();

// To enable this functionality, you have to
// 1) Add `#define MYS_ENABLE_DEBUG_TIMEOUT` before `#include mys.h`
// 2) Add `-lrt` to compiler for using `timer_create()`, `timer_settime()`, and `timer_delete()`
MYS_PUBLIC void _mys_debug_set_timeout(double timeout, const char *file, int line);
MYS_PUBLIC void _mys_debug_set_timeout_env(const char *env_name, const char *file, int line);
MYS_PUBLIC void mys_debug_clear_timeout();
#define mys_debug_set_timeout(timeout_sec) _mys_debug_set_timeout(timeout_sec, __FILE__, __LINE__)
#define mys_debug_set_timeout_env(env_name) _mys_debug_set_timeout_env(env_name, __FILE__, __LINE__)


/**
 * @brief Set the maximum number of stack frames to be printed in a backtrace.
 *
 * @param max_frames Maximum number of stack frames
 */
MYS_PUBLIC void mys_debug_set_max_frames(int max_frames);
/**
 * @brief Get the maximum number of stack frames to be printed in a backtrace.
 *
 * @return Maximum number of stack frames
 */
MYS_PUBLIC int mys_debug_get_max_frames();

/**
 * @brief Add a filter string to exclude matching stack frames from the backtrace.
 *
 * @param match_str Filter string to match stack frames
 */
MYS_PUBLIC void mys_debug_add_stack_filter(const char *match_str);
/**
 * @brief Remove a filter string that excludes matching stack frames from the backtrace.
 *
 * @param match_str Filter string to be removed
 */
MYS_PUBLIC void mys_debug_del_stack_filter(const char *match_str);





#ifdef OS_MACOS
#include <dispatch/dispatch.h>
#include <mach/boolean.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <time.h>
struct itimerspec {
    struct timespec it_interval; /* timer period */
    struct timespec it_value;    /* timer expiration */
};
struct sigevent;
/* If used a lot, queue should probably be outside of this struct */
struct macos_timer {
    dispatch_queue_t tim_queue;
    dispatch_source_t tim_timer;
    void (*tim_func)(union sigval);
    void *tim_arg;
};
typedef struct macos_timer *timer_t;
MYS_STATIC void _timer_cancel(void *arg);
MYS_STATIC void _timer_handler(void *arg);
MYS_STATIC int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid);
MYS_STATIC int timer_settime(timer_t tim, int flags, const struct itimerspec *its, struct itimerspec *remainvalue);
MYS_STATIC int timer_delete(timer_t tim);
#endif

/* gcc -rdynamic -funwind-tables -I${MYS_DIR}/include -g -Wall -Wextra test-debug.c -lrt && valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./a.out
=======================
#define MYS_IMPL
#define MYS_NO_MPI
#define MYS_ENABLE_DEBUG_TIMEOUT
#include <mys.h>

void emit_signal(int signal)
{
    if (signal == SIGSEGV) {
        int *a = (int *)0x1234;
        *a = 100;
    } else {
        kill(getpid(), signal);
    }
}
int main() {
    // MPI_Init(NULL, NULL);
    mys_debug_init();
    double t_start = mys_hrtime();
    mys_debug_add_stack_filter("/usr/lib64/libc.so.6");
    mys_debug_add_stack_filter(":?");
    // mys_debug_del_stack_filter("/usr/lib64/libc.so.6");
    // mys_debug_del_stack_filter(":?");
    mys_debug_set_max_frames(3);
    mys_debug_set_message("Tstart %.9f", t_start);
    mys_debug_set_timeout(3);
    sleep(1);
    // mys_debug_clear_timeout();
    sleep(5);
    // emit_signal(SIGSEGV);
    // emit_signal(SIGABRT);
    mys_debug_fini();
    ILOG(0, "HAHAHAH");
    // MPI_Finalize();
}

*/
