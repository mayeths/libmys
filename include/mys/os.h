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
#elif defined(OS_WINDOWS)
#include <windows.h>
#endif

static inline int busysleep(double sec)
{
#if defined(POSIX_COMPLIANCE)
    /* https://stackoverflow.com/a/8158862 */
    struct timespec req;
    req.tv_sec = (uint64_t)sec;
    req.tv_nsec = (sec - req.tv_sec) * 1000000000;
    do {
        if(nanosleep(&req, &req) == 0)
            break;
        else if(errno != EINTR)
            return -1;
    } while (req.tv_sec > 0 || req.tv_nsec > 0);
    return 0;
#elif defined(OS_WINDOWS)
    /* https://stackoverflow.com/a/5801863 */
    /* https://stackoverflow.com/a/26945754 */
    LARGE_INTEGER fr,t1,t2;
    if (!QueryPerformanceFrequency(&fr))
        return -1;
    if (!QueryPerformanceCounter(&t1))
        return -1;
    do {
        if (!QueryPerformanceCounter(&t2))
            return -1;
    } while(((double)t2.QuadPart-(double)t1.QuadPart)/(double)fr.QuadPart < sec);
    return 0;
#else
    return -2;
#endif
}

static inline char* execshell(const char* command) {
    FILE* fp;
    char* line = NULL;
    char* result = (char*) calloc(1, 1);
    size_t len = 0;

    fflush(NULL);
    fp = popen(command, "r");
    if (fp == NULL) {
        printf("Cannot execute command:\n%s\n", command);
        return NULL;
    }

    while(getline(&line, &len, fp) != -1) {
        size_t lenline = strlen(line);
        size_t lenres = strlen(result);
        result = (char*) realloc(result, lenline + lenres + 1);
        strncpy(result + lenres, line, lenline + 1);
        free(line);
        line = NULL;
    }

    fflush(fp);
    if (pclose(fp) != 0) {
        perror("Cannot close stream.\n");
    }
    return result;
}


/* Thanks to http://www.jukie.net/bart/blog/popenRWE */
/* https://github.com/sni/mod_gearman/blob/master/common/popenRWE.c */
/* https://github.com/marssaxman/ozette/blob/833b659757/src/console/popenRWE.cpp */

/* gcc pipe.c && valgrind --leak-check=full --track-fds=yes ./a.out
int main() {
    int pipeIOE[3];
    char *argv = "echo 123";
    prun_t result = prun_create(argv);
    printf("status:%d\n", result.status);
    printf("stdout:\n%s", result.out);
    printf("stderr:\n%s", result.err);
    prun_destroy(&result);
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
}
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

static inline int is_valid_fd(int fd) {
    return fcntl(fd, F_GETFL) != -1 || errno != EBADF;
}

static int popenRWE(int *ipipe, int *opipe, int *epipe, const char *command) {
    int in[2] = {-1, -1};
    int out[2] = {-1, -1};
    int err[2] = {-1, -1};
    int pid = -1;
    int rc = 0;

    rc = pipe(in);
    if (rc<0)
        goto error_in;
    rc = pipe(out);
    if (rc<0)
        goto error_out;
    rc = pipe(err);
    if (rc<0)
        goto error_err;

    pid = fork();
    if (pid > 0) { /* parent */
        if (is_valid_fd(in[0])) close(in[0]);
        if (is_valid_fd(out[1])) close(out[1]);
        if (is_valid_fd(err[1])) close(err[1]);
        *ipipe = in[1];
        *opipe = out[0];
        *epipe = err[0];
        return pid;
    } else if (pid == 0) { /* child */
        if (is_valid_fd(in[1])) close(in[1]);
        if (is_valid_fd(out[0])) close(out[0]);
        if (is_valid_fd(err[0])) close(err[0]);
        if (is_valid_fd(0)) close(0);
        if(!dup(in[0])) {
            ;
        }
        if (is_valid_fd(1)) close(1);
        if(!dup(out[1])) {
            ;
        }
        if (is_valid_fd(2)) close(2);
        if(!dup(err[1])) {
            ;
        }
        execl( "/bin/sh", "sh", "-c", command, NULL );
        _exit(1);
    } else
        goto error_fork;

    return pid;

error_fork:
    if (is_valid_fd(err[0])) close(err[0]);
    if (is_valid_fd(err[1])) close(err[1]);
error_err:
    if (is_valid_fd(out[0])) close(out[0]);
    if (is_valid_fd(out[1])) close(out[1]);
error_out:
    if (is_valid_fd(in[0])) close(in[0]);
    if (is_valid_fd(in[1])) close(in[1]);
error_in:
    return -1;
}

static int pcloseRWE(int pid, int ipipe, int opipe, int epipe)
{
    if (is_valid_fd(ipipe)) close(ipipe);
    if (is_valid_fd(opipe)) close(opipe);
    if (is_valid_fd(epipe)) close(epipe);
    int status;
    waitpid(pid, &status, 0);
    return status;
}

#define BUFFER_SIZE 128

