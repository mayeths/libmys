// make test-trace && valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./a.out
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MYS_IMPL
#define MYS_NO_MPI
#include "mys.h"

int main()
{
    mys_debug_init();
    mys_rand_seed_time();
    // uint64_t ndata = 512;
    // uint64_t ndata = 1 * 1024;
    // uint64_t ndata = 2 * 1024;
    // size_t ndata = 1 * 1024 * 1024;
    size_t ndata = mys_rand_i64(1 * 1024 * 1024, 4 * 1024 * 1024);
    double *data = (double *)calloc(sizeof(double), ndata);
    double *validate = (double *)calloc(sizeof(double), ndata);
    mys_rand_f64_array(data, ndata, 0, 1e9);
    mys_trace_t *trace = mys_trace_create();
    ILOG(0, "Benchmarking trace %zu elem", ndata);

    {
        double t0 = mys_hrtime();
        for (size_t i = 0; i < ndata; ++i) {
            mys_trace_1d(trace, data[i]);
        }
        double t1 = mys_hrtime();
        double time_taken = t1 - t0;
        ILOG(0, "trace %zu took %fms -> %fns/elem (%.2fmop/sec)", ndata, time_taken * 1e3, time_taken * 1e9 / ndata, ndata / time_taken / 1e6);
    }

    {
        size_t niter = 0;
        double t0 = mys_hrtime();
        mys_trace_iter_t *iter = mys_trace_start_iter(trace);
        while (iter != NULL) {
            validate[niter] = iter->e1->d1;
            AS_EQ_DOUBLE(validate[niter], data[niter]);
            niter += 1;
            iter = mys_trace_next_iter(trace, iter);
        }
        double t1 = mys_hrtime();
        double time_taken = t1 - t0;
        AS_EQ_SIZET(niter, ndata);
        ILOG(0, "iter %zu took %fms -> %fns/elem (%.2fmop/sec)", ndata, time_taken * 1e3, time_taken * 1e9 / ndata, ndata / time_taken / 1e6);
    }

    // test leak of interrupt
    {
        size_t niter = 0;
        mys_trace_iter_t *iter = mys_trace_start_iter(trace);
        while (iter != NULL) {
            if (niter < ndata / 2) {
                niter += 1;
                iter = mys_trace_next_iter(trace, iter);
            } else {
                mys_trace_interrupt_iter(trace, iter);
                break;
            }
        }
    }

    mys_trace_destroy(&trace);
    char *alive_str = NULL;
    char *freed_str = NULL;
    char *total_str = NULL;
    mys_readable_size(&alive_str, mys_arena_trace->alive, 2);
    mys_readable_size(&freed_str, mys_arena_trace->freed, 2);
    mys_readable_size(&total_str, mys_arena_trace->total, 2);
    ILOG(0, "arena alive %s freed %s total %s", alive_str, freed_str, total_str);
    AS_EQ_SIZET(mys_arena_trace->freed, mys_arena_trace->total);
    free(data);
    free(validate);
    free(alive_str);
    free(freed_str);
    free(total_str);
    mys_debug_fini();
    return 0;
}
