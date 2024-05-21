#include "_private.h"
#include "../os.h"

#ifdef MYS_ENABLE_NUMA
#include <numa.h>
#endif

// extern int kill(pid_t pid, int sig) __THROW;
// extern char *strdup(const char *s) __THROW;
// extern ssize_t readlink(const char *path, char *buf, size_t bufsize) __THROW;


static size_t _mys_readfd(char **buffer, size_t buffer_size, int fd, bool enable_realloc)
{
    size_t read_size = 0;

    if (*buffer == NULL) {
        if (!enable_realloc)
            goto finished;
        else {
            if (buffer_size == 0)
                buffer_size = 64;
            *buffer = (char *)realloc(NULL, buffer_size);
            if (*buffer == NULL)
                goto finished;
        }
    }

    while (1)
    {
        size_t remaining = buffer_size - read_size;
        if (remaining == 0) {
            if (!enable_realloc)
                goto finished;
            else {
                // read() will fail if the parameter nbyte exceeds INT_MAX,
                // and do not attempt a partial read.
                size_t threshold = 4096; /* size_t threshold = INT_MAX; */
                if (buffer_size * 2 < threshold)
                    buffer_size *= 2;
                else
                    buffer_size += threshold;
                *buffer = (char *)realloc(*buffer, buffer_size);
                if (*buffer == NULL)
                    goto finished;
            }
        }

        ssize_t bytes_read = read(fd, *buffer + read_size, remaining);
        if (bytes_read < 0) // an error occurred
            goto finished;
        read_size += (size_t)bytes_read;
        if ((size_t)bytes_read < remaining) // EOF reached
            goto finished;
    }

finished:
    return read_size;
}

static size_t _mys_cut_suffix_space(char *buf, size_t len)
{
    if (buf == NULL)
        return 0;

    while (len != 0 && buf[len - 1] == '\n')
        len -= 1;
    buf[len] = '\0';
    return len;
}

/**
 * @brief Create stdin/stdout/stderr pipe to subprocess opened with command
 * @note
 * Thanks to http://www.jukie.net/bart/blog/popenRWE
 * https://github.com/sni/mod_gearman/blob/master/common/popenRWE.c
 * https://github.com/marssaxman/ozette/blob/833b659757/src/console/popenRWE.cpp
 */
MYS_API mys_popen_t mys_popen_create(const char *command)
{
    mys_popen_t popen;
    popen.alive = false;
    popen.pid = -1;
    popen.ifd = -1;
    popen.ofd = -1;
    popen.efd = -1;
    popen.retval = -1;

    int in[2]  = {-1, -1};
    int out[2] = {-1, -1};
    int err[2] = {-1, -1};
    if (pipe(in)  == -1) goto finished_0;
    if (pipe(out) == -1) goto finished_1;
    if (pipe(err) == -1) goto finished_2;

    popen.pid = fork();
    
    if (popen.pid == 0) { // child
        close(in[1]);  close(0); dup(in[0]);  // pipe to stdin
        close(out[0]); close(1); dup(out[1]); // pipe to stdout
        close(err[0]); close(2); dup(err[1]); // pipe to stderr
        execl("/bin/sh", "sh", "-c", command, NULL);
        _exit(1);
    } else { // parent
        if (popen.pid == -1)
            goto finished_3;
        else {
            close(in[0]);  popen.ifd = in[1];
            close(out[1]); popen.ofd = out[0];
            close(err[1]); popen.efd = err[0];
            popen.alive = true;
            goto finished_0;
        }
    }

finished_3:
    close(err[0]);
    close(err[1]);
finished_2:
    close(out[0]);
    close(out[1]);
finished_1:
    close(in[0]);
    close(in[1]);
finished_0:
    return popen;
}

MYS_API bool mys_popen_test(mys_popen_t *popen)
{
    if (popen == NULL || !popen->alive)
        return false;
    int status;
    pid_t wpid = waitpid(popen->pid, &status, WNOHANG);
    if (wpid == 0)
        return true;
    else if (wpid == popen->pid) {
        popen->retval = WEXITSTATUS(status);
        popen->alive = false;
        return false;
    } else {
        popen->alive = false;
        return false;
    }
}