// static int readfd(char **target, FILE* input) {
//     char buffer[BUFFER_SIZE] = "";
//     int bytes, size, total;
//     strcpy(buffer,"");
//     size  = BUFFER_SIZE;
//     total = size;
//     while(fgets(buffer,sizeof(buffer)-1, input)){
//         bytes = strlen(buffer);
//         if(total < bytes + size) {
//             *target = realloc(*target, total+BUFFER_SIZE);
//             total += BUFFER_SIZE;
//         }
//         size += bytes;
//         strncat(*target, buffer, bytes);
//     }
//     return(size);
// }

static int readfd(char **target, FILE* input) {
    char buffer[BUFFER_SIZE];
    strcpy(buffer, "");
    int capacity = BUFFER_SIZE;
    int size = 0;
    while (fgets(buffer,sizeof(buffer)-1, input)) {
        int append = strlen(buffer);
        if (capacity < size + append) {
            *target = (char *)realloc(*target, capacity + BUFFER_SIZE);
            capacity += BUFFER_SIZE;
        }
        strncat(*target, buffer, capacity - size);
        size += append;
        // printf("read %d, size %d, cap %d\n", append, size, capacity);
    }
    return(size);
}


/* extract check result */
static char *extract_check_result(FILE *fp) {
    char *output;
    output = (char *)malloc(sizeof(char*)*BUFFER_SIZE);
    output[0] = '\0';
    readfd(&output, fp);
    return output;
}

typedef struct popen_t {
    int pid;
    int ifd;
    int ofd;
    int efd;
} popen_t;

typedef struct prun_t {
    int status;
    const char *out;
    const char *err;
} prun_t;

static const char *__empty_string = "";
static popen_t popen_create(const char *argv)
{
    popen_t result;
    result.ifd = -1;
    result.ofd = -1;
    result.efd = -1;
    result.pid = popenRWE(&result.ifd, &result.ofd, &result.efd, argv);
    return result;
}

MYS_API static prun_t prun_create(const char *argv)
{
    prun_t result;
    result.status = -1;
    result.out = __empty_string;
    result.err = __empty_string;

    popen_t pf = popen_create(argv);

    FILE *outfp = fdopen(pf.ofd,"r");
    if (outfp) {
        result.out = extract_check_result(outfp);
        fclose(outfp);
    }
    FILE *errfp = fdopen(pf.efd,"r");
    if (errfp) {
        result.err = extract_check_result(errfp);
        fclose(errfp);
    }
    result.status = pcloseRWE(pf.pid, pf.ifd, pf.ofd, pf.efd);
    return result;
}

MYS_API static void prun_destroy(prun_t *s)
{
    if (s->out != NULL && s->out != __empty_string)
        free((char *)s->out);
    if (s->err != NULL && s->err != __empty_string)
        free((char *)s->err);
    s->out = NULL;
    s->err = NULL;
    s->status = -1;
}

/* Remember to free return value. */
static inline char *bfilename(const char *path)
{
    const char *s = strrchr(path, '/');
    return s ? strdup(s + 1) : strdup(path);
}

static inline const char *procname()
{
    static char name[1024] = {'\0'};
    if (name[0] != '\0')
        return name;

    int pid = (int)getpid();
    char exe[1024];
    snprintf(exe, sizeof(exe), "/proc/%d/exe", pid);

    char path[1024];
    ssize_t n = readlink(exe, path, sizeof(path));
    if (n > 0 && (size_t)n < (sizeof(path) - 1)) {
        path[n] = '\0';
        char *bname = bfilename(path);
        size_t size = strnlen(bname, sizeof(name));
        strncpy(name, bname, size);
        free(bname);
    } else {
        snprintf(name, sizeof(name), "<error_exe.pid=%d>", pid);
    }

    // prun_t run = prun_create(path);
    // if (run.status == 0 && strlen(run.out) >= 1) {
    //     char *bname = bfilename(run.out);
    //     size_t size = strnlen(bname, sizeof(name));
    //     strncpy(name, bname, size);
    //     free(bname);
    // } else {
    //     snprintf(name, sizeof(name), "<error_exe.pid=%d>", pid);
    // }
    // prun_destroy(&run);
    return name;
}



/* https://stackoverflow.com/a/675193 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct stat Stat;

static int do_mkdir(const char *path, mode_t mode)
{
    Stat st;
    int  status = 0;
    if (stat(path, &st) != 0) {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0 && errno != EEXIST)
            status = -1;
    } else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        status = -1;
    }
    return status;
}

/**
** ensuredir - ensure all directories in path exist
** Algorithm takes the pessimistic view and works top-down to ensure
** each directory in path exists, rather than optimistically creating
** the last element and working backwards.
** ensuredir("/a/b/c/d/", 0777)
*/
static int ensuredir(const char *path, mode_t mode)
{
    char *p = strdup(path);
    char *pp = p;
    char *sp = NULL;
    int status = 0;
    while (status == 0 && (sp = strchr(pp, '/')) != 0) {
        if (sp != pp) {
            *sp = '\0';
            status = do_mkdir(p, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (status == 0)
        status = do_mkdir(path, mode);
    free(p);
    return status;
}

MYS_API static int ensureparent(const char *path, mode_t mode)
{
    char *dirc = strdup(path);
    // char *dname = dirname(dirc);
    // FIXME: why dname is not used?
    int status = ensuredir(dirc, mode);
    free(dirc);
    return status;
}

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
