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

#define SIZE_L 64 * 1024
__thread_local char l_mem[SIZE_L];

void do_swdma_copy(task_copy_args_t *args_ptr)
{
    int tid = athread_get_id(-1);
    task_copy_args_t task;
    athread_dma_get(&task, args_ptr, sizeof(task_copy_args_t));

    int lnarr = AILGN_4((sizeof(l_mem) / sizeof(double) / 3));
    double *lc = (double *)(&l_mem[0]);
    int tstart, tstop;
    mys_partition_naive(0, task.n, 64, tid, &tstart, &tstop);
    int tnum = tstop - tstart;
    if (tnum == 0) return;

    int nstep = (tnum - 1) / lnarr + 1;
    athread_rply_t dma_reply_a, dma_reply_c;
    int dma_reply_count_a = 0, dma_reply_count_c = 0;
    for (int step = 0; step < nstep; step++) {
        double *currla = NULL; /* curr */
        double *nextla = NULL; /* next */
        if (step % 2 == 0) {
            currla = (double *)(&l_mem[lnarr * sizeof(double)]);
            nextla = (double *)(&l_mem[lnarr * sizeof(double) * 2]);
        } else {
            currla = (double *)(&l_mem[lnarr * sizeof(double) * 2]);
            nextla = (double *)(&l_mem[lnarr * sizeof(double)]);
        }
        if (step + 1 < nstep) {
            int nexti = tstart + (step + 1) * lnarr;
            int nextn = MIN(tstop - nexti, lnarr);
            athread_dma_iget(nextla, (void *)&task.a[nexti], sizeof(double) * nextn, &dma_reply_a);
            dma_reply_count_a += 1;
        }
        int curri = tstart + step * lnarr;
        int currn = MIN(tstop - curri, lnarr);
        if (step == 0) {
            athread_dma_get(currla, (void *)&task.a[curri], sizeof(double) * currn);
        } else {
            athread_dma_wait_value(&dma_reply_a, dma_reply_count_a);
        }
        for (int j = 0; j < lnarr; j++) {
            lc[j] = currla[j];
        }
        if (step > 0) athread_dma_wait_value(&dma_reply_c, dma_reply_count_c);
        if (step + 1 < nstep) athread_dma_iput(&task.c[curri], lc, sizeof(double) * currn, &dma_reply_c);
        else athread_dma_put(&task.c[curri], lc, sizeof(double) * currn);
    }
}

void do_swdma_scale(task_scale_args_t *args_ptr)
{
    int tid = athread_get_id(-1);

    task_scale_args_t task;
    athread_dma_get(&task, args_ptr, sizeof(task_scale_args_t));
    int lnarr = AILGN_4((sizeof(l_mem) / sizeof(double) / 2));
    double *lc = (double *)(&l_mem[0]);
    double *la = (double *)(&l_mem[lnarr * sizeof(double)]);
    int tstart, tstop;
    mys_partition_naive(0, task.n, 64, tid, &tstart, &tstop);

    double scalar = task.scalar;
    for (int i = tstart; i < tstop; i += lnarr) {
        int bunch = (tstop - i) < lnarr ? (tstop - i) : lnarr;
        athread_dma_get(la, (void *)&task.a[i], sizeof(double) * bunch);
        for (int j = 0; j < lnarr; j++) {
            lc[j] = scalar * la[j];
        }
        athread_dma_put(&task.c[i], lc, sizeof(double) * bunch);
    }
}

void do_swdma_add(task_add_args_t *args_ptr)
{
    int tid = athread_get_id(-1);

    task_add_args_t task;
    athread_dma_get(&task, args_ptr, sizeof(task_add_args_t));
    int lnarr = AILGN_4((sizeof(l_mem) / sizeof(double) / 3));
    double *lc = (double *)(&l_mem[0]);
    double *la = (double *)(&l_mem[lnarr * sizeof(double)]);
    double *lb = (double *)(&l_mem[lnarr * sizeof(double) * 2]);
    int tstart, tstop;
    mys_partition_naive(0, task.n, 64, tid, &tstart, &tstop);

    for (int i = tstart; i < tstop; i += lnarr) {
        int bunch = (tstop - i) < lnarr ? (tstop - i) : lnarr;
        athread_dma_get(la, (void *)&task.a[i], sizeof(double) * bunch);
        athread_dma_get(lb, (void *)&task.b[i], sizeof(double) * bunch);
        for (int j = 0; j < lnarr; j++) {
            lc[j] = la[j] + lb[j];
        }
        athread_dma_put(&task.c[i], lc, sizeof(double) * bunch);
    }
}

void do_swdma_triad(task_triad_args_t *args_ptr)
{
    int tid = athread_get_id(-1);

    task_triad_args_t task;
    athread_dma_get(&task, args_ptr, sizeof(task_triad_args_t));
    int lnarr = AILGN_4(sizeof(l_mem) / sizeof(double) / 3);
    double *lc = (double *)(&l_mem[0]);
    double *la = (double *)(&l_mem[lnarr * sizeof(double)]);
    double *lb = (double *)(&l_mem[lnarr * sizeof(double) * 2]);
    int tstart, tstop;
    mys_partition_naive(0, task.n, 64, tid, &tstart, &tstop);

    double scalar = task.scalar;
    for (int i = tstart; i < tstop; i += lnarr) {
        int bunch = (tstop - i) < lnarr ? (tstop - i) : lnarr;
        athread_dma_get(la, (void *)&task.a[i], sizeof(double) * bunch);
        athread_dma_get(lb, (void *)&task.b[i], sizeof(double) * bunch);
        for (int j = 0; j < lnarr; j++) {
            lc[j] = scalar * la[j] + lb[j];
        }
        athread_dma_put(&task.c[i], lc, sizeof(double) * bunch);
    }
    flush_slave_cache();
}

void swdma_xcpe_runtime()
{
    while (1) {
        int task_id;
        void *args_ptr;
        xcpe_next(&task_id, &args_ptr);

        if (task_id == TASK_COPY_ID) {
            do_swdma_copy(args_ptr);
        } else if (task_id == TASK_SCALE_ID) {
            do_swdma_scale(args_ptr);
        } else if (task_id == TASK_ADD_ID) {
            do_swdma_add(args_ptr);
        } else if (task_id == TASK_TRIAD_ID) {
            do_swdma_triad(args_ptr);
        } else if (task_id == TASK_EXIT) {
            break;
        }
        athread_ssync_master_array();
    }
}

