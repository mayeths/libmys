#pragma once

// https://github.com/openucx/ucx/blob/da0fdd7d0bd6d4592cd26f61ce712a4a77073fa0/src/ucs/debug/debug.c#L1080

#include "_config.h"
#include "os.h"

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
#include <sys/epoll.h>
#include <signal.h>
#include <execinfo.h>


#define MYS_SIGNAL_LAST_MESSAGE_MAX 1024

/**
 * @brief Initialize libmys' signal handlers for debugging
 */
MYS_API void mys_debug_init();
/**
 * @brief Finalize libmys' signal handlers and revert old handlers
 */
MYS_API void mys_debug_fini();
/**
 * @brief Set the (thread local) message for signal handlers to print.
 * 
 * @param fmt Formatter
 * @param ... Format arguments
 * 
 * @note This message length should not exceed `MYS_SIGNAL_LAST_MESSAGE_MAX`
 */
MYS_API void mys_debug_set_message(const char *fmt, ...);
/**
 * @brief Get the (thread local) message for signal handlers to print.
 * @return Constructed last message
 */
MYS_API const char *mys_debug_get_message();
MYS_API void mys_debug_clear_message();

MYS_API void mys_debug_set_style(int style);
MYS_API int mys_debug_get_style();

MYS_API void mys_debug_set_kill_timer(double timeout);
MYS_API void mys_debug_clear_kill_timer();

/* gcc -Wall -Wextra -I${MYS_DIR}/include -g test-debug.c && valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./a.out
=======================
#define MYS_IMPL
#define MYS_NO_MPI
#include <mys.h>

void emit_signal(int signal)
{
    if (signal == SIGSEGV) {
        int *a = NULL;
        *a = 100;
    } else {
        kill(getpid(), signal);
    }
}
int main() {
    // MPI_Init(NULL, NULL);
    mys_debug_init();
    mys_debug_set_message("HAHA %p", emit_signal);
    emit_signal(SIGSEGV);
    mys_debug_fini();
    // MPI_Finalize();
}

*/
