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
#include "../_config.h"
#include "../errno.h"
#include "../mpistubs.h"
#include "../hrtime.h"

MYS_PUBLIC const char *mys_hrname()
{
#if defined(MYS_HRTIMER_HAVE_AACH64)
    return mys_hrname_aarch64();
#elif defined(MYS_HRTIMER_HAVE_POSIX)
    return mys_hrname_posix();
#elif defined(MYS_HRTIMER_HAVE_MPI)
    return mys_hrname_mpi();
#else
#error No default high resolution timer is available.
#endif
}

MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC uint64_t mys_hrtick()
{
#if defined(MYS_HRTIMER_HAVE_AACH64)
    return mys_hrtick_aarch64();
#elif defined(MYS_HRTIMER_HAVE_POSIX)
    return mys_hrtick_posix();
#elif defined(MYS_HRTIMER_HAVE_MPI)
    return mys_hrtick_mpi();
#else
#error No default high resolution timer is available.
#endif
}

MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC uint64_t mys_hrfreq()
{
#if defined(MYS_HRTIMER_HAVE_AACH64)
    return mys_hrfreq_aarch64();
#elif defined(MYS_HRTIMER_HAVE_POSIX)
    return mys_hrfreq_posix();
#elif defined(MYS_HRTIMER_HAVE_MPI)
    return mys_hrfreq_mpi();
#else
#error No default high resolution timer is available.
#endif
}

MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC double mys_hrtime()
{
#if defined(MYS_HRTIMER_HAVE_AACH64)
    return mys_hrtime_aarch64();
#elif defined(MYS_HRTIMER_HAVE_POSIX)
    return mys_hrtime_posix();
#elif defined(MYS_HRTIMER_HAVE_MPI)
    return mys_hrtime_mpi();
#else
#error No default high resolution timer is available.
#endif
}

MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC void mys_hrreset()
{
#if defined(MYS_HRTIMER_HAVE_AACH64)
    mys_hrreset_aarch64();
#elif defined(MYS_HRTIMER_HAVE_POSIX)
    mys_hrreset_posix();
#elif defined(MYS_HRTIMER_HAVE_MPI)
    mys_hrreset_mpi();
#else
#error No default high resolution timer is available.
#endif
}

MYS_PUBLIC void mys_hrsync(mys_MPI_Comm comm)
{
#if defined(MYS_HRTIMER_HAVE_AACH64)
    mys_hrsync_aarch64(comm);
#elif defined(MYS_HRTIMER_HAVE_POSIX)
    mys_hrsync_posix(comm);
#elif defined(MYS_HRTIMER_HAVE_MPI)
    mys_hrsync_mpi(comm);
#else
#error No default high resolution timer is available.
#endif
}

#if defined(MYS_HRTIMER_HAVE_AACH64)
typedef struct _mys_hrtime_aarch64_G_t {
    bool inited;
    uint64_t offset;
} _mys_hrtime_aarch64_G_t;
mys_thread_local _mys_hrtime_aarch64_G_t _mys_hrtime_aarch64_G = {
    .inited = false,
    .offset = 0,
};
MYS_PUBLIC const char *mys_hrname_aarch64() {
    return "High-resolution timer by AArch64 assembly (10ns)";
}
MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC uint64_t mys_hrtick_aarch64() {
    if (_mys_hrtime_aarch64_G.inited == false) {
        mys_hrreset_aarch64();
    }
    uint64_t t;
    __asm__ __volatile__("mrs %0, CNTVCT_EL0" : "=r"(t));
    return t - _mys_hrtime_aarch64_G.offset;
}
MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC uint64_t mys_hrfreq_aarch64() {
    uint64_t f;
    __asm__ __volatile__("mrs %0, CNTFRQ_EL0" : "=r"(f));
    return f;
}
MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC double mys_hrtime_aarch64() {
    return (double)mys_hrtick_aarch64() / (double)mys_hrfreq_aarch64();
}
MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC void mys_hrreset_aarch64() {
    __asm__ __volatile__("mrs %0, CNTVCT_EL0" : "=r"(_mys_hrtime_aarch64_G.offset));
    _mys_hrtime_aarch64_G.inited = true;
}
MYS_PUBLIC void mys_hrsync_aarch64(mys_MPI_Comm comm)
{
    for (size_t i = 0; i < 8; i++) mys_MPI_Barrier(comm);
    mys_hrreset_aarch64();
    for (size_t i = 0; i < 2; i++) mys_MPI_Barrier(comm);
}
#endif

