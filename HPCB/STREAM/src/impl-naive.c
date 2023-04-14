#include <mpi.h>
#include "common.h"

typedef struct naive_data_t {
    MPI_Comm comm;
    int narr;
    STREAM_TYPE scalar;
} naive_data_t;

const char *naive_d()
{
    return "MPI Only";
}

void naive_i(MPI_Comm comm, int narr, const STREAM_TYPE scalar, void **metadata)
{
    naive_data_t *meta = NULL;
    meta = (naive_data_t *)calloc(1, sizeof(naive_data_t));
    meta->comm = comm;
    meta->narr = narr;
    meta->scalar = scalar;
    *metadata = meta;
}

void naive_f(void **metadata)
{
    free(*metadata);
    *metadata = NULL;
}

void naive_c(void *metadata, STREAM_TYPE *c, const STREAM_TYPE *a)
{
    naive_data_t *meta = (naive_data_t *)metadata;
    for (int j = 0; j < meta->narr; j++)
        c[j] = a[j];
}

void naive_s(void *metadata, STREAM_TYPE *c, const STREAM_TYPE *a)
{
    naive_data_t *meta = (naive_data_t *)metadata;
    for (int j = 0; j < meta->narr; j++)
        c[j] = meta->scalar * a[j];
}

void naive_a(void *metadata, STREAM_TYPE *c, const STREAM_TYPE *a, const STREAM_TYPE *b)
{
    naive_data_t *meta = (naive_data_t *)metadata;
    for (int j = 0; j < meta->narr; j++)
        c[j] = a[j] + b[j];
}

void naive_t(void *metadata, STREAM_TYPE *c, const STREAM_TYPE *a, const STREAM_TYPE *b)
{
    naive_data_t *meta = (naive_data_t *)metadata;
    for (int j = 0; j < meta->narr; j++)
        c[j] = meta->scalar * a[j] + b[j];
}