MYS_API bool mys_popen_wait(mys_popen_t *popen)
{
    if (popen == NULL || !popen->alive)
        return false;
    close(popen->ifd); popen->ifd = -1;
    close(popen->ofd); popen->ofd = -1;
    close(popen->efd); popen->efd = -1;
    int status;
    pid_t wpid = waitpid(popen->pid, &status, 0);
    if (wpid == popen->pid) {
        popen->retval = WEXITSTATUS(status);
        popen->alive = false;
        return true;
    } else {
        popen->retval = -1;
        popen->alive = false;
        return false;
    }
}

MYS_API bool mys_popen_kill(mys_popen_t *popen, int signo)
{
    if (popen == NULL || !popen->alive)
        return false;
    kill(popen->pid, signo);
    close(popen->ifd); popen->ifd = -1;
    close(popen->ofd); popen->ofd = -1;
    close(popen->efd); popen->efd = -1;
    int status;
    pid_t wpid = waitpid(popen->pid, &status, 0);
    if (wpid == popen->pid) {
        popen->retval = WEXITSTATUS(status);
        popen->alive = false;
        return true;
    } else {
        popen->retval = -1;
        popen->alive = false;
        return false;
    }
}

//// prun

MYS_API mys_prun_t mys_prun_create(const char *command, char *buf_out, size_t max_out, char *buf_err, size_t max_err)
{
    mys_prun_t prun;
    prun.success = false;
    prun.out = buf_out;
    prun.err = buf_err;
    prun.len_out = 0;
    prun.len_err = 0;
    prun.retval = -1;
    prun._alloced = false;

    mys_popen_t popen = mys_popen_create(command);
    if (!popen.alive)
        return prun;

    prun.len_out = _mys_readfd(&prun.out, max_out, popen.ofd, prun._alloced);
    prun.len_err = _mys_readfd(&prun.err, max_err, popen.efd, prun._alloced);
    prun.len_out = _mys_cut_suffix_space(prun.out, prun.len_out);
    prun.len_err = _mys_cut_suffix_space(prun.err, prun.len_err);
    mys_popen_wait(&popen);
    prun.success = true;
    prun.retval = popen.retval;
    return prun;
}

MYS_API mys_prun_t mys_prun_create2(const char *command, ...)
{
    char *command_real = NULL;
    int needed = 0;
    mys_prun_t prun;
    prun.success = false;
    prun.out = NULL;
    prun.err = NULL;
    prun.len_out = 0;
    prun.len_err = 0;
    prun.retval = -1;
    prun._alloced = true;

    va_list vargs, vargs_test;
    va_start(vargs, command);
    va_copy(vargs_test, vargs);
    needed = vsnprintf(NULL, 0, command, vargs_test) + 1;
    command_real = (char *)malloc(needed);
    vsnprintf(command_real, needed, command, vargs);
    va_end(vargs);

    mys_popen_t popen = mys_popen_create(command_real);
    free(command_real);
    if (!popen.alive)
        return prun;

    prun.len_out = _mys_readfd(&prun.out, 0, popen.ofd, prun._alloced);
    prun.len_err = _mys_readfd(&prun.err, 0, popen.efd, prun._alloced);
    prun.len_out = _mys_cut_suffix_space(prun.out, prun.len_out);
    prun.len_err = _mys_cut_suffix_space(prun.err, prun.len_err);
    mys_popen_wait(&popen);
    prun.success = true;
    prun.retval = popen.retval;
    return prun;
}

MYS_API void mys_prun_destroy(mys_prun_t *prun)
{
    if (prun == NULL || !prun->success)
        return;
    if (prun->_alloced) {
        if (prun->out != NULL) free(prun->out);
        if (prun->err != NULL) free(prun->err);
    }
    prun->out = NULL;
    prun->err = NULL;
    prun->len_out = 0;
    prun->len_err = 0;
}


MYS_API bool mys_mkdir(const char *path, mode_t mode)
{
    struct stat st;
    bool success = true;
    if (stat(path, &st) != 0) {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0 && errno != EEXIST)
            success = false;
    } else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        success = false;
    }
    return success;
}