#if defined(MYS_HRTIMER_HAVE_POSIX)
/*
 * clock_gettime [https://man7.org/linux/man-pages/man2/clock_gettime.2.html]
 * gettimeofday  [https://linux.die.net/man/2/gettimeofday]
 * It often takes 4 ns and doesn't involve any system call [https://stackoverflow.com/a/42190077]
 * Difference between CLOCK_REALTIME and CLOCK_MONOTONIC [https://stackoverflow.com/a/3527632]
 */
typedef struct _mys_hrtime_posix_G_t {
    bool inited;
    uint64_t offset;
} _mys_hrtime_posix_G_t;
mys_thread_local _mys_hrtime_posix_G_t _mys_hrtime_posix_G = {
    .inited = false,
    .offset = 0,
};
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
MYS_PUBLIC const char *mys_hrname_posix() {
#if defined(CLOCK_MONOTONIC)
    return "High-resolution timer by clock_gettime() of <time.h> (1us~10us)";
#else
    return "High-resolution timer by gettimeofday() of <sys/time.h> (1us~10us)";
#endif
}
MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC uint64_t mys_hrtick_posix() {
    uint64_t t = 0;
    if (_mys_hrtime_posix_G.inited == false) {
        mys_hrreset_posix();
    }
#if defined(CLOCK_MONOTONIC)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    t = (uint64_t)ts.tv_sec * (uint64_t)1000000000 + (uint64_t)ts.tv_nsec;
#else
    struct timeval ts;
    gettimeofday(&ts, NULL);
    t = (uint64_t)ts.tv_sec * (uint64_t)1000000 + (uint64_t)ts.tv_usec;
#endif
    return t - _mys_hrtime_posix_G.offset;
}
MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC uint64_t mys_hrfreq_posix() {
#if defined(CLOCK_MONOTONIC)
    return 1000000000;
#else
    return 1000000;
#endif
}
MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC double mys_hrtime_posix() {
    return (double)mys_hrtick_posix() / (double)mys_hrfreq_posix();
}
MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC void mys_hrreset_posix() {
#if defined(CLOCK_MONOTONIC)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    _mys_hrtime_posix_G.offset = (uint64_t)ts.tv_sec * (uint64_t)1000000000 + (uint64_t)ts.tv_nsec;
#else
    struct timeval ts;
    gettimeofday(&ts, NULL);
    _mys_hrtime_posix_G.offset = (uint64_t)ts.tv_sec * (uint64_t)1000000 + (uint64_t)ts.tv_usec;
#endif
    _mys_hrtime_posix_G.inited = true;
}
MYS_PUBLIC void mys_hrsync_posix(mys_MPI_Comm comm)
{
    for (size_t i = 0; i < 8; i++) mys_MPI_Barrier(comm);
    mys_hrreset_posix();
    for (size_t i = 0; i < 2; i++) mys_MPI_Barrier(comm);
}
#endif

#if defined(MYS_HRTIMER_HAVE_MPI)
typedef struct _mys_hrtime_mpi_G_t {
    bool inited;
    double offset;
} _mys_hrtime_mpi_G_t;
mys_thread_local _mys_hrtime_mpi_G_t _mys_hrtime_mpi_G = {
    .inited = false,
    .offset = 0,
};
MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC const char *mys_hrname_mpi() {
    return "High-resolution timer by <mpi.h> (1us~10us)";
}
MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC uint64_t mys_hrtick_mpi() {
    if (_mys_hrtime_mpi_G.inited == false) {
        mys_hrreset_mpi();
    }
    double current = mys_MPI_Wtime() - _mys_hrtime_mpi_G.offset;
    return (uint64_t)(current * 1e9); // in nano second
}
MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC uint64_t mys_hrfreq_mpi() {
    return (uint64_t)1000000000;
}
MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC double mys_hrtime_mpi() {
    return (double)mys_hrtick_mpi() / (double)mys_hrfreq_mpi();
}
MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC void mys_hrreset_mpi() {
    _mys_hrtime_mpi_G.offset = mys_MPI_Wtime();
    _mys_hrtime_mpi_G.inited = true;
}
MYS_PUBLIC void mys_hrsync_mpi(mys_MPI_Comm comm)
{
    for (size_t i = 0; i < 8; i++) mys_MPI_Barrier(comm);
    mys_hrreset_mpi();
    for (size_t i = 0; i < 2; i++) mys_MPI_Barrier(comm);
}
#endif
