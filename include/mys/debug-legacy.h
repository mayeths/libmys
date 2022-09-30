#pragma once

#include <stdio.h>
#include <mpi.h>

/*
#define ENSURE_MPI_INIT() do {               \
    int i;                                   \
    MPI_Initialized(&i);                     \
    if (!i) {                                \
        MPI_Init_thread(NULL,NULL,           \
        MPI_THREAD_SINGLE,&i);               \
        fprintf(stdout,                      \
            ">>>>>--- Nevel let libmys init" \
            " MPI you dumbass ---<<<<<\n");  \
        fflush(stdout);                      \
    }                                        \
}while(0)
*/
// static inline int MYRANK()
// {
//     ENSURE_MPI_INIT();
//     int myrank = 0;
//     MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
//     return myrank;
// }
// static inline int NRANKS()
// {
//     ENSURE_MPI_INIT();
//     int nranks = 0;
//     MPI_Comm_size(MPI_COMM_WORLD, &nranks);
//     return nranks;
// }
// static inline void BARRIER()
// {
//     ENSURE_MPI_INIT();
//     MPI_Barrier(MPI_COMM_WORLD);
// }

// FIXME: The VA_ARGS after if() will cause collective OP. hang
// Use snprintf() before if() to trigger function calls in VA_ARGS
// Or use static inline vaargs function(force to initalize argument)
// + macro(get __FILE__, __LINE__)
/*
#ifdef DEBUG
#warning DEBUG was predefined, skipped definition from libmys
#else
#define DEBUG(who, fmt, ...) do {                          \
    int nranks = NRANKS(), myrank = MYRANK();              \
    int nprefix = trunc(log10(nranks)) + 1;                \
    nprefix = nprefix > 3 ? nprefix : 3;                   \
    _Pragma("omp critical (mys)")                          \
    if ((myrank) == (who)) {                               \
        fprintf(stdout, "[DEBUG::%0*d %s:%03u] " fmt "\n", \
            nprefix, myrank,                               \
            __FILE__, __LINE__, ##__VA_ARGS__ );           \
        fflush(stdout);                                    \
    }                                                      \
} while (0)
#endif
*/

/*
#ifdef DEBUG_ORDERED
#warning DEBUG_ORDERED was predefined, skipped definition from libmys
#else
#define DEBUG_ORDERED(fmt, ...) do {   \
    int nranks = NRANKS();             \
    for (int i = 0; i < nranks; i++) { \
        DEBUG(i, fmt, ##__VA_ARGS__);  \
        BARRIER();                     \
    }                                  \
} while (0)
#endif
*/

/*
#ifdef PRINTF
#warning PRINTF was predefined, skipped definition from libmys
#else
#define PRINTF(who, fmt, ...) do {           \
    int myrank = MYRANK();                   \
    _Pragma("omp critical (mys)")            \
    if ((myrank) == (who)) {                 \
        fprintf(stdout, fmt, ##__VA_ARGS__); \
        fflush(stdout);                      \
    }                                        \
} while (0)
#endif
*/

/*
#ifdef ASSERT
#warning ASSERT was predefined, skipped definition from libmys
#else
#define ASSERT(exp, fmt, ...) do {                          \
    _Pragma("omp critical (mys)")                           \
    if (!(exp)) {                                           \
        int nranks = NRANKS(), myrank = MYRANK();           \
        int nprefix = trunc(log10(nranks)) + 1;             \
        nprefix = nprefix > 3 ? nprefix : 3;                \
        fprintf(stdout, "[ASSERT::%0*d %s:%03u] " fmt "\n", \
            nprefix, myrank,                                \
            __FILE__, __LINE__, ##__VA_ARGS__);             \
        fflush(stdout);                                     \
        exit(1);                                            \
    }                                                       \
} while (0)
#endif
*/

/*
#ifdef FAILED
#warning FAILED was predefined, skipped definition from libmys
#else
#define FAILED(fmt, ...) do {                               \
    int myrank = MYRANK();                                  \
    _Pragma("omp critical (mys)")                           \
    fprintf(stdout, "[FAILED %s:%03u rank=%03d] " fmt "\n", \
        __FILE__, __LINE__, myrank, ##__VA_ARGS__           \
    );                                                      \
    fflush(stdout);                                         \
    exit(1);                                                \
} while (0)
#endif


#ifdef THROW_NOT_IMPL
#warning THROW_NOT_IMPL was predefined, skipped definition from libmys
#else
#define THROW_NOT_IMPL() FAILED("Not implemented.")
#endif
*/

/*
#ifdef WAIT_FLAG
#warning WAIT_FLAG was predefined, skipped definition from libmys
#else
#include <unistd.h>
#include <sys/stat.h>
#define WAIT_FLAG(file) do {                      \
    int myrank = MYRANK();                        \
    _Pragma("omp critical (mys)")                 \
    if (myrank == 0) {                            \
        fprintf(stdout, "[WAIT %s:%03u] "         \
            "Use \"touch %s\" to continue... ",   \
            __FILE__, __LINE__, file);            \
        fflush(stdout);                           \
    }                                             \
    struct stat fstat;                            \
    time_t mod_time;                              \
    if (stat(file, &fstat) == 0) {                \
        mod_time = fstat.st_mtime;                \
    } else {                                      \
        mod_time = 0;                             \
    }                                             \
    while (true) {                                \
        if (stat(file, &fstat) == 0) {            \
            if (mod_time < fstat.st_mtime) break; \
        }                                         \
        sleep(1);                                 \
    }                                             \
    MPI_Barrier(MPI_COMM_WORLD);                  \
    _Pragma("omp critical (mys)")                 \
    if (myrank == 0) {                            \
        fprintf(stdout, "OK\n");                  \
        fflush(stdout);                           \
    }                                             \
} while (0)
#endif
*/
