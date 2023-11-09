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
#include <alloca.h>
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

/**
 * @brief Initialize libmys' signal handlers for debugging
 */
MYS_API void mys_debug_init();
/**
 * @brief Finalize libmys' signal handlers and revert old handlers
 */
MYS_API void mys_debug_fini();
/**
 * @brief Get the (thread local) message for signal handlers to print.
 * @return Constructed last message
 */
MYS_API void mys_debug_get_message(char *buffer);
/**
 * @brief Set the (thread local) message for signal handlers to print.
 * 
 * @param fmt Formatter
 * @param ... Format arguments
 * 
 * @note This message length should not exceed `MYS_SIGNAL_LAST_MESSAGE_MAX`
 */
MYS_API void mys_debug_set_message(const char *fmt, ...);
MYS_API void mys_debug_clear_message();

MYS_API void mys_debug_set_style(int style);
MYS_API int mys_debug_get_style();

// To enable this functionality, you have to
// 1) Add `#define MYS_ENABLE_DEBUG_TIMEOUT` before `#include mys.h`
// 2) Add `-lrt` to compiler for using `timer_create()`, `timer_settime()`, and `timer_delete()`
MYS_API void mys_debug_set_timeout(double timeout);
MYS_API void mys_debug_clear_timeout();

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
