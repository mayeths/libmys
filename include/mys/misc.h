#pragma once

#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "myspi.h"
#include "thread.h"

#define WAIT_FLAG(flagfile) __mys_wait_flag(__FILE__, __LINE__, flagfile)

static inline void __mys_wait_flag(const char *file, int line, const char *flagfile) {
    int myrank = __mys_myrank();
    // MYS_OMP_CRITICAL
    {
        int nranks = __mys_nranks();
        int nprefix = trunc(log10(nranks)) + 1;
        nprefix = nprefix > 3 ? nprefix : 3;
        if (myrank == 0) {
            fprintf(stdout, "[WAIT::%0*d %s:%03d] Use \"touch %s\" to continue... ", nprefix, 0, file, line, flagfile);
            fflush(stdout);
        }
    }
    struct stat fstat;
    time_t mod_time;
    if (stat(flagfile, &fstat) == 0) {
        mod_time = fstat.st_mtime;
    } else {
        mod_time = 0;
    }
    while (1) {
        if (stat(flagfile, &fstat) == 0) {
            if (mod_time < fstat.st_mtime) break;
        }
        sleep(1);
    }
    __mys_barrier();
    // MYS_OMP_CRITICAL
    if (myrank == 0) {
        fprintf(stdout, "OK\n");
        fflush(stdout);
    }
}

