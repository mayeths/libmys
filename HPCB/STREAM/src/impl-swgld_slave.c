#define XCPE_SLAVE_IMPL
#include "common-sw.h"

#ifndef __sw_slave__
#define __thread_local
#define __uncached
#endif

static void mys_partition_naive(const int gs, const int ge, const int n, const int i, int *tstart, int *tstop) {
    const int total = ge - gs;
    int size = total / n;
    int rest = total % n;
    (*tstart) = gs;
    if (i < rest) {
        size += 1;
        (*tstart) += (i * size);
    } else {
        (*tstart) += rest + (i * size);
    }
    (*tstop) = (*tstart) + size;
}

void do_swgld_copy(task_copy_args_t *args_ptr)
{
    int tid = athread_get_id(-1);
    task_copy_args_t task;
    athread_dma_get(&task, args_ptr, sizeof(task_copy_args_t));

    int tstart, tstop;
    mys_partition_naive(0, task.n, 64, tid, &tstart, &tstop);

    double *c = task.c;
    const double *a = task.a;
    for (int i = tstart; i < tstop; i += 1) {
        c[i] = a[i];
    }
    flush_slave_cache();
}

void do_swgld_scale(task_scale_args_t *args_ptr)
{
    int tid = athread_get_id(-1);
    task_scale_args_t task;
    athread_dma_get(&task, args_ptr, sizeof(task_scale_args_t));

    int tstart, tstop;
    mys_partition_naive(0, task.n, 64, tid, &tstart, &tstop);

    double scalar = task.scalar;
    double *c = task.c;
    const double *a = task.a;
    for (int i = tstart; i < tstop; i += 1) {
        c[i] = scalar * a[i];
    }
    flush_slave_cache();
}

void do_swgld_add(task_add_args_t *args_ptr)
{
    int tid = athread_get_id(-1);
    task_add_args_t task;
    athread_dma_get(&task, args_ptr, sizeof(task_add_args_t));

    int tstart, tstop;
    mys_partition_naive(0, task.n, 64, tid, &tstart, &tstop);

    double *c = task.c;
    const double *a = task.a;
    const double *b = task.b;
    for (int i = tstart; i < tstop; i += 1) {
        c[i] = a[i] + b[i];
    }
    flush_slave_cache();
}

void do_swgld_triad(task_triad_args_t *args_ptr)
{
    int tid = athread_get_id(-1);
    task_triad_args_t task;
    athread_dma_get(&task, args_ptr, sizeof(task_triad_args_t));

    int tstart, tstop;
    mys_partition_naive(0, task.n, 64, tid, &tstart, &tstop);

    double scalar = task.scalar;
    double *c = task.c;
    const double *a = task.a;
    const double *b = task.b;
    for (int i = tstart; i < tstop; i += 1) {
        c[i] = scalar * a[i] + b[i];
    }
    flush_slave_cache();
}

void swgld_xcpe_runtime()
{
    while (1) {
        int task_id;
        void *args_ptr;
        xcpe_next(&task_id, &args_ptr);

        if (task_id == TASK_COPY_ID) {
            do_swgld_copy(args_ptr);
        } else if (task_id == TASK_SCALE_ID) {
            do_swgld_scale(args_ptr);
        } else if (task_id == TASK_ADD_ID) {
            do_swgld_add(args_ptr);
        } else if (task_id == TASK_TRIAD_ID) {
            do_swgld_triad(args_ptr);
        } else if (task_id == TASK_EXIT) {
            break;
        }
        athread_ssync_master_array();
    }
}