MYS_API bool mys_ensure_dir(const char *path, mode_t mode)
{
    char *p = strdup(path);
    char *pp = p;
    char *sp = NULL;
    bool success = true;
    while (success && (sp = strchr(pp, '/')) != 0) {
        if (sp != pp) {
            *sp = '\0';
            success = mys_mkdir(p, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (success)
        success = mys_mkdir(path, mode);
    free(p);
    return success;
}

MYS_API bool mys_ensure_parent(const char *path, mode_t mode)
{
    char *pathcopy = strdup(path);
    char *dname = dirname(pathcopy);
    bool success = mys_ensure_dir(dname, mode);
    free(pathcopy);
    return success;
}

// MYS_API int mys_busysleep(double seconds)
// {
// #if defined(POSIX_COMPLIANCE)
//     /* https://stackoverflow.com/a/8158862 */
//     struct timespec req;
//     req.tv_sec = (uint64_t)seconds;
//     req.tv_nsec = (uint64_t)((seconds - (double)req.tv_sec) * 1000000000);
//     do {
//         if(nanosleep(&req, &req) == 0)
//             break;
//         else if(errno != EINTR)
//             return -1;
//     } while (req.tv_sec > 0 || req.tv_nsec > 0);
//     return 0;
// #elif defined(OS_WINDOWS)
//     /* https://stackoverflow.com/a/5801863 */
//     /* https://stackoverflow.com/a/26945754 */
//     LARGE_INTEGER fr,t1,t2;
//     if (!QueryPerformanceFrequency(&fr))
//         return -1;
//     if (!QueryPerformanceCounter(&t1))
//         return -1;
//     do {
//         if (!QueryPerformanceCounter(&t2))
//             return -1;
//     } while(((double)t2.QuadPart-(double)t1.QuadPart)/(double)fr.QuadPart < sec);
//     return 0;
// #else
//     return -2;
// #endif
// }

MYS_API const char *mys_procname()
{
#if defined(OS_LINUX)
    static char exe[128] = { '\0' };
    if (exe[0] == '\0') {
        int ret = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
        if (ret > 0) {
            exe[ret] = '\0';
        } else {
            char *reason = strerror(errno);
            int pid = (int)getpid();
            snprintf(exe, sizeof(exe), "<error_exe.pid=%d (%s)>", pid, reason);
        }
    }
    return exe;
#elif defined(OS_BSD)
    const char *getprogname(void);
    return getprogname();
#endif
}

MYS_API void mys_wait_flag(const char *file, int line, const char *flagfile)
{
    int myrank = mys_mpi_myrank();
    int nranks = mys_mpi_nranks();
    int digits = mys_math_trunc(mys_math_log10(nranks)) + 1;
    digits = digits > 3 ? digits : 3;
    if (myrank == 0) {
        fprintf(stdout, "[WAIT::%0*d %s:%03d] Use \"touch %s\" to continue... ",
            digits, 0, file, line, flagfile);
        fflush(stdout);
    }
    struct stat fstat;
    time_t last_modified = stat(flagfile, &fstat) == 0 ? fstat.st_mtime : 0;
    while (stat(flagfile, &fstat) == 0 && fstat.st_mtime <= last_modified)
        sleep(1);

    mys_mpi_barrier();
    if (myrank == 0) {
        fprintf(stdout, "OK\n");
        fflush(stdout);
    }
}

#ifdef MYS_ENABLE_AFFINITY
#include <sched.h>
mys_thread_local char _mys_affinity_buffer[256];
MYS_API const char *mys_get_affinity() {
    int ncores = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if ((int)ncores > (int)sizeof(_mys_affinity_buffer))
        return NULL;

    cpu_set_t cpu_set;
    if (sched_getaffinity(0, sizeof(cpu_set_t), &cpu_set) == -1)
        return NULL;

    for (int i = 0; i < ncores; i++) {
        _mys_affinity_buffer[i] = CPU_ISSET(i, &cpu_set) ? '1' : '0';
    }
    _mys_affinity_buffer[ncores] = '\0';
    return _mys_affinity_buffer;
}

MYS_API void mys_print_affinity(FILE *fd)
{
    int myrank = mys_mpi_myrank();
    int nranks = mys_mpi_nranks();
    for (int rank = 0; rank < nranks; rank++) {
#ifdef _OPENMP
        #pragma omp parallel
#endif
        if (myrank == rank) {
#ifdef _OPENMP
            int nthreads = omp_get_num_threads();
            int thread_id = omp_get_thread_num();
            #pragma omp for ordered schedule(static,1)
#else
            int nthreads = 1;
            int thread_id = 0;
#endif
            for (int t = 0; t < nthreads; t++) {
#ifdef _OPENMP
                #pragma omp ordered
#endif
                {
                    int rank_digits = mys_math_trunc(mys_math_log10(nranks)) + 1;
                    int thread_digits = mys_math_trunc(mys_math_log10(nthreads)) + 1;
                    rank_digits = rank_digits > 3 ? rank_digits : 3;
                    thread_digits = thread_digits > 1 ? thread_digits : 1;
                    const char *affinity = mys_get_affinity();
                    fprintf(fd, "rank=%0*d:%0*d affinity=%s\n", rank_digits, myrank, thread_digits, thread_id, affinity);
                    fflush(fd);
                }
            }
        }
        mys_mpi_barrier();
    }
}

MYS_API void mys_stick_affinity()
{
#ifdef _OPENMP
    #pragma omp parallel
#endif
    {
#ifdef _OPENMP
        int thread_id = omp_get_thread_num();
#else
        int thread_id = 0;
#endif
        const char *affinity = mys_get_affinity();
        int len = strnlen(affinity, INT_MAX);
        int count = 0;
        for (int i = 0; i < len; i++) {
            if (affinity[i] == '0')
                continue;
            if (count == thread_id) {
                cpu_set_t cpu_set;
                CPU_ZERO(&cpu_set);
                CPU_SET(i, &cpu_set);
                sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set);
                break;
            }
            count += 1;
        }
    }
}
#endif

#ifdef MYS_ENABLE_NUMA
MYS_API int mys_numa_query(void *ptr)
{
    long page_size = sysconf(_SC_PAGESIZE);
    size_t mask = ~((size_t)page_size-1);
    void *aligned_ptr = (void *)((size_t)ptr & mask);
    int status[1] = { -1 };
    int ret = numa_move_pages(0, 1, &aligned_ptr, NULL, status, 0);
    if (ret != 0) {
        return -1;
    }
    return status[0];
}

MYS_API int mys_numa_move(void *ptr, int numa_id)
{
    long page_size = sysconf(_SC_PAGESIZE);
    size_t mask = ~((size_t)page_size-1);
    void *aligned_ptr = (void *)((size_t)ptr & mask);
    int nodes[1] = { numa_id };
    int status[1] = { -1 };
    return numa_move_pages(0, 1, &aligned_ptr, nodes, status, 0);
}
#endif

/* Safe string to numeric https://stackoverflow.com/a/18544436 */

MYS_API const char *mys_env_str(const char *name, const char *default_val)
{
    const char *val = getenv(name);
    if (val == NULL)
        return default_val;
    return val;
}

MYS_API int64_t mys_env_i64(const char *name, int64_t default_val)
{
    const char *str = getenv(name);
    if (str == NULL)
        return default_val;

    char *stop = NULL;
    errno = 0;
    int64_t num = strtoll(str, &stop, 10);
    int error = errno;
    errno = 0;

    if (stop == str)
        return default_val; /* contains with non-number */
    if ((num == LLONG_MAX || num == LLONG_MIN) && error == ERANGE)
        return default_val; /* number out of range for LONG */
    return num;
}

MYS_API int32_t mys_env_i32(const char *name, int32_t default_val)
{
    int64_t num = mys_env_i64(name, (int64_t)default_val);
    if ((num < INT_MIN) || (num > INT_MAX))
        return default_val; /* number out of range for INT */
    return (int32_t)num;
}

MYS_API double mys_env_f64(const char *name, double default_val)
{
    const char *str = getenv(name);
    if (str == NULL)
        return default_val;

    char *stop = NULL;
    errno = 0;
    double num = strtod(str, &stop);
    int error = errno;
    errno = 0;

    if (stop == str)
        return default_val; /* contains with non-number */
    if (error == ERANGE)
        return default_val; /* number out of range for DOUBLE */
    if (num != num)
        return default_val; /* Not A Number */
    return num;
}

/*
 * We don't assume to use IEEE754 arithmetic where
 * default_val float->double->float is unchanged.
 * Use strtof instead of mys_env_f64
 */
MYS_API float mys_env_f32(const char *name, float default_val)
{
    const char *str = getenv(name);
    if (str == NULL)
        return default_val;

    char *stop = NULL;
    errno = 0;
    float num = strtof(str, &stop);
    int error = errno;
    errno = 0;

    if (stop == str)
        return default_val; /* contains with non-number */
    if (error == ERANGE)
        return default_val; /* number out of range for FLOAT */
    if (num != num)
        return default_val; /* Not A Number */
    return num;
}
