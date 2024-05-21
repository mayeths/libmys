#include "_private.h"
#include "../thread.h"

mys_thread_local uint32_t __mys_thread_id = UINT32_MAX;
static uint32_t __mys_thread_count = 0;

MYS_API uint32_t mys_thread_id()
{
    if (__mys_thread_id == UINT32_MAX) {
        __mys_thread_id = mys_atomic_fetch_add(&__mys_thread_count, 1, MYS_ATOMIC_RELAXED);
    }
    return __mys_thread_id;
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
    uint32_t oval = __MYS_MUTEX_IDLE;
    uint32_t nval = __MYS_MUTEX_INVALID;
    bool ok = mys_atomic_compare_exchange(&lock->tid, &oval, &nval, MYS_ATOMIC_RELAXED, MYS_ATOMIC_RELAXED);
    return (ok == true) ? 0 : (oval == __MYS_MUTEX_INVALID) ? EINVAL : EBUSY;
}

MYS_API int mys_mutex_lock(mys_mutex_t *lock)
{
    uint32_t tid = mys_thread_id();
    uint32_t oval = __MYS_MUTEX_IDLE;
    uint32_t nval = tid;
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
    uint32_t tid = mys_thread_id();
    uint32_t oval = tid;
    uint32_t nval = __MYS_MUTEX_IDLE;
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
