#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "config.h"
#include "macro.h"

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
 * @param mode The mode to create parent directories.
 * 
 * @note
 * Algorithm takes the pessimistic view and works top-down to ensure
 * each directory in path exists, rather than optimistically creating
 * the last element and working backwards.
 * mys_ensure_dir("/a/b/c/d/", 0777)
 */
MYS_API int mys_ensure_dir(const char *path, mode_t mode);
/**
 * @brief Ensure all directories in parent path exist
 * 
 * @param path The path where all parent directories will be ensured to exist.
 * @param mode The mode to create parent directories
 */
MYS_API int mys_ensure_parent(const char *path, mode_t mode);
MYS_API int mys_busysleep(double seconds);
MYS_API const char *mys_procname();


////// Legacy

typedef mys_popen_t popen_t;
typedef mys_prun_t prun_t;
MYS_API static popen_t popen_create(const char *argv) { return mys_popen_create(argv); }
MYS_API static prun_t prun_create(const char *argv) { return mys_prun_create(argv); }
MYS_API static int prun_destroy(prun_t *pd) { return mys_prun_destroy(pd); }
MYS_API static int busysleep(double sec) { return mys_busysleep(sec); }
MYS_API static char *bfilename(const char *path) { char *r; mys_bfilename(path, &r); return r; }
MYS_API static const char *procname() { return mys_procname(); }
MYS_API static int do_mkdir(const char *path, mode_t mode) { return mys_do_mkdir(path, mode); }
MYS_API static int ensuredir(const char *path, mode_t mode) { return mys_ensure_dir(path, mode); }
MYS_API static int ensureparent(const char *path, mode_t mode) { return mys_ensure_parent(path, mode); }

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
