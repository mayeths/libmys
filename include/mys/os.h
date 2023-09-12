#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "_config.h"
#include "_lib/mpi.h"
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

typedef struct mys_prun_t {
    int retval;
    char *out;
    char *err;
} mys_prun_t;

typedef struct mys_popen_t {
    int pid;
    int ifd;
    int ofd;
    int efd;
} mys_popen_t;

/**
 * @brief Open a subprocess using system shell
 * 
 * @param argv The subprocess command line
 * @return The pid and stdin/stdout/stderr file descriptor
 */
MYS_API mys_popen_t mys_popen_create(const char *argv);
MYS_API int mys_popen_destroy(mys_popen_t *popend);
/**
 * @brief Run a subprocess using system shell,
 * capture the exit code and stdout/stderr messages.
 * 
 * @param argv The subprocess command line
 */
MYS_API mys_prun_t mys_prun_create(const char *argv);
/**
 * @brief Destroy the captured results from mys_prun_create.
 * 
 * @param s The subprocess command line
 */
MYS_API int mys_prun_destroy(mys_prun_t *pd);
/**
 * @brief
 * 
 * @param path 
 * @return MYS_API* 
 * 
 * @note Remember to free return value
 */
MYS_API void mys_bfilename(const char *path, char **basename);
/**
 * @brief 
 * 
 * @param path The path to do mkdir
 * @param mode The mkdir mode (commonly 0777)
 * 
 * @note https://stackoverflow.com/a/675193
 */
MYS_API int mys_do_mkdir(const char *path, mode_t mode);
/**
 * @brief Ensure all directories in path exist
 * 
 * @param path The path where all parent directories will be ensured to exist.
 * @param mode The mode to create parent directories (usually 0777).
 * 
 * @note mys_ensure_dir("/a/b/c/d/", 0777)
 * @note Algorithm takes the pessimistic view and works top-down to ensure
 *       each directory in path exists, rather than optimistically creating
 *       the last element and working backwards.
 */
MYS_API int mys_ensure_dir(const char *path, mode_t mode);
/**
 * @brief Ensure all directories in parent path exist
 * 
 * @param path The path where all parent directories will be ensured to exist.
 * @param mode The mode to create parent directories
 * 
 * @note mys_ensure_parent("/a/b/c/d/e.txt", 0777)
 * @note Algorithm takes the pessimistic view and works top-down to ensure
 *       each directory in path exists, rather than optimistically creating
 *       the last element and working backwards.
 */
MYS_API int mys_ensure_parent(const char *path, mode_t mode);
MYS_API int mys_busysleep(double seconds);
MYS_API const char *mys_procname();
MYS_API void mys_wait_flag(const char *file, int line, const char *flagfile);
#if defined(OS_LINUX)
MYS_API const char *mys_get_affinity();
MYS_API void mys_print_affinity(FILE *fd);
MYS_API void mys_stick_affinity();
#endif


////// Legacy
#if !defined(MYS_NO_LEGACY) && !defined(MYS_NO_LEGACY_OS)
typedef mys_popen_t popen_t;
typedef mys_prun_t prun_t;
MYS_API popen_t popen_create(const char *argv);
MYS_API prun_t prun_create(const char *argv);
MYS_API int prun_destroy(prun_t *pd);
MYS_API int busysleep(double sec);
MYS_API char *bfilename(const char *path);
MYS_API const char *procname();
MYS_API int do_mkdir(const char *path, mode_t mode);
MYS_API int ensuredir(const char *path, mode_t mode);
MYS_API int ensureparent(const char *path, mode_t mode);

#ifndef WAIT_FLAG
#define WAIT_FLAG(flagfile) mys_wait_flag(__FILE__, __LINE__, flagfile)
#else
#warning legacy WAIT_FLAG was predefined before libmys
#endif /*WAIT_FLAG*/

#endif /*MYS_NO_LEGACY*/

////// Internal

typedef struct _mys_hrtime_G_t {
    bool inited;
    union {
        uint64_t u;
        double d;
    } start;
} _mys_hrtime_G_t;

/////// check memory leak by valgrind
/* gcc pipe.c && valgrind --leak-check=full --track-fds=yes ./a.out
int main(int argc, char **argv)
{
    mys_prun_t prun = mys_prun_create("ls *");
    printf(">%s<%s<%d\n", prun.out, prun.err, prun.retval);
    mys_prun_destroy(&prun);
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
