#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "_config.h"
#include "mpi.h"
#include "macro.h"
#include "thread.h"

#if defined(POSIX_COMPLIANCE)
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <libgen.h>
#elif defined(OS_WINDOWS)
#include <windows.h>
#endif
#if defined(OS_LINUX)
#include <sched.h>
#endif


///////////////////////////////////
// Run and wait a command to finish
///////////////////////////////////

typedef struct mys_prun_t {
    bool success; // if calling subprocess success
    char *out;
    char *err;
    size_t len_out;
    size_t len_err;
    int retval;
    bool _alloced;
} mys_prun_t;

/**
 * @brief Run a subprocess using system shell,
 * capture the exit code and stdout/stderr messages.
 * 
 * @param command Subprocess command line
 * @param buf_out Buffer to hold stdout of sub-process
 * @param max_out Maximum bytes of `buf_out` can hold
 * @param buf_err Buffer to hold stderr of sub-process
 * @param max_err Maximum bytes of `buf_err` can hold
 * @return The `mys_prun_t` handler that containing exit code and associated data
 * 
 * @note This subroutine is async-signal-safe.
 * @note On Linux and MacOS, system shell is "/bin/sh".
 */
MYS_API mys_prun_t mys_prun_create(const char *command, char *buf_out, size_t max_out, char *buf_err, size_t max_err);
/**
 * @brief Run a subprocess using system shell,
 * capture the exit code and stdout/stderr messages in new allcoated buffer.
 * 
 * @param command Subprocess command line
 * @return The `mys_prun_t` handler that containing exit code and associated data
 * 
 * @note This subroutine is NOT async-signal-safe, due to this subroutine will allocate memory.
 */
MYS_API mys_prun_t mys_prun_create2(const char *command);
/**
 * @brief Destroy the handler from `mys_prun_create()` or `mys_prun_create2()`.
 * 
 * @param prun The handler
 * 
 * @note This subroutine is async-signal-safe depending on argument.
 * @note If argument `prun` is created by `mys_prun_create()`, then it is async-signal-safe.
 * @note If argument `prun` is created by `mys_prun_create2()`, then it is NOT async-signal-safe.
 */
MYS_API void mys_prun_destroy(mys_prun_t *prun);


///////////////////////////////////
// Open a pipe to subprocess
///////////////////////////////////

typedef struct mys_popen_t {
    bool alive; // if the corresponding subprocess still running
    int pid;
    int ifd;
    int ofd;
    int efd;
    int retval;
} mys_popen_t;

/**
 * @brief Open a subprocess using system shell
 * 
 * @param command The subprocess command
 * @return The `mys_popen_t` handler that containing subprocess pid
 * and stdin/stdout/stderr file descriptor
 * 
 * @note This subroutine is async-signal-safe.
 */
MYS_API mys_popen_t mys_popen_create(const char *command);
/**
 * @brief Test status of subprocess that associated with `popen`.
 * 
 * @param popen The handler
 * @return true if subprocess still alive, otherwise false
 * 
 * @note This subroutine is async-signal-safe.
 * @note Upon subprocess exited or failed, `popen->retval` is set to its exit code.
 * @note You still have to call `mys_popen_wait()` to clean up if test return false.
 */
MYS_API bool mys_popen_test(mys_popen_t *popen);
/**
 * @brief Wait subprocess that associated with `popen` to exit or fail.
 * 
 * @param popen The handler
 * @return true if waiting subprocess exited or failed (that is, has a exit code),
 * otherwise false to indicate wait error
 * 
 * @note This subroutine is async-signal-safe.
 * @note Upon subprocess exited or failed, `popen->retval` is set to its exit code.
 */
MYS_API bool mys_popen_wait(mys_popen_t *popen);
/**
 * @brief Kill subprocess that associated with `popen` immediately.
 * 
 * @param popen The handler
 * @return true if killing subprocess exited or failed (that is, has a exit code),
 * otherwise false to indicate kill error
 * 
 * @note This subroutine is async-signal-safe.
 * @note Upon subprocess is killed, `popen->retval` is set to its exit code.
 */
MYS_API bool mys_popen_kill(mys_popen_t *popen, int signo);



///////////////////////////////////
// File system related functions
///////////////////////////////////
/**
 * @brief Make directory
 * 
 * @param path The path to mkdir
 * @param mode The mkdir mode (usually 0777)
 * @return true if make successfully or directory already exist
 * 
 * @note https://stackoverflow.com/a/675193
 * @note Failed if one of the intermediate directories doesn't exist
 */
MYS_API bool mys_mkdir(const char *path, mode_t mode);
/**
 * @brief Make directory and all intermediate directories
 * 
 * @param path The path to mkdir
 * @param mode The mkdir mode (usually 0777)
 * @return true if make successfully or directory already exist
 * 
 * @note mys_ensure_dir("/a/b/c/d/", 0777)
 * @note Intermediate directories are created with permission bits of “rwxrwxrwx” (0777)
 */
MYS_API bool mys_ensure_dir(const char *path, mode_t mode);
/**
 * @brief Make parent directory and all intermediate directories
 * @brief Ensure all directories in parent path exist
 * 
 * @param path The path to mkdir its parent directory
 * @param mode The mkdir mode (usually 0777)
 * @return true if make successfully or directory already exist
 * 
 * @note mys_ensure_parent("/a/b/c/d/e.txt", 0777)
 * @note Intermediate directories are created with permission bits of “rwxrwxrwx” (0777)
 */
MYS_API bool mys_ensure_parent(const char *path, mode_t mode);
MYS_API int mys_busysleep(double seconds);
MYS_API const char *mys_procname();
MYS_API void mys_wait_flag(const char *file, int line, const char *flagfile);
#if defined(OS_LINUX)
MYS_API const char *mys_get_affinity();
MYS_API void mys_print_affinity(FILE *fd);
MYS_API void mys_stick_affinity();
#endif

////// Internal

typedef struct _mys_hrtime_G_t {
    bool inited;
    union {
        uint64_t u;
        double d;
    } start;
} _mys_hrtime_G_t;

/////// check memory leak by valgrind
/* gcc -I${MYS_DIR}/include -Wall -Wextra ./prun.c && valgrind --leak-check=full --track-fds=yes --track-fds=yes ./a.out
#define MYS_IMPL
#define MYS_NO_MPI
#include <mys.h>

int main() 
{
    char buf_out1[1024];
    char buf_out2[1024];
    mys_prun_t run1 = mys_prun_create("ls a.c", buf_out1, sizeof(buf_out1), NULL, 0);
    mys_prun_t run2 = mys_prun_create("ls asdf", buf_out2, sizeof(buf_out2), NULL, 0);
    printf("run1: |%s|\n", buf_out1);
    printf("run2: |%s|\n", buf_out2);
    mys_prun_destroy(&run1);
    mys_prun_destroy(&run2);
}
*/
/* gcc -g ensuredir.c && valgrind --leak-check=full ./a.out 444/123.txt
int main(int argc, char **argv)
{
    int             i;
    for (i = 1; i < argc; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            if (fork() == 0)
            {
                int rc = ensureparent(argv[i], 0777);
                if (rc != 0)
                    fprintf(stderr, "%d: failed to create (%d: %s): %s\n",
                            (int)getpid(), errno, strerror(errno), argv[i]);
                exit(rc == 0 ? 0 : -1);
            }
        }
        int status;
        int fail = 0;
        while (wait(&status) != -1)
        {
            if (WEXITSTATUS(status) != 0)
                fail = 1;
        }
        if (fail == 0)
            printf("created: %s\n", argv[i]);
    }
    return(0);
}
*/
