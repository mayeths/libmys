/* 
 * Copyright (c) 2024 Haopeng Huang - All Rights Reserved
 * 
 * Licensed under the MIT License. You may use, distribute,
 * and modify this code under the terms of the MIT license.
 * You should have received a copy of the MIT license along
 * with this file. If not, see:
 * 
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include "_config.h"
#include "atomic.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>

/**********************************/
// mys_thread_local
/**********************************/
/* https://stackoverflow.com/a/18298965 */
#if defined(COMPILER_GCC)
#define mys_thread_local __thread
#elif defined(COMPILER_CLANG)
#define mys_thread_local __thread
#elif defined(COMPILER_ICC)
#define mys_thread_local __thread
#elif defined(COMPILER_MSVC)
#define mys_thread_local __declspec(thread)
#elif __STDC_VERSION__ >= 201112 && !defined(__STDC_NO_THREADS__)
#define mys_thread_local _Thread_local
#elif defined(__cplusplus)
// C++ thread_local does't allow static non-zero initalze
#define mys_thread_local thread_local
#else
#error "Cannot define mys_thread_local"
#endif

/**********************************/
// mys_thread
/**********************************/
// intel compiler complains that mys_mutex_lock uses mys_atomic_load_n
// on int32_t type thread_id. Seems like it only provide uint32_t atomic functions
MYS_PUBLIC uint32_t mys_thread_id();

/**********************************/
// mys_mutex_t
/**********************************/
#ifdef MYS_USE_POSIX_MUTEX
#include <pthread.h>
typedef pthread_mutex_t mys_mutex_t;
#define MYS_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#else
struct mys_mutex_t { uint32_t tid; };
typedef struct mys_mutex_t mys_mutex_t;
#define __MYS_MUTEX_IDLE      (UINT32_MAX - 1)
#define __MYS_MUTEX_INVALID   (UINT32_MAX - 2)
#define MYS_MUTEX_INITIALIZER { .tid = __MYS_MUTEX_IDLE }
#endif /*MYS_USE_POSIX_MUTEX*/
///////////
MYS_PUBLIC int mys_mutex_init(mys_mutex_t *lock);
MYS_PUBLIC int mys_mutex_destroy(mys_mutex_t *lock);
MYS_PUBLIC int mys_mutex_lock(mys_mutex_t *lock);
MYS_PUBLIC int mys_mutex_unlock(mys_mutex_t *lock);

/* gcc -Wall -Wextra -O3 -I${MYS_DIR}/include -g -fopenmp a.c && ./a.out 4999

#define MYS_IMPL
#define MYS_NO_MPI
// #define MYS_USE_POSIX_MUTEX
#include <mys.h>
#include <omp.h>

double test_it(int *data, int *saws, int count, int nthreads)
{
    mys_mutex_t mys_mutex = MYS_MUTEX_INITIALIZER;
    mys_mutex_init(&mys_mutex);
    memset(data, 0, sizeof(int) * count);
    memset(saws, 0, sizeof(int) * nthreads);

    double start = mys_hrtime();
    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();
        int val = -1;
        {
            mys_mutex_lock(&mys_mutex);
            val = data[count - 1]++;
            mys_mutex_unlock(&mys_mutex);
        }
        int how_many_saw = mys_atomic_add_fetch(&saws[val], 1, MYS_ATOMIC_RELAXED);
        ASX_EQ_I32(1, how_many_saw, "should only thread %d see val %d", tid, val);
    }
    double stop = mys_hrtime();
    double time = stop - start;

    for (int val = 0; val < nthreads; val++)
        ASX_EQ_I32(saws[val], 1, "should only one thread see val %d", val);
    mys_mutex_destroy(&mys_mutex);
    return time * 1e6;
}

int main(int argc, const char **argv)
{
    int count = (argc == 1) ? 1024 * 1024 : atoi(argv[1]);
    int nwarms = 100;
    int niters = 10000;
    int nthreads = omp_get_max_threads();
    int *data = (int *)malloc(sizeof(int) * count);
    int *saws = (int *)malloc(sizeof(int) * nthreads);
    printf("nthreads=%d\n", nthreads);

    for (int warm = 0; warm < nwarms; warm++) {
        test_it(data, saws, count, nthreads);
    }

    double tot_time = 0;
    double min_time = 1e9;
    double max_time = 0;
    for (int iter = 0; iter < niters; iter++) {
        double time = test_it(data, saws, count, nthreads);
        tot_time += time;
        max_time = time > max_time ? time : max_time;
        min_time = time < min_time ? time : min_time;
    }

    printf("Used %7.3f / %d = %7.3f (max %7.3f min %7.3f) us\n",
        tot_time, niters, tot_time / (double)niters, max_time, min_time);
    free(data);
    free(saws);
}

*/
