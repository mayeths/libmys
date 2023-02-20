#pragma once
/* Please check https://github.com/nclack/tictoc for platform-timer (Windows/Linux/MacOS) */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "macro.h"


MYS_API const char *mys_hrname();
MYS_API uint64_t mys_hrtick();
MYS_API uint64_t mys_hrfreq();
MYS_API double mys_hrtime();

MYS_API const char *mys_hrname_mpi();
MYS_API uint64_t mys_hrtick_mpi();
MYS_API uint64_t mys_hrfreq_mpi();
MYS_API double mys_hrtime_mpi();

MYS_API const char *mys_hrname_openmp();
MYS_API uint64_t mys_hrtick_openmp();
MYS_API uint64_t mys_hrfreq_openmp();
MYS_API double mys_hrtime_openmp();

#if defined(ARCH_AARCH64)
#define MYS_ENABLED_HRTIMER_AACH64
MYS_API const char *mys_hrname_aarch64();
MYS_API uint64_t mys_hrtick_aarch64();
MYS_API uint64_t mys_hrfreq_aarch64();
MYS_API double mys_hrtime_aarch64();
#endif

#if defined(ARCH_X64) && defined(TSC_FREQ) && TSC_FREQ > 1
#define MYS_ENABLED_HRTIMER_X64
MYS_API const char *mys_hrname_x64();
MYS_API uint64_t mys_hrtick_x64();
MYS_API uint64_t mys_hrfreq_x64();
MYS_API double mys_hrtime_x64();
#endif

#if defined(POSIX_COMPLIANCE)
#define MYS_ENABLED_HRTIMER_POSIX
MYS_API const char *mys_hrname_posix();
MYS_API uint64_t mys_hrtick_posix();
MYS_API uint64_t mys_hrfreq_posix();
MYS_API double mys_hrtime_posix();
#endif

#if defined(OS_WINDOWS)
#define MYS_ENABLED_HRTIMER_WINDOWS
MYS_API const char *mys_hrname_windows();
MYS_API uint64_t mys_hrtick_windows();
MYS_API uint64_t mys_hrfreq_windows();
MYS_API double mys_hrtime_windows();
#endif

//////// Legacy
#if !defined(MYS_NO_LEGACY) && !defined(MYS_NO_LEGACY_HRTIME)
MYS_API static const char *hrname() { return mys_hrname(); }
MYS_API static uint64_t hrtick() { return mys_hrtick(); }
MYS_API static uint64_t hrfreq() { return mys_hrfreq(); }
MYS_API static double hrtime() { return mys_hrtime(); }
#endif

/******* AUX *******/

#if defined(POSIX_COMPLIANCE)
#include <stdlib.h>
#include <unistd.h>
static inline uint64_t test_freq()
{
    uint64_t raw1 = hrtick();
    sleep(1);
    uint64_t raw2 = hrtick();
    return raw2 - raw1;
}
#endif

/* g++ -std=c++11 -I../include -lm ./a.cpp && ./a.out
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int main(int argc, const char **argv)
{
    uint64_t n = 1000000000;
    double t = 0;
    double t1 = hrtime();
    for (uint64_t i = 0; i < n; i++) {
        t += hrtime();
    }
    double t2 = hrtime();
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
