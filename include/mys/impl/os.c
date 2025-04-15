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
#include "../_config.h"
#include "../errno.h"
#include "../mpistubs.h"
#include "../os.h"
#include "../memory.h"
#include "../hrtime.h"

#ifdef MYS_ENABLE_NUMA
#include <numa.h>
#endif

#ifdef _OPENMP
#include <omp.h>
#endif

// extern int kill(pid_t pid, int sig) __THROW;
// extern char *strdup(const char *s) __THROW;
// extern ssize_t readlink(const char *path, char *buf, size_t bufsize) __THROW;

#if 0 // this version doesn't has null terminator handling
MYS_PUBLIC size_t mys_readfd(char **buffer, size_t *buffer_size, int fd, bool enable_realloc)
{
    size_t read_size = 0;

    if (*buffer == NULL) {
        if (!enable_realloc)
            goto finished;
        else {
            size_t old_buffer_size = *buffer_size;
            if (*buffer_size == 0)
                *buffer_size = 64;
            *buffer = (char *)mys_realloc2(MYS_ARENA_OS, NULL, *buffer_size, old_buffer_size);
            if (*buffer == NULL)
                goto finished;
        }
    }

    while (1)
    {
        size_t remaining = *buffer_size - read_size;
        if (remaining == 0) {
            if (!enable_realloc)
                goto finished;
            else {
                // read() will fail if the parameter nbyte exceeds INT_MAX,
                // and do not attempt a partial read.
                size_t old_buffer_size = *buffer_size;
                size_t threshold = 4096; /* size_t threshold = INT_MAX; */
                if (*buffer_size * 2 < threshold)
                    *buffer_size *= 2;
                else
                    *buffer_size += threshold;
                *buffer = (char *)mys_realloc2(MYS_ARENA_OS, *buffer, *buffer_size, old_buffer_size);
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
#else
MYS_PUBLIC size_t mys_readfd(char **buffer, size_t *buffer_size, int fd, bool enable_realloc)
{
    size_t read_size = 0;
    bool needs_null_terminator = false;

    if (*buffer == NULL) {
        if (!enable_realloc) {
            return 0;
        } else {
            size_t old_buffer_size = *buffer_size;
            if (*buffer_size == 0)
                *buffer_size = 4096;
            *buffer = (char *)mys_realloc2(MYS_ARENA_OS, NULL, *buffer_size, old_buffer_size);
            if (*buffer == NULL)
                return 0;
        }
    }

    while (1) {
        size_t remaining = *buffer_size - read_size;
        if (remaining <= 1) { // Always leave room for null terminator if we might need it
            if (!enable_realloc) {
                needs_null_terminator = true;
                break; // Buffer full, but we'll add null terminator later
            }

            size_t new_size;
            if (*buffer_size <= (SIZE_MAX / 2)) {
                new_size = *buffer_size * 2;
            } else {
                if (*buffer_size == SIZE_MAX - 1) { // Leave room for null terminator
                    needs_null_terminator = true;
                    break;
                }
                new_size = SIZE_MAX - 1; // Reserve space for null terminator
            }

            char *new_buffer = (char *)mys_realloc2(MYS_ARENA_OS, *buffer, new_size, *buffer_size);
            if (new_buffer == NULL) {
                needs_null_terminator = true;
                break;
            }

            *buffer = new_buffer;
            *buffer_size = new_size;
            remaining = *buffer_size - read_size;
        }

        ssize_t bytes_read = read(fd, *buffer + read_size, remaining - 1); // Leave space for null
        
        if (bytes_read < 0) {
            // Read error
            if (read_size > 0) {
                needs_null_terminator = true;
            }
            break;
        }
        
        if (bytes_read == 0) {
            // EOF reached
            needs_null_terminator = true;
            break;
        }

        read_size += (size_t)bytes_read;
    }

    // Ensure null termination if we read any data
    if (needs_null_terminator) {
        if (read_size < *buffer_size) {
            // There's room, just add null terminator
            (*buffer)[read_size] = '\0';
        } else if (enable_realloc) {
            // Need to realloc to add null terminator
            char *new_buffer = (char *)mys_realloc2(MYS_ARENA_OS, *buffer, read_size + 1, *buffer_size);
            if (new_buffer) {
                *buffer = new_buffer;
                *buffer_size = read_size + 1;
                (*buffer)[read_size] = '\0';
            } else {
                // Realloc failed, fallback: truncate
                if (*buffer_size > 0) {
                    (*buffer)[*buffer_size - 1] = '\0';
                    read_size = *buffer_size - 1;
                }
            }
        } else {
            // No realloc allowed, try to fit null terminator
            if (*buffer_size > 0) {
                (*buffer)[*buffer_size - 1] = '\0';
                read_size = *buffer_size - 1;
            }
        }
    }

    return read_size;
}
#endif

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
MYS_PUBLIC mys_popen_t mys_popen_create(const char *command)
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
        close(in[1]);  close(0); in[1]=dup(in[0]);  // pipe to stdin, in[1] must store 0 after dup
        close(out[0]); close(1); out[0]=dup(out[1]); // pipe to stdout, out[0] must store 1 after dup
        close(err[0]); close(2); err[0]=dup(err[1]); // pipe to stderr, err[0] must store 2 after dup
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

MYS_PUBLIC bool mys_popen_test(mys_popen_t *popen)
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

MYS_PUBLIC bool mys_popen_wait(mys_popen_t *popen)
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

MYS_PUBLIC bool mys_popen_kill(mys_popen_t *popen, int signo)
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

MYS_PUBLIC mys_prun_t mys_prun_create(const char *command, char *buf_out, size_t max_out, char *buf_err, size_t max_err)
{
    mys_prun_t prun;
    prun.cmd = NULL;
    prun.out = buf_out;
    prun.err = buf_err;
    prun.len_out = 0;
    prun.len_err = 0;
    prun.retval = -1;
    prun.success = false;
    prun._alloced = false;
    prun._cap_cmd = 0;
    prun._cap_out = max_out;
    prun._cap_err = max_err;

    mys_popen_t popen = mys_popen_create(command);
    if (!popen.alive)
        return prun;

    prun.len_out = mys_readfd(&prun.out, &prun._cap_out, popen.ofd, prun._alloced);
    prun.len_err = mys_readfd(&prun.err, &prun._cap_err, popen.efd, prun._alloced);
    prun.len_out = _mys_cut_suffix_space(prun.out, prun.len_out);
    prun.len_err = _mys_cut_suffix_space(prun.err, prun.len_err);
    mys_popen_wait(&popen);
    prun.success = true;
    prun.retval = popen.retval;
    return prun;
}

MYS_PUBLIC mys_prun_t mys_prun_create2(const char *command, ...)
{
    mys_prun_t prun;
    prun.cmd = NULL;
    prun.out = NULL;
    prun.err = NULL;
    prun.len_out = 0;
    prun.len_err = 0;
    prun.retval = -1;
    prun.success = false;
    prun._alloced = true;
    prun._cap_cmd = 0;
    prun._cap_out = 0;
    prun._cap_err = 0;

    va_list vargs, vargs_test;
    va_start(vargs, command);
    va_copy(vargs_test, vargs);
    prun._cap_cmd = vsnprintf(NULL, 0, command, vargs_test) + 1;
    prun.cmd = (char *)mys_malloc2(MYS_ARENA_OS, prun._cap_cmd);
    vsnprintf(prun.cmd, prun._cap_cmd, command, vargs);
    va_end(vargs);

    mys_popen_t popen = mys_popen_create(prun.cmd);
    if (!popen.alive)
        return prun;

    prun.len_out = mys_readfd(&prun.out, &prun._cap_out, popen.ofd, prun._alloced);
    prun.len_err = mys_readfd(&prun.err, &prun._cap_err, popen.efd, prun._alloced);
    prun.len_out = _mys_cut_suffix_space(prun.out, prun.len_out);
    prun.len_err = _mys_cut_suffix_space(prun.err, prun.len_err);
    mys_popen_wait(&popen);
    prun.success = true;
    prun.retval = popen.retval;
    return prun;
}

MYS_PUBLIC void mys_prun_destroy(mys_prun_t *prun)
{
    if (prun == NULL || !prun->success)
        return;
    if (prun->_alloced) {
        if (prun->cmd != NULL) mys_free2(MYS_ARENA_OS, prun->cmd, prun->_cap_cmd);
        if (prun->out != NULL) mys_free2(MYS_ARENA_OS, prun->out, prun->_cap_out);
        if (prun->err != NULL) mys_free2(MYS_ARENA_OS, prun->err, prun->_cap_err);
    }
    prun->out = NULL;
    prun->err = NULL;
    prun->len_out = 0;
    prun->len_err = 0;
}


MYS_PUBLIC bool mys_mkdir(const char *path, mode_t mode)
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

MYS_PUBLIC bool mys_ensure_dir(const char *path, mode_t mode)
{
    size_t len = strlen(path) + 1;
    char *p = (char *)mys_malloc2(MYS_ARENA_OS, len);
    memcpy(p, path, len);
    // char *p = strdup(path);
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
    // free(p);
    mys_free2(MYS_ARENA_OS, p, len);
    return success;
}

MYS_PUBLIC bool mys_ensure_parent(const char *path, mode_t mode)
{
    size_t len = strlen(path) + 1;
    char *pathcopy = (char *)mys_malloc2(MYS_ARENA_OS, len);
    memcpy(pathcopy, path, len);
    // char *pathcopy = strdup(path);
    char *dname = dirname(pathcopy);
    bool success = mys_ensure_dir(dname, mode);
    // free(pathcopy);
    mys_free2(MYS_ARENA_OS, pathcopy, len);
    return success;
}

MYS_PUBLIC int mys_busysleep(double seconds)
{
    double t_start = mys_hrtime();
    double t_curr = mys_hrtime();
    while (t_curr - t_start < seconds) {
        t_curr = mys_hrtime();
    }
    return 0;
}

MYS_PUBLIC const char *mys_procname()
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

MYS_PUBLIC void mys_wait_flag(const char *file, int line, const char *flagfile)
{
    int myrank, nranks;
    mys_MPI_Comm_rank(mys_MPI_COMM_WORLD, &myrank);
    mys_MPI_Comm_size(mys_MPI_COMM_WORLD, &nranks);
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

    mys_MPI_Barrier(mys_MPI_COMM_WORLD);
    if (myrank == 0) {
        fprintf(stdout, "OK\n");
        fflush(stdout);
    }
}

#ifdef MYS_ENABLE_AFFINITY
#include <sched.h>
mys_thread_local char _mys_affinity_buffer[256];
MYS_PUBLIC const char *mys_get_affinity() {
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

MYS_PUBLIC void mys_print_affinity(FILE *fd)
{
    int myrank, nranks;
    mys_MPI_Comm_rank(mys_MPI_COMM_WORLD, &myrank);
    mys_MPI_Comm_size(mys_MPI_COMM_WORLD, &nranks);
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
        mys_MPI_Barrier(mys_MPI_COMM_WORLD);
    }
}

MYS_PUBLIC void mys_stick_affinity()
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
MYS_PUBLIC int mys_numa_query(void *ptr)
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

MYS_PUBLIC int mys_numa_query_continuous(void *ptr, size_t size)
{
    if (size == 0)
        return -1;

    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1)
        return -1;

    uintptr_t page_mask = ~((uintptr_t)page_size - 1);
    void *aligned_ptr = (void *)((uintptr_t)ptr & page_mask);
    void *end_ptr = (void *)((uintptr_t)ptr + size);
    uintptr_t real_size = (uintptr_t)end_ptr - (uintptr_t)aligned_ptr;
    size_t num_pages = (real_size + page_size - 1) / page_size;
    int last_numa = -1;

    void **pages = (void **)malloc(num_pages * sizeof(void *));
    int *statuses = (int *)malloc(num_pages * sizeof(int));
    if (pages == NULL || statuses == NULL)
        goto finished;

    for (size_t i = 0; i < num_pages; i++) {
        pages[i] = (void *)((uintptr_t)aligned_ptr + i * page_size);
        statuses[i] = -1;
    }

    if (numa_move_pages(0, num_pages, pages, NULL, statuses, 0) != 0)
        goto finished;

    last_numa = statuses[0];
    for (size_t i = 1; i < num_pages; i++) {
        if (statuses[i] != last_numa) {
            last_numa = -1;
            break;
        }
    }

finished:
    if (pages) free(pages);
    if (statuses) free(statuses);
    return last_numa;
}

MYS_PUBLIC int mys_numa_move(void *ptr, int numa_id)
{
    long page_size = sysconf(_SC_PAGESIZE);
    size_t mask = ~((size_t)page_size-1);
    void *aligned_ptr = (void *)((size_t)ptr & mask);
    int nodes[1] = { numa_id };
    int status[1] = { -1 };
    return numa_move_pages(0, 1, &aligned_ptr, nodes, status, 0);
}

MYS_PUBLIC int mys_current_cpu()
{
    return sched_getcpu();
}

MYS_PUBLIC int mys_current_numa()
{
    return numa_node_of_cpu(mys_current_cpu());
}

MYS_PUBLIC int mys_cpu_num()
{
    return (int)sysconf(_SC_NPROCESSORS_ONLN);
}

MYS_PUBLIC int mys_numa_num()
{
    int max_node = numa_max_node();
    if (max_node < 0) {
        perror("numa_max_node failed");
        return -1;
    }
    return max_node + 1; // max_node is 0-based
}

MYS_PUBLIC int mys_numa_size(int numa)
{
    struct bitmask *cpumask = numa_allocate_cpumask();
    if (!cpumask) {
        perror("Failed to allocate cpumask");
        return -1;
    }

    if (numa_node_to_cpus(numa, cpumask) != 0) {
        perror("numa_node_to_cpus failed");
        numa_free_cpumask(cpumask);
        return -1;
    }

    int cpu_count = 0;
    for (int cpu = 0; cpu < (int)cpumask->size; cpu++) {
        if (numa_bitmask_isbitset(cpumask, cpu)) {
            cpu_count++;
        }
    }

    numa_free_cpumask(cpumask);
    return cpu_count;
}

MYS_PUBLIC int mys_cpu_affinity_num(pid_t pid)
{
    struct bitmask *cpumask = numa_allocate_cpumask();

    if (numa_sched_getaffinity(pid, cpumask) == -1) {
        perror("numa_sched_getaffinity failed");
        numa_free_cpumask(cpumask);
        return -1;
    }

    int cpu_affinity_num = 0;

    for (int cpu = 0; cpu < (int)cpumask->size; cpu++) {
        if (numa_bitmask_isbitset(cpumask, cpu)) {
            cpu_affinity_num += 1;
        }
    }

    numa_free_cpumask(cpumask);
    return cpu_affinity_num;
}

MYS_PUBLIC int mys_numa_affinity_num(pid_t pid)
{
    struct bitmask *cpumask = numa_allocate_cpumask();
    if (numa_sched_getaffinity(pid, cpumask) == -1) {
        perror("numa_sched_getaffinity failed");
        numa_free_cpumask(cpumask);
        return -1;
    }

    int *numas = (int *)malloc(sizeof(int) * cpumask->size);
    if (!numas) {
        perror("Memory allocation for NUMA nodes failed");
        numa_free_cpumask(cpumask);
        return -1;
    }

    int num_entries = 0;

    for (int cpu = 0; cpu < (int)cpumask->size; cpu++) {
        if (numa_bitmask_isbitset(cpumask, cpu)) {
            int numa = numa_node_of_cpu(cpu);
            if (numa >= 0) {
                numas[num_entries++] = numa;
            }
        }
    }

    mys_sort_i32(numas, num_entries);

    int numa_affinity_num = 0;
    for (int i = 0; i < num_entries; i++) {
        if (i == 0 || numas[i] != numas[i - 1]) {
            numa_affinity_num += 1;
        }
    }

    numa_free_cpumask(cpumask);
    free(numas);
    return numa_affinity_num;
}
#endif /*MYS_ENABLE_NUMA*/

/* Safe string to numeric https://stackoverflow.com/a/18544436 */

MYS_PUBLIC const char *mys_env_str(const char *name, const char *default_val)
{
    const char *val = getenv(name);
    if (val == NULL)
        return default_val;
    return val;
}

MYS_PUBLIC int mys_env_int(const char *name, int default_val)
{
    return mys_str_to_int(getenv(name), default_val);
}

MYS_PUBLIC long mys_env_long(const char *name, long default_val)
{
    return mys_str_to_long(getenv(name), default_val);
}

MYS_PUBLIC size_t mys_env_sizet(const char *name, size_t default_val)
{
    return mys_str_to_sizet(getenv(name), default_val);
}

MYS_PUBLIC uint64_t mys_env_u64(const char *name, uint64_t default_val)
{
    return mys_str_to_u64(getenv(name), default_val);
}

MYS_PUBLIC uint32_t mys_env_u32(const char *name, uint32_t default_val)
{
    return mys_str_to_u32(getenv(name), default_val);
}

MYS_PUBLIC int64_t mys_env_i64(const char *name, int64_t default_val)
{
    return mys_str_to_i64(getenv(name), default_val);
}

MYS_PUBLIC int32_t mys_env_i32(const char *name, int32_t default_val)
{
    return mys_str_to_i32(getenv(name), default_val);
}

MYS_PUBLIC double mys_env_f64(const char *name, double default_val)
{
    return mys_str_to_f64(getenv(name), default_val);
}

/*
 * We don't assume to use IEEE754 arithmetic where
 * default_val float->double->float is unchanged.
 * Use strtof instead of mys_env_f64
 */
MYS_PUBLIC float mys_env_f32(const char *name, float default_val)
{
    return mys_str_to_f32(getenv(name), default_val);
}
