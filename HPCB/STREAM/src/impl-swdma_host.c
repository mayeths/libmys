#include <mpi.h>

#define XCPE_HOST_IMPL
#include "common-sw.h"

#ifndef __sw_host__
#define SLAVE_FUN(name) name
#endif
extern void SLAVE_FUN(swdma_xcpe_runtime)();

typedef struct swdma_data_t {
    MPI_Comm comm;
    int narr;
    double scalar;
} swdma_data_t;

const char *swdma_d()
{
    return "DMA only";
}

void swdma_i(MPI_Comm comm, int narr, const double scalar, void **metadata)
{
    swdma_data_t *meta = NULL;
    meta = (swdma_data_t *)calloc(1, sizeof(swdma_data_t));
    meta->comm = comm;
    meta->narr = narr;
    meta->scalar = scalar;
    *metadata = meta;
 
    athread_init();
    athread_spawn(swdma_xcpe_runtime, 0);
}

void swdma_f(void **metadata)
{
    xcpe_stop_runtime();
    athread_join();
    free(*metadata);
    *metadata = NULL;
}

void swdma_c(void *metadata, double *c, const double *a)
{
    swdma_data_t *meta = (swdma_data_t *)metadata;

    // for (int j = 0; j < meta->narr; j++)
    //     c[j] = a[j];

    task_copy_args_t task_copy;
    task_copy.n = meta->narr;
    task_copy.c = c;
    task_copy.a = a;
    xcpe_dispatch(TASK_COPY_ID, &task_copy);
    xcpe_waitjob();
}

void swdma_s(void *metadata, double *c, const double *a)
{
    swdma_data_t *meta = (swdma_data_t *)metadata;
    // for (int j = 0; j < meta->narr; j++)
    //     c[j] = meta->scalar * a[j];
    task_triad_args_t task_scale;
    task_scale.n = meta->narr;
    task_scale.scalar = meta->scalar;
    task_scale.c = c;
    task_scale.a = a;
    xcpe_dispatch(TASK_SCALE_ID, &task_scale);
    xcpe_waitjob();
}

void swdma_a(void *metadata, double *c, const double *a, const double *b)
{
    swdma_data_t *meta = (swdma_data_t *)metadata;
    // for (int j = 0; j < meta->narr; j++)
    //     c[j] = a[j] + b[j];
    task_add_args_t task_add;
    task_add.n = meta->narr;
    task_add.c = c;
    task_add.a = a;
    task_add.b = b;
    xcpe_dispatch(TASK_ADD_ID, &task_add);
    xcpe_waitjob();
}

void swdma_t(void *metadata, double *c, const double *a, const double *b)
{
    swdma_data_t *meta = (swdma_data_t *)metadata;
    // for (int j = 0; j < meta->narr; j++)
    //     c[j] = meta->scalar * a[j] + b[j];
    task_triad_args_t task_triad;
    task_triad.n = meta->narr;
    task_triad.scalar = meta->scalar;
    task_triad.c = c;
    task_triad.a = a;
    task_triad.b = b;
    xcpe_dispatch(TASK_TRIAD_ID, &task_triad);
    xcpe_waitjob();
}
