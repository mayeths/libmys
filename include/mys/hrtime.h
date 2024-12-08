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
/* Please check https://github.com/nclack/tictoc for platform-timer (Windows/Linux/MacOS) */
/* See OpenMPI Wiki https://github.com/open-mpi/ompi/wiki/Timers for how they think and implement timer */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>

#include "_config.h"
#include "macro.h"
#include "mpi.h"
#include "thread.h"

// The default high-resolution timer
MYS_PUBLIC const char *mys_hrname();
MYS_PUBLIC uint64_t mys_hrtick();
MYS_PUBLIC uint64_t mys_hrfreq();
MYS_PUBLIC double mys_hrtime();
MYS_PUBLIC void mys_hrreset();
MYS_PUBLIC void mys_hrsync();

#if defined(ARCH_AARCH64)
#define MYS_HRTIMER_HAVE_AACH64
MYS_PUBLIC const char *mys_hrname_aarch64();
MYS_PUBLIC uint64_t mys_hrtick_aarch64();
MYS_PUBLIC uint64_t mys_hrfreq_aarch64();
MYS_PUBLIC double mys_hrtime_aarch64();
MYS_PUBLIC void mys_hrreset_aarch64();
MYS_PUBLIC void mys_hrsync_aarch64();
#endif

#if defined(POSIX_COMPLIANCE)
#define MYS_HRTIMER_HAVE_POSIX
MYS_PUBLIC const char *mys_hrname_posix();
MYS_PUBLIC uint64_t mys_hrtick_posix();
MYS_PUBLIC uint64_t mys_hrfreq_posix();
MYS_PUBLIC double mys_hrtime_posix();
MYS_PUBLIC void mys_hrreset_posix();
MYS_PUBLIC void mys_hrsync_posix();
#endif

#define MYS_HRTIMER_HAVE_MPI
MYS_PUBLIC const char *mys_hrname_mpi();
MYS_PUBLIC uint64_t mys_hrtick_mpi();
MYS_PUBLIC uint64_t mys_hrfreq_mpi();
MYS_PUBLIC double mys_hrtime_mpi();
MYS_PUBLIC void mys_hrreset_mpi();
MYS_PUBLIC void mys_hrsync_mpi();

/* g++ -std=c++11 -I../include -lm ./a.cpp && ./a.out
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#define MYS_IMPL
#include <mys.h>

int main(int argc, const char **argv)
{
    uint64_t n = 1000000000;
    double t = 0;
    double t1 = mys_hrtime();
    for (uint64_t i = 0; i < n; i++) {
        t += mys_hrtime();
    }
    double t2 = mys_hrtime();
    double tdiff = t2 - t1;
    DEBUG(0, "tdiff %E/%llu=%E seconds", tdiff, n, tdiff/(double)n);
}

Apple M1:
    * no -O: 7.059100E-09 seconds
    * -O0: 7.061317E-09 seconds
    * -O1: 5.355991E-09 seconds
    * -O2: 2.669528E-09 seconds
    * -O3: 2.716072E-09 seconds
*/
