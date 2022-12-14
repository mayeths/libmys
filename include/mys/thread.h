#pragma once

#include "config.h"
#include "memory.h"

/**********************************/
// mys_mutex_t
/**********************************/

typedef struct mys_mutex_t {
    int guard;
} mys_mutex_t;

#define __MYS_COMPARE_AND_SWAP(addr, oval, nval) __sync_val_compare_and_swap(addr, oval, nval)
#define __MYS_MUTEX_UNINITIALIZE 0
#define __MYS_MUTEX_IDLE 1
#define __MYS_MUTEX_BUSY 2
#define MYS_MUTEX_INITIALIZER { .guard = __MYS_MUTEX_IDLE }

static inline void mys_mutex_init(mys_mutex_t *lock)
{
    __MYS_COMPARE_AND_SWAP(&lock->guard, __MYS_MUTEX_UNINITIALIZE, __MYS_MUTEX_IDLE);
    smp_mb();
}
static inline void mys_mutex_lock(mys_mutex_t *lock)
{
    while(__MYS_COMPARE_AND_SWAP(&lock->guard, __MYS_MUTEX_IDLE, __MYS_MUTEX_BUSY) != __MYS_MUTEX_IDLE)
        continue;
    smp_mb();
}
static inline void mys_mutex_unlock(mys_mutex_t *lock)
{
    while(__MYS_COMPARE_AND_SWAP(&lock->guard, __MYS_MUTEX_BUSY, __MYS_MUTEX_IDLE) != __MYS_MUTEX_BUSY)
        continue;
    smp_mb();
}
/* gcc -O3 -g -fopenmp a.c && ./a.out 4999
    int size = 1024 * 1024;
    if (argc >= 2) {
        size = atoi(argv[1]);
    }
    int *data = (int *)calloc(sizeof(int), size);
    mys_mutex_t mys_mutex = MYS_MUTEX_INITIALIZER;
    #pragma omp parallel num_threads(96)
    {
        int nthreads = omp_get_num_threads();
        int tid = omp_get_thread_num();
        {
            mys_mutex_lock(&mys_mutex);
            data[size - 1] += 1;
            mys_mutex_unlock(&mys_mutex);
        }
        int num = data[size - 1];
        if (num >= 95) printf("%d see flag is 1 and num is %d\n", tid, num);
    }
*/


/**********************************/
// mys_thread_local
/**********************************/

#ifdef __cplusplus
#define mys_thread_local thread_local
#else
/* https://stackoverflow.com/a/18298965 */
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG) || defined(COMPILER_ICC)
#define mys_thread_local __thread
#elif defined(COMPILER_MSVC)
#define mys_thread_local __declspec(thread)
#elif __STDC_VERSION__ >= 201112 && !defined(__STDC_NO_THREADS__)
#define mys_thread_local _Thread_local
#else
#error "Cannot define mys_thread_local"
#endif

#endif
