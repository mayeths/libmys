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


MYS_PUBLIC const char *mys_hrname();
MYS_PUBLIC uint64_t mys_hrtick();
MYS_PUBLIC uint64_t mys_hrfreq();
MYS_PUBLIC double mys_hrtime();

MYS_PUBLIC const char *mys_hrname_mpi();
MYS_PUBLIC uint64_t mys_hrtick_mpi();
MYS_PUBLIC uint64_t mys_hrfreq_mpi();
MYS_PUBLIC double mys_hrtime_mpi();

MYS_PUBLIC const char *mys_hrname_openmp();
MYS_PUBLIC uint64_t mys_hrtick_openmp();
MYS_PUBLIC uint64_t mys_hrfreq_openmp();
MYS_PUBLIC double mys_hrtime_openmp();

#if defined(ARCH_AARCH64)
#define MYS_ENABLED_HRTIMER_AACH64
MYS_PUBLIC const char *mys_hrname_aarch64();
MYS_PUBLIC uint64_t mys_hrtick_aarch64();
MYS_PUBLIC uint64_t mys_hrfreq_aarch64();
MYS_PUBLIC double mys_hrtime_aarch64();
#endif

#if defined(ARCH_X64) && defined(TSC_FREQ) && TSC_FREQ > 1
#define MYS_ENABLED_HRTIMER_X64
MYS_PUBLIC const char *mys_hrname_x64();
MYS_PUBLIC uint64_t mys_hrtick_x64();
MYS_PUBLIC uint64_t mys_hrfreq_x64();
MYS_PUBLIC double mys_hrtime_x64();
#endif

#if defined(POSIX_COMPLIANCE)
#define MYS_ENABLED_HRTIMER_POSIX
MYS_PUBLIC const char *mys_hrname_posix();
MYS_PUBLIC uint64_t mys_hrtick_posix();
MYS_PUBLIC uint64_t mys_hrfreq_posix();
MYS_PUBLIC double mys_hrtime_posix();
#endif

#if defined(__cplusplus)
#define MYS_ENABLED_HRTIMER_CXX
MYS_PUBLIC const char *mys_hrname_cxx();
MYS_PUBLIC uint64_t mys_hrtick_cxx();
MYS_PUBLIC uint64_t mys_hrfreq_cxx();
MYS_PUBLIC double mys_hrtime_cxx();
#endif

#if defined(OS_WINDOWS)
#define MYS_ENABLED_HRTIMER_WINDOWS
MYS_PUBLIC const char *mys_hrname_windows();
MYS_PUBLIC uint64_t mys_hrtick_windows();
MYS_PUBLIC uint64_t mys_hrfreq_windows();
MYS_PUBLIC double mys_hrtime_windows();
#endif


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
