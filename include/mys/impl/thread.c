#include "../thread.h"

#ifdef _OPENMP
#include <omp.h>
#else
mys_thread_local int __mys_thread_id = -1;
static int __mys_thread_count = 0;
#endif

MYS_API int mys_thread_id()
{
#ifdef _OPENMP
    return omp_get_thread_num();
#else
    if (__mys_thread_id == -1) {
        __mys_thread_id = mys_atomic_fetch_add(&__mys_thread_count, 1, MYS_ATOMIC_RELAXED);
    }
    return __mys_thread_id;
#endif
}


#ifdef MYS_USE_POSIX_MUTEX
MYS_API int mys_mutex_init(mys_mutex_t *lock)
{
    return pthread_mutex_init(lock, NULL);
}

MYS_API int mys_mutex_destroy(mys_mutex_t *lock)
{
    return pthread_mutex_destroy(lock);
}

MYS_API int mys_mutex_lock(mys_mutex_t *lock)
{
    return pthread_mutex_lock(lock);
}

MYS_API int mys_mutex_unlock(mys_mutex_t *lock)
{
    return pthread_mutex_unlock(lock);
}
#else
MYS_API int mys_mutex_init(mys_mutex_t *lock)
{
    mys_atomic_store_n(&lock->tid, __MYS_MUTEX_IDLE, MYS_ATOMIC_RELAXED);
    return 0;
}

MYS_API int mys_mutex_destroy(mys_mutex_t *lock)
{
    int oval = __MYS_MUTEX_IDLE;
    int nval = __MYS_MUTEX_INVALID;
    bool ok = mys_atomic_compare_exchange(&lock->tid, &oval, &nval, MYS_ATOMIC_RELAXED, MYS_ATOMIC_RELAXED);
    return (ok == true) ? 0 : (oval == __MYS_MUTEX_INVALID) ? EINVAL : EBUSY;
}

MYS_API int mys_mutex_lock(mys_mutex_t *lock)
{
    int tid = mys_thread_id();
    int oval = __MYS_MUTEX_IDLE;
    int nval = tid;
    while (!mys_atomic_compare_exchange(&lock->tid, &oval, &nval, MYS_ATOMIC_ACQUIRE, MYS_ATOMIC_RELAXED)) {
        if (oval == tid)
            return EDEADLK;
        if (oval == __MYS_MUTEX_INVALID)
            return EINVAL;
        oval = __MYS_MUTEX_IDLE;
    }
    return 0;
}

MYS_API int mys_mutex_unlock(mys_mutex_t *lock)
{
    int tid = mys_thread_id();
    int oval = tid;
    int nval = __MYS_MUTEX_IDLE;
    while (!mys_atomic_compare_exchange(&lock->tid, &oval, &nval, MYS_ATOMIC_RELEASE, MYS_ATOMIC_RELAXED)) {
        if (oval != tid)
            return EPERM;
        if (oval == __MYS_MUTEX_INVALID)
            return EINVAL;
        if (oval == __MYS_MUTEX_IDLE)
            return 0;
        oval = tid;
    }
    return 0;
}
#endif /*MYS_USE_POSIX_MUTEX*/
