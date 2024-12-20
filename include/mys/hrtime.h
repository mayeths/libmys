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
#include "mpistubs.h"
#include "thread.h"

/**
 * @brief Get the name of the default high-resolution timer
 * 
 * This function returns the name of the default high-resolution timer in use. The timer name may vary
 * depending on the platform and configuration.
 * 
 * @return A string representing the name of the default high-resolution timer.
 */
MYS_PUBLIC const char *mys_hrname();

/**
 * @brief Get the current time based on high-resolution timer
 * 
 * This function returns the current time in seconds based on the high-resolution timer.
 * 
 * @return The current time in seconds as a `double` value.
 * 
 * @see mys_hrtick, mys_hrfreq
 */
MYS_PUBLIC double mys_hrtime();

/**
 * @brief Get the number of timer ticks since the last timer reset
 * 
 * This function returns the number of high-resolution timer ticks that have occurred since the last call to 
 * `mys_hrreset()` for this process. The value represents the number of ticks from the timer’s epoch, and 
 * it is used to measure the passage of time since the last reset.
 * 
 * @return The number of timer ticks as a `uint64_t` value.
 * 
 * @note If `mys_hrreset()` has not been called, this value represents the number of ticks since the program started.
 */
MYS_PUBLIC uint64_t mys_hrtick();

/**
 * @brief Get the frequency of the default high-resolution timer
 * 
 * This function returns the frequency of the default high-resolution timer, i.e., the number of ticks per second.
 * The frequency is constant for the lifetime of the program and is used to convert timer ticks into seconds.
 * 
 * @return The frequency of the timer in ticks per second.
 */
MYS_PUBLIC uint64_t mys_hrfreq();

/**
 * @brief Reset the timestamp offset for the current process
 * 
 * This function resets the timestamp offset for the default high-resolution timer to zero. After calling this function, 
 * the subsequent calls to `mys_hrtime()` or `mys_hrtick()` will restart counting from zero for this process. 
 * It is useful for recalibrating or aligning the timer between different processes or stages of execution.
 * 
 * @note This function affects only the current process. Other processes in a distributed system are unaffected.
 */
MYS_PUBLIC void mys_hrreset();

/**
 * @brief Synchronize timers across all MPI processes
 * 
 * This function synchronizes the following built-in timers (if they are available) across all MPI processes 
 * to minimize timestamp offset errors as much as possible:
 * 
 * - AArch64-based timer (when `MYS_HRTIMER_HAVE_AARCH64` is defined)
 * 
 * - POSIX-based timer (when `MYS_HRTIMER_HAVE_POSIX` is defined)
 * 
 * - MPI-based timer (when `MYS_HRTIMER_HAVE_MPI` is defined)
 * 
 * @param comm The MPI communicator (typically `MPI_COMM_WORLD` or any other valid communicator).
 */
MYS_PUBLIC void mys_hrsync(mys_MPI_Comm comm);

#if defined(ARCH_AARCH64)
#define MYS_HRTIMER_HAVE_AARCH64
MYS_PUBLIC const char *mys_hrname_aarch64();
MYS_PUBLIC uint64_t mys_hrtick_aarch64();
MYS_PUBLIC uint64_t mys_hrfreq_aarch64();
MYS_PUBLIC double mys_hrtime_aarch64();
MYS_PUBLIC void mys_hrreset_aarch64();
#endif

#if defined(POSIX_COMPLIANCE)
#define MYS_HRTIMER_HAVE_POSIX
MYS_PUBLIC const char *mys_hrname_posix();
MYS_PUBLIC uint64_t mys_hrtick_posix();
MYS_PUBLIC uint64_t mys_hrfreq_posix();
MYS_PUBLIC double mys_hrtime_posix();
MYS_PUBLIC void mys_hrreset_posix();
#endif

#define MYS_HRTIMER_HAVE_MPI
MYS_PUBLIC const char *mys_hrname_mpi();
MYS_PUBLIC uint64_t mys_hrtick_mpi();
MYS_PUBLIC uint64_t mys_hrfreq_mpi();
MYS_PUBLIC double mys_hrtime_mpi();
MYS_PUBLIC void mys_hrreset_mpi();

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
