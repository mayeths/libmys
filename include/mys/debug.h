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
#include <signal.h>
#include <execinfo.h>


// TODO: 要是有一个 mys_debug_set_last_msg("Current iter %d level %d", iter, level)
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
 * @brief Set a (thread local) message for signal handlers to print.
 * 
 * @param fmt Formatter
 * @param ... Format arguments
 * @return Constructed last message
 * 
 * @note This message length should not exceed `MYS_SIGNAL_LAST_MESSAGE_MAX`
 */
MYS_API const char *mys_debug_last_message(const char *fmt, ...);



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
    mys_debug_last_message("HAHA %p", emit_signal);
    emit_signal(SIGSEGV);
    mys_debug_fini();
    // MPI_Finalize();
}

*/
